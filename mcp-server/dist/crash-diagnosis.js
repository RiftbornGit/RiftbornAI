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
import * as fs from "fs";
import * as path from "path";
// ── Regexes ───────────────────────────────────────────────────────────────────
const EXCEPTION_RE = /Unhandled Exception: (EXCEPTION_\w+)/;
const FATAL_ERROR_RE = /Fatal error: \[File:([^\]]+)\] \[Line: (\d+)\]/;
const ASSERTION_RE = /Assertion failed: (.+?) \[File:([^\]]+)\] \[Line: (\d+)\]/;
const ENSURE_RE = /Ensure condition failed: (.+?) \[File:([^\]]+)\]/;
const GPU_CRASH_RE = /GPU (?:Crashed|Hung|Timeout)/i;
const FILE_LINE_RE = /(?<file>[A-Za-z0-9_./\\-]+\.(?:cpp|h|inl)):(?<line>\d+)/g;
const FUNCTION_RE = /(?<fn>[A-Za-z_][A-Za-z0-9_:<>~]*)\s*(?:\(|\[)/;
// ── Project path resolution ───────────────────────────────────────────────────
/**
 * Walk up from the MCP server directory to find the UE project root
 * (directory containing a .uproject file).
 */
function findProjectRoot() {
    // Start from this file's directory, walk up
    let dir = path.resolve(path.dirname(new URL(import.meta.url).pathname.replace(/^\/([A-Z]:)/, "$1")));
    for (let i = 0; i < 10; i++) {
        try {
            const entries = fs.readdirSync(dir);
            if (entries.some((e) => e.endsWith(".uproject")))
                return dir;
        }
        catch { /* permission error or doesn't exist */ }
        const parent = path.dirname(dir);
        if (parent === dir)
            break;
        dir = parent;
    }
    return null;
}
/**
 * Resolve the UE project directory from (in priority order):
 * 1. Explicit `project_dir` argument
 * 2. RIFTBORN_PROJECT_DIR environment variable
 * 3. Auto-detection by walking up from __dirname
 */
export function resolveProjectDir(explicit) {
    if (explicit && fs.existsSync(explicit)) {
        // Validate that the explicit path actually contains a .uproject file
        try {
            const entries = fs.readdirSync(explicit);
            if (!entries.some((e) => e.endsWith(".uproject")))
                return null;
        }
        catch {
            return null;
        }
        return explicit;
    }
    const envDir = process.env.RIFTBORN_PROJECT_DIR;
    if (envDir && fs.existsSync(envDir))
        return envDir;
    return findProjectRoot();
}
function findCrashLogs(projectDir) {
    const crashDir = path.join(projectDir, "Saved", "Crashes");
    const logsDir = path.join(projectDir, "Saved", "Logs");
    const logs = [];
    // Saved/Crashes/<subdir>/*.log
    if (fs.existsSync(crashDir)) {
        try {
            for (const subdir of fs.readdirSync(crashDir)) {
                const full = path.join(crashDir, subdir);
                if (!fs.statSync(full).isDirectory())
                    continue;
                for (const file of fs.readdirSync(full)) {
                    if (!file.endsWith(".log"))
                        continue;
                    const fp = path.join(full, file);
                    const lst = fs.lstatSync(fp);
                    if (lst.isSymbolicLink())
                        continue;
                    logs.push({ path: fp, mtime: lst.mtimeMs });
                }
            }
        }
        catch { /* access error */ }
    }
    // Saved/Logs/*.log
    if (fs.existsSync(logsDir)) {
        try {
            for (const file of fs.readdirSync(logsDir)) {
                if (!file.endsWith(".log"))
                    continue;
                const fp = path.join(logsDir, file);
                const lst = fs.lstatSync(fp);
                if (lst.isSymbolicLink())
                    continue;
                logs.push({ path: fp, mtime: lst.mtimeMs });
            }
        }
        catch { /* access error */ }
    }
    // Most recent first
    logs.sort((a, b) => b.mtime - a.mtime);
    return logs;
}
// ── Stack frame extraction ────────────────────────────────────────────────────
function extractStackFrames(text) {
    const frames = [];
    const seen = new Set();
    for (const line of text.split("\n")) {
        FILE_LINE_RE.lastIndex = 0;
        const m = FILE_LINE_RE.exec(line);
        if (!m || !m.groups)
            continue;
        const file = path.basename(m.groups.file);
        const lineNo = parseInt(m.groups.line, 10);
        const key = `${file}:${lineNo}`;
        if (seen.has(key))
            continue;
        seen.add(key);
        const fm = FUNCTION_RE.exec(line);
        frames.push({
            function: fm?.groups?.fn ?? "Unknown",
            file,
            line: lineNo,
        });
    }
    return frames;
}
// ── Context extraction ────────────────────────────────────────────────────────
function extractCrashContext(text, maxLines = 30) {
    const lines = text.split("\n");
    // Find the crash/fatal/assertion line and grab surrounding context
    for (let i = 0; i < lines.length; i++) {
        if (EXCEPTION_RE.test(lines[i]) ||
            FATAL_ERROR_RE.test(lines[i]) ||
            ASSERTION_RE.test(lines[i]) ||
            ENSURE_RE.test(lines[i]) ||
            GPU_CRASH_RE.test(lines[i])) {
            const start = Math.max(0, i - 5);
            const end = Math.min(lines.length, i + maxLines);
            return lines.slice(start, end).map((l) => l.trimEnd());
        }
    }
    // Fallback: last N lines (often contain the crash)
    return lines.slice(-maxLines).map((l) => l.trimEnd());
}
// ── Repair guidance ───────────────────────────────────────────────────────────
function generateRepairDirective(exceptionType, rootFrame, rawText) {
    const loc = rootFrame ? `${rootFrame.file}:${rootFrame.line} (${rootFrame.function})` : "unknown location";
    if (exceptionType === "EXCEPTION_ACCESS_VIOLATION") {
        return `Null pointer dereference at ${loc}. Guard pointer access with IsValid() / nullptr check before dereferencing.`;
    }
    if (exceptionType === "EXCEPTION_STACK_OVERFLOW") {
        return `Stack overflow at ${loc}. Break infinite recursion — check for cyclic calls or unbounded recursion depth.`;
    }
    // Assertion / Ensure
    const assertionMatch = ASSERTION_RE.exec(rawText);
    if (assertionMatch) {
        return `Assertion failed: "${assertionMatch[1].substring(0, 200)}" at ${loc}. Check the asserted condition and fix the violated invariant.`;
    }
    const ensureMatch = ENSURE_RE.exec(rawText);
    if (ensureMatch) {
        return `Ensure condition failed: "${ensureMatch[1].substring(0, 200)}" at ${loc}. This is a soft crash — add proper validation before this code path.`;
    }
    // Fatal error
    if (FATAL_ERROR_RE.test(rawText)) {
        return `Fatal error at ${loc}. Read the context lines for the exact error message and fix the triggering condition.`;
    }
    // GPU crash
    if (GPU_CRASH_RE.test(rawText)) {
        return `GPU crash/hang detected. Check for infinite shader loops, excessive draw calls, or driver incompatibility. Try reducing material complexity or disabling Nanite/Lumen temporarily.`;
    }
    return `Crash at ${loc}. Read the context lines and stack frames to diagnose root cause.`;
}
// ── Main diagnosis ────────────────────────────────────────────────────────────
export function diagnoseCrash(projectDir, maxAge) {
    const dir = resolveProjectDir(projectDir);
    if (!dir) {
        return {
            status: "PROJECT_DIR_NOT_FOUND",
            error: "Could not locate the UE project directory. Pass project_dir explicitly or set RIFTBORN_PROJECT_DIR.",
        };
    }
    const logs = findCrashLogs(dir);
    if (logs.length === 0) {
        return {
            status: "NO_CRASH_LOGS",
            searched_dir: path.join(dir, "Saved"),
        };
    }
    // Use most recent log
    const latest = logs[0];
    const ageSeconds = (Date.now() - latest.mtime) / 1000;
    // If caller specified maxAge, skip stale logs
    if (maxAge && ageSeconds > maxAge) {
        return {
            status: "NO_RECENT_CRASH",
            crash_age_seconds: Math.round(ageSeconds),
            source_log: latest.path,
        };
    }
    let rawText;
    try {
        const MAX_CRASH_LOG_BYTES = 20 * 1024 * 1024; // 20 MiB
        const logStat = fs.statSync(latest.path);
        if (logStat.size > MAX_CRASH_LOG_BYTES) {
            return {
                status: "LOG_TOO_LARGE",
                source_log: latest.path,
                crash_age_seconds: Math.round(ageSeconds),
            };
        }
        rawText = fs.readFileSync(latest.path, { encoding: "utf-8" });
    }
    catch (e) {
        return { status: "IO_ERROR", error: String(e), source_log: latest.path };
    }
    // Detect crash type
    const exMatch = EXCEPTION_RE.exec(rawText);
    const fatalMatch = FATAL_ERROR_RE.exec(rawText);
    const assertMatch = ASSERTION_RE.exec(rawText);
    const ensureMatch = ENSURE_RE.exec(rawText);
    const gpuMatch = GPU_CRASH_RE.exec(rawText);
    const hasCrash = exMatch || fatalMatch || assertMatch || ensureMatch || gpuMatch;
    if (!hasCrash) {
        return {
            status: "NO_CRASH_SIGNATURE",
            source_log: latest.path,
            crash_age_seconds: Math.round(ageSeconds),
        };
    }
    const exceptionType = exMatch
        ? exMatch[1]
        : assertMatch
            ? "ASSERTION_FAILED"
            : ensureMatch
                ? "ENSURE_FAILED"
                : fatalMatch
                    ? "FATAL_ERROR"
                    : "GPU_CRASH";
    const frames = extractStackFrames(rawText);
    const rootFrame = frames.length > 0 ? frames[0] : null;
    const contextLines = extractCrashContext(rawText);
    const directive = generateRepairDirective(exceptionType, rootFrame, rawText);
    return {
        status: "CRASH_DIAGNOSED",
        exception_type: exceptionType,
        faulting_frame: rootFrame,
        stack_frames: frames.slice(0, 20),
        stack_frame_count: frames.length,
        repair_directive: directive,
        context_lines: contextLines,
        source_log: latest.path,
        crash_age_seconds: Math.round(ageSeconds),
    };
}
//# sourceMappingURL=crash-diagnosis.js.map