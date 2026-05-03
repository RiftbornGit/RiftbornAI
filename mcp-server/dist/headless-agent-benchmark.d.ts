import { type HarnessModelClient, type HeadlessHarnessScenario, type HeadlessHarnessTranscript } from "./headless-agent-harness.js";
export interface HeadlessHarnessBenchmarkIssue {
    category: "tool_path" | "unnecessary_calls" | "delegation" | "final_output";
    severity: "info" | "warn" | "error";
    message: string;
}
export interface HeadlessHarnessBenchmarkScorecard {
    pass_threshold: number;
    passed: boolean;
    tool_path_quality: number;
    unnecessary_calls: number;
    delegation_quality: number;
    final_diagnosis_quality: number;
    aggregate_score: number;
    actual: {
        tool_path: string[];
        total_tool_calls: number;
        delegation_mode: "direct" | "subagent" | "agent_task" | "mixed";
        child_tool_paths: string[][];
    };
    issues: HeadlessHarnessBenchmarkIssue[];
}
export interface HeadlessHarnessBenchmarkSuiteScenario {
    path: string;
    label?: string;
    model?: string;
    max_iterations?: number;
}
export interface HeadlessHarnessBenchmarkSuite {
    name: string;
    description?: string;
    model?: string;
    max_iterations?: number;
    pass_threshold?: number;
    scenarios: HeadlessHarnessBenchmarkSuiteScenario[];
}
export interface HeadlessHarnessLoadedBenchmarkSuite extends HeadlessHarnessBenchmarkSuite {
    suite_path: string;
    suite_directory: string;
}
export interface HeadlessHarnessBenchmarkScenarioReport {
    scenario_name: string;
    scenario_path: string;
    transcript_path: string;
    scorecard?: HeadlessHarnessBenchmarkScorecard;
}
export interface HeadlessHarnessBenchmarkSuiteReport {
    name: string;
    description?: string;
    suite_path: string;
    report_path: string;
    started_at: string;
    completed_at: string;
    scenario_count: number;
    scored_count: number;
    passed_count: number;
    failed_count: number;
    aggregate_score: number;
    scenario_reports: HeadlessHarnessBenchmarkScenarioReport[];
}
export interface RunHeadlessHarnessBenchmarkSuiteOptions {
    suite: HeadlessHarnessLoadedBenchmarkSuite;
    client: HarnessModelClient;
    model?: string;
    maxIterations?: number;
    outputReportPath?: string;
}
export declare function evaluateHeadlessHarnessTranscript(scenario: HeadlessHarnessScenario, transcript: HeadlessHarnessTranscript): HeadlessHarnessBenchmarkScorecard | undefined;
export declare function loadHeadlessHarnessBenchmarkSuite(filePath: string): HeadlessHarnessLoadedBenchmarkSuite;
export declare function runHeadlessHarnessBenchmarkSuite(options: RunHeadlessHarnessBenchmarkSuiteOptions): Promise<HeadlessHarnessBenchmarkSuiteReport>;
export declare function summarizeHeadlessHarnessBenchmarkSuite(report: HeadlessHarnessBenchmarkSuiteReport): string;