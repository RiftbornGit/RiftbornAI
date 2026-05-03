import type { RiftbornResponse } from "./riftborn-types.js";
import { type ReadinessTier } from "./tool-readiness.js";
export interface HeadlessHarnessMockTool {
    tool?: string;
    bridge_tool?: string;
    top_level_tool?: string;
    args?: Record<string, unknown>;
    times?: number;
    response: RiftbornResponse;
}
export interface HeadlessHarnessMockHttp {
    method?: "GET" | "POST";
    path: string;
    body?: Record<string, unknown>;
    top_level_tool?: string;
    times?: number;
    response: RiftbornResponse;
}
export interface HeadlessHarnessSubagentProfile {
    instructions?: string;
    model?: string;
    max_iterations?: number;
    max_output_tokens?: number;
    reasoning_effort?: string;
    tool_tiers?: ReadinessTier[];
    tool_names?: string[];
    mock_tools?: HeadlessHarnessMockTool[];
    mock_http?: HeadlessHarnessMockHttp[];
}
export interface HeadlessHarnessAgentTaskRuntime {
    execution_mode?: "immediate" | "deferred";
    progression_mode?: "poll" | "time";
    complete_after_polls?: number;
    start_after_ms?: number;
    complete_after_ms?: number;
    initial_status?: "queued" | "running";
}
export interface HeadlessHarnessBenchmarkToolPathExpectation {
    required_tools?: string[];
    ordered_subsequence?: string[];
    forbidden_tools?: string[];
    ideal_total_calls?: number;
    max_total_calls?: number;
}
export interface HeadlessHarnessBenchmarkDelegationExpectation {
    expected_mode?: "direct" | "subagent" | "agent_task";
    child_required_tools?: string[];
    child_ordered_subsequence?: string[];
    child_forbidden_tools?: string[];
}
export interface HeadlessHarnessBenchmarkFinalOutputExpectation {
    required_phrases?: string[];
    forbidden_phrases?: string[];
}
export interface HeadlessHarnessBenchmarkExpectations {
    pass_threshold?: number;
    tool_path?: HeadlessHarnessBenchmarkToolPathExpectation;
    delegation?: HeadlessHarnessBenchmarkDelegationExpectation;
    final_output?: HeadlessHarnessBenchmarkFinalOutputExpectation;
}
export interface HeadlessHarnessScenario {
    name?: string;
    prompt: string;
    instructions?: string;
    model?: string;
    max_iterations?: number;
    max_output_tokens?: number;
    reasoning_effort?: string;
    tool_tiers?: ReadinessTier[];
    tool_names: string[];
    bridge_mode?: "mock" | "live";
    bridge_host?: string;
    bridge_http_port?: number;
    mock_tools?: HeadlessHarnessMockTool[];
    mock_http?: HeadlessHarnessMockHttp[];
    max_subagent_depth?: number;
    subagent_profiles?: Record<string, HeadlessHarnessSubagentProfile>;
    agent_task_profile?: HeadlessHarnessSubagentProfile;
    agent_task_runtime?: HeadlessHarnessAgentTaskRuntime;
    benchmark?: HeadlessHarnessBenchmarkExpectations;
}
export interface HarnessModelRequest {
    model: string;
    instructions?: string;
    input: unknown;
    tools: Array<Record<string, unknown>>;
    previousResponseId?: string;
    maxOutputTokens?: number;
    reasoningEffort?: string;
}
export interface HarnessModelFunctionCall {
    id?: string;
    callId: string;
    name: string;
    argumentsText: string;
    parsedArguments?: Record<string, unknown>;
    parseError?: string;
}
export interface HarnessModelResponse {
    id?: string;
    requestId?: string;
    clientRequestId?: string;
    outputText: string;
    functionCalls: HarnessModelFunctionCall[];
    usage?: Record<string, unknown>;
    raw: Record<string, unknown>;
    rawOutputItems: unknown[];
}
export interface HarnessModelClient {
    createResponse(request: HarnessModelRequest): Promise<HarnessModelResponse>;
}
export interface HarnessToolExecutionRecord {
    depth: number;
    iteration: number;
    call_id: string;
    requested_tool: string;
    arguments_text: string;
    parsed_arguments?: Record<string, unknown>;
    parse_error?: string;
    dispatch_result: RiftbornResponse;
    session_entries: unknown[];
    pipeline_traces: unknown[];
    mock_backend_log: unknown[];
    subagent_run_indices?: number[];
    agent_task_run_indices?: number[];
}
export interface HarnessIterationRecord {
    iteration: number;
    response_id?: string;
    request_id?: string;
    client_request_id?: string;
    output_text: string;
    function_calls: Array<Record<string, unknown>>;
    usage?: Record<string, unknown>;
    raw_output_items: unknown[];
}
export interface HarnessSubagentRunRecord {
    parent_iteration: number;
    parent_call_id: string;
    parent_tool: string;
    depth: number;
    task: string;
    profile: string;
    transcript: HeadlessHarnessTranscript;
}
export interface HarnessAgentTaskRunRecord {
    parent_iteration: number;
    parent_call_id: string;
    parent_tool: string;
    depth: number;
    task_id: string;
    session_id: string;
    request: string;
    status: string;
    transcript?: HeadlessHarnessTranscript;
    error?: string;
}
export interface HeadlessHarnessTranscript {
    depth: number;
    parent_call_id?: string;
    scenario_name: string;
    prompt: string;
    instructions: string;
    model: string;
    started_at: string;
    completed_at: string;
    stop_reason: "completed" | "max_iterations";
    max_iterations: number;
    tool_names: string[];
    visible_tools: Array<Record<string, unknown>>;
    iterations: HarnessIterationRecord[];
    tool_executions: HarnessToolExecutionRecord[];
    subagent_runs: HarnessSubagentRunRecord[];
    agent_task_runs: HarnessAgentTaskRunRecord[];
    final_output_text: string;
    session_history: Record<string, unknown>;
    pipeline_trace_report: Record<string, unknown>;
    mock_backend_log: unknown[];
}
export declare function parseHeadlessHarnessScenario(raw: unknown): HeadlessHarnessScenario;
export declare class OpenAIResponsesClient implements HarnessModelClient {
    private readonly fetchImpl;
    private readonly apiKey;
    private readonly baseUrl;
    private readonly organization?;
    private readonly project?;
    constructor(options?: {
        apiKey?: string;
        baseUrl?: string;
        organization?: string;
        project?: string;
        fetchImpl?: typeof fetch;
    });
    createResponse(request: HarnessModelRequest): Promise<HarnessModelResponse>;
}
export interface RunHeadlessHarnessOptions {
    scenario: HeadlessHarnessScenario;
    client: HarnessModelClient;
    model?: string;
    maxIterations?: number;
    depth?: number;
    parentCallId?: string;
}
export declare function runHeadlessAgentHarness(options: RunHeadlessHarnessOptions): Promise<HeadlessHarnessTranscript>;
export declare function defaultHarnessTranscriptPath(scenarioName: string): string;
export declare function writeHarnessTranscript(filePath: string, transcript: HeadlessHarnessTranscript): string;
export declare function summarizeHarnessTranscript(transcript: HeadlessHarnessTranscript): string;
export declare function loadHeadlessHarnessScenario(filePath: string): HeadlessHarnessScenario;