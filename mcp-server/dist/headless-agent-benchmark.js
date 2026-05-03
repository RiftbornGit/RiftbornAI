import * as fs from "node:fs";
import * as path from "node:path";
import { defaultHarnessTranscriptPath, loadHeadlessHarnessScenario, runHeadlessAgentHarness, writeHarnessTranscript, } from "./headless-agent-harness.js";
const DEFAULT_BENCHMARK_PASS_THRESHOLD = 0.75;
function average(values) {
    if (values.length === 0) {
        return 1;
    }
    return values.reduce((sum, value) => sum + value, 0) / values.length;
}
function clampScore(value) {
    if (!Number.isFinite(value)) {
        return 0;
    }
    return Math.min(Math.max(value, 0), 1);
}
function normalizeSuiteScenarioEntry(raw, index) {
    if (typeof raw === "string") {
        const trimmed = raw.trim();
        if (!trimmed) {
            throw new Error(`suite.scenarios[${index}] must not be empty.`);
        }
        return { path: trimmed };
    }
    if (!raw || typeof raw !== "object" || Array.isArray(raw)) {
        throw new Error(`suite.scenarios[${index}] must be a string or object.`);
    }
    const value = raw;
    const scenarioPath = typeof value.path === "string" ? value.path.trim() : "";
    if (!scenarioPath) {
        throw new Error(`suite.scenarios[${index}].path is required.`);
    }
    return {
        path: scenarioPath,
        label: typeof value.label === "string" ? value.label.trim() || undefined : undefined,
        model: typeof value.model === "string" ? value.model.trim() || undefined : undefined,
        max_iterations: typeof value.max_iterations === "number" && Number.isFinite(value.max_iterations)
            ? Math.min(Math.max(1, Math.trunc(value.max_iterations)), 50)
            : undefined,
    };
}
function slugify(value) {
    const slug = value
        .toLowerCase()
        .replace(/[^a-z0-9]+/g, "-")
        .replace(/^-+|-+$/g, "");
    return slug || "suite";
}
function defaultHarnessBenchmarkReportPath(suiteName) {
    const stamp = new Date().toISOString().replace(/[:.]/g, "-");
    return path.resolve(path.dirname(defaultHarnessTranscriptPath(suiteName)), "suites", `${slugify(suiteName)}-${stamp}`, "suite-report.json");
}
function containsPhraseCaseInsensitive(haystack, needle) {
    return haystack.toLowerCase().includes(needle.toLowerCase());
}
function computeOrderedCoverage(actual, expected) {
    if (expected.length === 0) {
        return 1;
    }
    let actualIndex = 0;
    let matched = 0;
    for (const expectedTool of expected) {
        while (actualIndex < actual.length && actual[actualIndex] !== expectedTool) {
            actualIndex += 1;
        }
        if (actualIndex >= actual.length) {
            break;
        }
        matched += 1;
        actualIndex += 1;
    }
    return matched / expected.length;
}
function getDelegationMode(transcript) {
    const hasSubagent = transcript.subagent_runs.length > 0;
    const hasAgentTask = transcript.agent_task_runs.length > 0;
    if (hasSubagent && hasAgentTask) {
        return "mixed";
    }
    if (hasSubagent) {
        return "subagent";
    }
    if (hasAgentTask) {
        return "agent_task";
    }
    return "direct";
}
function getChildTranscripts(transcript, mode) {
    if (mode === "subagent") {
        return transcript.subagent_runs.map((entry) => entry.transcript);
    }
    if (mode === "agent_task") {
        return transcript.agent_task_runs
            .map((entry) => entry.transcript)
            .filter((entry) => entry !== undefined);
    }
    if (mode === "mixed") {
        return [
            ...transcript.subagent_runs.map((entry) => entry.transcript),
            ...transcript.agent_task_runs
                .map((entry) => entry.transcript)
                .filter((entry) => entry !== undefined),
        ];
    }
    return [];
}
function evaluateToolPathExpectation(actualPath, expectation, issues, category, label) {
    if (!expectation) {
        return 1;
    }
    const metrics = [];
    if (expectation.required_tools && expectation.required_tools.length > 0) {
        const matched = expectation.required_tools.filter((tool) => actualPath.includes(tool));
        const coverage = matched.length / expectation.required_tools.length;
        metrics.push(coverage);
        if (coverage < 1) {
            const missing = expectation.required_tools.filter((tool) => !actualPath.includes(tool));
            issues.push({
                category,
                severity: "error",
                message: `${label} missing required tool(s): ${missing.join(", ")}`,
            });
        }
    }
    if (expectation.ordered_subsequence && expectation.ordered_subsequence.length > 0) {
        const coverage = computeOrderedCoverage(actualPath, expectation.ordered_subsequence);
        metrics.push(coverage);
        if (coverage < 1) {
            issues.push({
                category,
                severity: "warn",
                message: `${label} path did not follow the expected ordered subsequence: ${expectation.ordered_subsequence.join(" -> ")}`,
            });
        }
    }
    if (expectation.forbidden_tools && expectation.forbidden_tools.length > 0) {
        const hits = expectation.forbidden_tools.filter((tool) => actualPath.includes(tool));
        metrics.push(hits.length === 0 ? 1 : 0);
        if (hits.length > 0) {
            issues.push({
                category,
                severity: "error",
                message: `${label} used forbidden tool(s): ${hits.join(", ")}`,
            });
        }
    }
    return clampScore(average(metrics));
}
function evaluateUnnecessaryCallScore(totalToolCalls, expectation, issues) {
    if (!expectation) {
        return 1;
    }
    if (expectation.ideal_total_calls !== undefined) {
        if (totalToolCalls <= expectation.ideal_total_calls) {
            return 1;
        }
        issues.push({
            category: "unnecessary_calls",
            severity: "warn",
            message: `Used ${totalToolCalls} tool calls; ideal budget was ${expectation.ideal_total_calls}.`,
        });
        return clampScore(expectation.ideal_total_calls / totalToolCalls);
    }
    if (expectation.max_total_calls !== undefined) {
        if (totalToolCalls <= expectation.max_total_calls) {
            return 1;
        }
        issues.push({
            category: "unnecessary_calls",
            severity: "warn",
            message: `Used ${totalToolCalls} tool calls; max budget was ${expectation.max_total_calls}.`,
        });
        return clampScore(expectation.max_total_calls / totalToolCalls);
    }
    return 1;
}
function evaluateDelegationScore(transcript, expectation, issues) {
    const mode = getDelegationMode(transcript);
    const childTranscripts = getChildTranscripts(transcript, mode);
    const childToolPaths = childTranscripts.map((entry) => entry.tool_executions.map((tool) => tool.requested_tool));
    if (!expectation) {
        return {
            score: 1,
            mode,
            childToolPaths,
        };
    }
    const metrics = [];
    if (expectation.expected_mode) {
        const matches = expectation.expected_mode === mode;
        metrics.push(matches ? 1 : 0);
        if (!matches) {
            issues.push({
                category: "delegation",
                severity: "error",
                message: `Expected delegation mode '${expectation.expected_mode}' but observed '${mode}'.`,
            });
        }
    }
    const childExpectationPresent = Boolean(expectation.child_required_tools?.length
        || expectation.child_ordered_subsequence?.length
        || expectation.child_forbidden_tools?.length);
    if (childExpectationPresent) {
        if (childToolPaths.length === 0) {
            metrics.push(0);
            issues.push({
                category: "delegation",
                severity: "error",
                message: "Expected delegated child work but no child transcript was available.",
            });
        }
        else {
            const childScores = childToolPaths.map((childPath) => evaluateToolPathExpectation(childPath, {
                required_tools: expectation.child_required_tools,
                ordered_subsequence: expectation.child_ordered_subsequence,
                forbidden_tools: expectation.child_forbidden_tools,
            }, issues, "delegation", "Delegated child"));
            metrics.push(Math.max(...childScores));
        }
    }
    return {
        score: clampScore(average(metrics)),
        mode,
        childToolPaths,
    };
}
function evaluateFinalOutputScore(finalOutput, expectation, issues) {
    if (!expectation) {
        return 1;
    }
    const normalizedOutput = finalOutput.trim();
    const metrics = [];
    if (expectation.required_phrases && expectation.required_phrases.length > 0) {
        const matched = expectation.required_phrases.filter((phrase) => containsPhraseCaseInsensitive(normalizedOutput, phrase));
        const coverage = matched.length / expectation.required_phrases.length;
        metrics.push(coverage);
        if (coverage < 1) {
            const missing = expectation.required_phrases.filter((phrase) => !containsPhraseCaseInsensitive(normalizedOutput, phrase));
            issues.push({
                category: "final_output",
                severity: "error",
                message: `Final output missing required phrase(s): ${missing.join(", ")}`,
            });
        }
    }
    if (expectation.forbidden_phrases && expectation.forbidden_phrases.length > 0) {
        const hits = expectation.forbidden_phrases.filter((phrase) => containsPhraseCaseInsensitive(normalizedOutput, phrase));
        metrics.push(hits.length === 0 ? 1 : 0);
        if (hits.length > 0) {
            issues.push({
                category: "final_output",
                severity: "warn",
                message: `Final output used forbidden phrase(s): ${hits.join(", ")}`,
            });
        }
    }
    if (!normalizedOutput) {
        issues.push({
            category: "final_output",
            severity: "error",
            message: "Model returned no final output text.",
        });
        metrics.push(0);
    }
    return clampScore(average(metrics));
}
export function evaluateHeadlessHarnessTranscript(scenario, transcript) {
    const benchmark = scenario.benchmark;
    if (!benchmark) {
        return undefined;
    }
    const issues = [];
    const actualToolPath = transcript.tool_executions.map((entry) => entry.requested_tool);
    const toolPathScore = evaluateToolPathExpectation(actualToolPath, benchmark.tool_path, issues, "tool_path", "Top-level");
    const unnecessaryCallScore = evaluateUnnecessaryCallScore(transcript.tool_executions.length, benchmark.tool_path, issues);
    const delegation = evaluateDelegationScore(transcript, benchmark.delegation, issues);
    const finalOutputScore = evaluateFinalOutputScore(transcript.final_output_text, benchmark.final_output, issues);
    const aggregateScore = clampScore(average([
        toolPathScore,
        unnecessaryCallScore,
        delegation.score,
        finalOutputScore,
    ]));
    const passThreshold = benchmark.pass_threshold ?? DEFAULT_BENCHMARK_PASS_THRESHOLD;
    return {
        pass_threshold: passThreshold,
        passed: aggregateScore >= passThreshold,
        tool_path_quality: toolPathScore,
        unnecessary_calls: unnecessaryCallScore,
        delegation_quality: delegation.score,
        final_diagnosis_quality: finalOutputScore,
        aggregate_score: aggregateScore,
        actual: {
            tool_path: actualToolPath,
            total_tool_calls: transcript.tool_executions.length,
            delegation_mode: delegation.mode,
            child_tool_paths: delegation.childToolPaths,
        },
        issues,
    };
}
export function loadHeadlessHarnessBenchmarkSuite(filePath) {
    const resolvedPath = path.resolve(filePath);
    const raw = JSON.parse(fs.readFileSync(resolvedPath, "utf8"));
    const name = typeof raw.name === "string" ? raw.name.trim() : "";
    if (!name) {
        throw new Error("Benchmark suite requires a non-empty 'name' string.");
    }
    if (!Array.isArray(raw.scenarios) || raw.scenarios.length === 0) {
        throw new Error("Benchmark suite requires a non-empty 'scenarios' array.");
    }
    return {
        suite_path: resolvedPath,
        suite_directory: path.dirname(resolvedPath),
        name,
        description: typeof raw.description === "string" ? raw.description.trim() || undefined : undefined,
        model: typeof raw.model === "string" ? raw.model.trim() || undefined : undefined,
        max_iterations: typeof raw.max_iterations === "number" && Number.isFinite(raw.max_iterations)
            ? Math.min(Math.max(1, Math.trunc(raw.max_iterations)), 50)
            : undefined,
        pass_threshold: typeof raw.pass_threshold === "number" && Number.isFinite(raw.pass_threshold)
            ? Math.min(Math.max(raw.pass_threshold, 0), 1)
            : undefined,
        scenarios: raw.scenarios.map((entry, index) => normalizeSuiteScenarioEntry(entry, index)),
    };
}
function defaultScenarioTranscriptPathForSuite(reportPath, scenarioName) {
    const reportDirectory = path.dirname(reportPath);
    return path.resolve(reportDirectory, "transcripts", `${slugify(scenarioName)}.json`);
}
export async function runHeadlessHarnessBenchmarkSuite(options) {
    const reportPath = options.outputReportPath
        ? path.resolve(options.outputReportPath)
        : defaultHarnessBenchmarkReportPath(options.suite.name);
    const startedAt = new Date().toISOString();
    const scenarioReports = [];
    for (const entry of options.suite.scenarios) {
        const scenarioPath = path.resolve(options.suite.suite_directory, entry.path);
        const scenario = loadHeadlessHarnessScenario(scenarioPath);
        const transcript = await runHeadlessAgentHarness({
            scenario,
            client: options.client,
            model: entry.model ?? options.model ?? options.suite.model,
            maxIterations: entry.max_iterations ?? options.maxIterations ?? options.suite.max_iterations,
        });
        const transcriptPath = defaultScenarioTranscriptPathForSuite(reportPath, scenario.name ?? entry.label ?? path.basename(entry.path, path.extname(entry.path)));
        writeHarnessTranscript(transcriptPath, transcript);
        scenarioReports.push({
            scenario_name: scenario.name ?? path.basename(entry.path, path.extname(entry.path)),
            scenario_path: scenarioPath,
            transcript_path: transcriptPath,
            scorecard: (() => {
                const scorecard = evaluateHeadlessHarnessTranscript(scenario, transcript);
                if (!scorecard) {
                    return undefined;
                }
                if (options.suite.pass_threshold !== undefined
                    && scenario.benchmark?.pass_threshold === undefined) {
                    scorecard.pass_threshold = options.suite.pass_threshold;
                    scorecard.passed = scorecard.aggregate_score >= scorecard.pass_threshold;
                }
                return scorecard;
            })(),
        });
    }
    const scoredReports = scenarioReports.filter((entry) => entry.scorecard !== undefined);
    const passedCount = scoredReports.filter((entry) => entry.scorecard?.passed).length;
    const failedCount = scoredReports.length - passedCount;
    const aggregateScore = scoredReports.length > 0
        ? average(scoredReports.map((entry) => entry.scorecard?.aggregate_score ?? 0))
        : 0;
    const report = {
        name: options.suite.name,
        description: options.suite.description,
        suite_path: options.suite.suite_path,
        report_path: reportPath,
        started_at: startedAt,
        completed_at: new Date().toISOString(),
        scenario_count: scenarioReports.length,
        scored_count: scoredReports.length,
        passed_count: passedCount,
        failed_count: failedCount,
        aggregate_score: aggregateScore,
        scenario_reports: scenarioReports,
    };
    fs.mkdirSync(path.dirname(reportPath), { recursive: true });
    fs.writeFileSync(reportPath, JSON.stringify(report, null, 2), "utf8");
    return report;
}
export function summarizeHeadlessHarnessBenchmarkSuite(report) {
    const summaryLines = [
        `Suite: ${report.name}`,
        `Scenarios: ${report.scenario_count}`,
        `Scored: ${report.scored_count}`,
        `Passed: ${report.passed_count}`,
        `Failed: ${report.failed_count}`,
        `Average score: ${(report.aggregate_score * 100).toFixed(1)}%`,
    ];
    const worstFailures = report.scenario_reports
        .filter((entry) => entry.scorecard && !entry.scorecard.passed)
        .sort((left, right) => (left.scorecard?.aggregate_score ?? 1) - (right.scorecard?.aggregate_score ?? 1))
        .slice(0, 5);
    if (worstFailures.length > 0) {
        summaryLines.push("Failures:");
        for (const failure of worstFailures) {
            const scorecard = failure.scorecard;
            const firstIssue = scorecard.issues[0]?.message ?? "No issue details recorded.";
            summaryLines.push(`- ${failure.scenario_name}: ${(scorecard.aggregate_score * 100).toFixed(1)}% (${firstIssue})`);
        }
    }
    return summaryLines.join("\n");
}