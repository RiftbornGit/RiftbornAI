/**
 * Crash Diagnosis Engine — reads UE crash logs directly from disk.
 *
 * This runs entirely in the MCP server (Node.js) without the C++ bridge,
 * because when UE crashes the bridge is DOWN.  The tool parses:
 *   - Saved/Crashes/<folder>/*.log   (crash minidump logs)
 *   - Saved/Logs/RiftbornAI.log      (editor log written before death)
 *
 * Returns structured JSON: exception type, faulting frame, stack trace,
 * repair guidance, and the raw crash context lines.
 */
export interface StackFrame {
    function: string;
    file: string;
    line: number;
}
export interface CrashDiagnosis {
    status: string;
    exception_type?: string;
    faulting_frame?: StackFrame | null;
    stack_frames?: StackFrame[];
    stack_frame_count?: number;
    repair_directive?: string;
    context_lines?: string[];
    source_log?: string;
    searched_dir?: string;
    error?: string;
    crash_age_seconds?: number;
}
/**
 * Resolve the UE project directory from (in priority order):
 * 1. Explicit `project_dir` argument
 * 2. RIFTBORN_PROJECT_DIR environment variable
 * 3. Auto-detection by walking up from __dirname
 */
export declare function resolveProjectDir(explicit?: string): string | null;
export declare function diagnoseCrash(projectDir?: string, maxAge?: number): CrashDiagnosis;
//# sourceMappingURL=crash-diagnosis.d.ts.map