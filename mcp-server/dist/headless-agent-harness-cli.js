#!/usr/bin/env node
import * as path from "node:path";
import { loadHeadlessHarnessBenchmarkSuite, runHeadlessHarnessBenchmarkSuite, summarizeHeadlessHarnessBenchmarkSuite, } from "./headless-agent-benchmark.js";
import { OpenAIResponsesClient, defaultHarnessTranscriptPath, loadHeadlessHarnessScenario, runHeadlessAgentHarness, summarizeHarnessTranscript, writeHarnessTranscript, } from "./headless-agent-harness.js";
function parseCliArgs(argv) {
    const options = { help: false };
    for (let index = 0; index < argv.length; index++) {
        const arg = argv[index];
        switch (arg) {
            case "--scenario":
            case "-s":
                options.scenarioPath = argv[++index];
                break;
            case "--suite":
                options.suitePath = argv[++index];
                break;
            case "--output":
            case "-o":
                options.outputPath = argv[++index];
                break;
            case "--model":
            case "-m":
                options.model = argv[++index];
                break;
            case "--max-iterations":
                options.maxIterations = Number(argv[++index]);
                break;
            case "--help":
            case "-h":
                options.help = true;
                break;
            default:
                throw new Error(`Unknown argument: ${arg}`);
        }
    }
    return options;
}
function printHelp() {
    console.log(`RiftbornAI Headless Agent Harness

Usage:
  pnpm harness -- --scenario <scenario.json>
  pnpm harness -- --suite <suite.json>
  pnpm harness -- --scenario <path> --output <transcript.json> --model gpt-5.4

Arguments:
  --scenario, -s         Path to a harness scenario JSON file
  --suite                Path to a benchmark suite JSON file
  --output, -o           Optional transcript path in scenario mode, or suite report path in suite mode
  --model, -m            Optional model override
  --max-iterations       Optional loop cap override
  --help, -h             Show this help
`);
}
async function main() {
    const cli = parseCliArgs(process.argv.slice(2));
    if (cli.help) {
        printHelp();
        return;
    }
    if (!cli.scenarioPath) {
        if (!cli.suitePath) {
            throw new Error("Missing required --scenario <path> or --suite <path> argument.");
        }
    }
    if (cli.scenarioPath && cli.suitePath) {
        throw new Error("Use either --scenario or --suite, not both.");
    }
    const client = new OpenAIResponsesClient();
    if (cli.suitePath) {
        const resolvedSuitePath = path.resolve(process.cwd(), cli.suitePath);
        const suite = loadHeadlessHarnessBenchmarkSuite(resolvedSuitePath);
        const report = await runHeadlessHarnessBenchmarkSuite({
            suite,
            client,
            model: cli.model,
            maxIterations: cli.maxIterations,
            outputReportPath: cli.outputPath ? path.resolve(process.cwd(), cli.outputPath) : undefined,
        });
        console.log(summarizeHeadlessHarnessBenchmarkSuite(report));
        console.log(`Report: ${report.report_path}`);
        return;
    }
    const resolvedScenarioPath = path.resolve(process.cwd(), cli.scenarioPath);
    const scenario = loadHeadlessHarnessScenario(resolvedScenarioPath);
    const transcript = await runHeadlessAgentHarness({
        scenario,
        client,
        model: cli.model,
        maxIterations: cli.maxIterations,
    });
    const outputPath = cli.outputPath
        ? path.resolve(process.cwd(), cli.outputPath)
        : defaultHarnessTranscriptPath(transcript.scenario_name);
    writeHarnessTranscript(outputPath, transcript);
    console.log(summarizeHarnessTranscript(transcript));
    console.log(`Transcript: ${outputPath}`);
}
main().catch((error) => {
    console.error(`[RiftbornAI] Headless harness failed: ${error instanceof Error ? error.message : String(error)}`);
    process.exit(1);
});
