/**
 * Round 18 — Session Persistence
 *
 * Serializes key pipeline state to a temp file so MCP reconnections
 * don't lose context.  Four state sources are persisted:
 *
 *   1. ContextPropagator  — last-known output values (actor labels, paths, etc.)
 *   2. SessionTracker     — recent call ring buffer + counters
 *   3. SceneChangeLog     — tracked scene mutations + category counts
 *   4. ProgressTracker    — build-phase milestones
 *
 * State is written lazily (debounced) after each tool dispatch.
 * On startup, state is restored only if the file is < MAX_AGE_MS old.
 */
import * as fs from "fs";
import * as os from "os";
import * as path from "path";
import * as crypto from "crypto";
import { sanitizeSessionEntry } from "./system-enhancements.js";
import { createSanitizer } from "./sanitize-utils.js";
// ────────────────────────────────────────────────────────────────────────────
// Config
// ────────────────────────────────────────────────────────────────────────────
const STATE_FILENAME = "riftborn-session-state.json";
const MAX_AGE_MS = 5 * 60 * 1000; // 5 minutes — stale state is discarded
const DEBOUNCE_MS = 2_000; // coalesce writes within 2 s
const STATE_VERSION = 1; // bump when schema changes
const MAX_RESTORE_ITEMS = 500; // cap restored array sizes to prevent OOM
const MAX_STATE_FILE_BYTES = 1_048_576; // 1 MiB cap for untrusted tmp-file reads
// ────────────────────────────────────────────────────────────────────────────
// Path helper
// ────────────────────────────────────────────────────────────────────────────
function defaultStatePath() {
    const scope = crypto
        .createHash("sha256")
        .update(process.cwd())
        .digest("hex")
        .slice(0, 12);
    return path.join(os.tmpdir(), `${scope}-${STATE_FILENAME}`);
}
function inspectSnapshotPath(filePath) {
    try {
        const stats = fs.lstatSync(filePath);
        if (stats.isSymbolicLink()) {
            return null;
        }
        return stats;
    }
    catch {
        return null;
    }
}
function pathExists(filePath) {
    try {
        fs.lstatSync(filePath);
        return true;
    }
    catch {
        return false;
    }
}
const sanitizeSnapshotValue = createSanitizer({ maxDepth: 20, trackCircular: true });
function sanitizeLoadedSnapshot(value) {
    const sanitized = sanitizeSnapshotValue(value);
    if (!sanitized || typeof sanitized !== "object" || Array.isArray(sanitized)) {
        return null;
    }
    const snapshot = sanitized;
    if (typeof snapshot.version !== "number" || typeof snapshot.saved_at !== "number") {
        return null;
    }
    const contextRaw = snapshot.context && typeof snapshot.context === "object" && !Array.isArray(snapshot.context)
        ? snapshot.context
        : {};
    const context = {};
    for (const [key, entry] of Object.entries(contextRaw)) {
        if (typeof entry === "string") {
            context[key] = entry;
        }
    }
    const sessionRaw = snapshot.session && typeof snapshot.session === "object" && !Array.isArray(snapshot.session)
        ? snapshot.session
        : {};
    const sceneRaw = snapshot.scene && typeof snapshot.scene === "object" && !Array.isArray(snapshot.scene)
        ? snapshot.scene
        : {};
    const progressRaw = snapshot.progress && typeof snapshot.progress === "object" && !Array.isArray(snapshot.progress)
        ? snapshot.progress
        : {};
    const countsRaw = sceneRaw.counts && typeof sceneRaw.counts === "object" && !Array.isArray(sceneRaw.counts)
        ? sceneRaw.counts
        : {};
    const counts = {};
    for (const [key, entry] of Object.entries(countsRaw)) {
        if (typeof entry === "number" && Number.isFinite(entry)) {
            counts[key] = Math.max(0, Math.trunc(entry));
        }
    }
    return {
        version: snapshot.version,
        saved_at: snapshot.saved_at,
        context,
        session: {
            buffer: Array.isArray(sessionRaw.buffer)
                ? sessionRaw.buffer
                    .slice(0, 500) // Cap before mapping to prevent DoS from crafted snapshot
                    .map(sanitizeSessionEntry)
                    .filter((e) => e !== null)
                : [],
            totalCalls: typeof sessionRaw.totalCalls === "number" && Number.isFinite(sessionRaw.totalCalls)
                ? Math.max(0, Math.trunc(sessionRaw.totalCalls))
                : 0,
            totalErrors: typeof sessionRaw.totalErrors === "number" && Number.isFinite(sessionRaw.totalErrors)
                ? Math.max(0, Math.trunc(sessionRaw.totalErrors))
                : 0,
        },
        scene: {
            changes: Array.isArray(sceneRaw.changes) ? sceneRaw.changes : [],
            nextId: typeof sceneRaw.nextId === "number" && Number.isFinite(sceneRaw.nextId)
                ? Math.max(1, Math.trunc(sceneRaw.nextId))
                : 1,
            counts,
        },
        progress: {
            milestones: Array.isArray(progressRaw.milestones) ? progressRaw.milestones : [],
        },
    };
}
function shouldRestoreScopedContext(context, currentLevelName) {
    const observedLevel = typeof context.last_observed_level === "string"
        ? context.last_observed_level.trim()
        : "";
    const normalizedCurrentLevel = typeof currentLevelName === "string"
        ? currentLevelName.trim()
        : "";
    if (!observedLevel || !normalizedCurrentLevel) {
        return false;
    }
    return observedLevel === normalizedCurrentLevel;
}
export function collectSnapshot(sources) {
    return {
        version: STATE_VERSION,
        saved_at: Date.now(),
        context: sources.contextPropagator.snapshot(),
        session: sources.sessionTracker.serialize(),
        scene: sources.sceneChangeLog.serialize(),
        progress: { milestones: sources.progressTracker.serialize() },
    };
}
// ────────────────────────────────────────────────────────────────────────────
// Restore from snapshot into live instances
// ────────────────────────────────────────────────────────────────────────────
export function applySnapshot(snapshot, targets, opts) {
    if (!snapshot || typeof snapshot !== "object") {
        return { restored: false, reason: "invalid_snapshot" };
    }
    if (snapshot.version !== STATE_VERSION) {
        return { restored: false, reason: `version_mismatch: expected ${STATE_VERSION}, got ${snapshot.version}` };
    }
    const age = Date.now() - snapshot.saved_at;
    if (age > MAX_AGE_MS) {
        return { restored: false, reason: `stale: age ${Math.round(age / 1000)}s exceeds ${MAX_AGE_MS / 1000}s limit` };
    }
    const snapshotContext = snapshot.context ?? {};
    targets.contextPropagator.restore(shouldRestoreScopedContext(snapshotContext, opts?.currentLevelName) ? snapshotContext : {});
    const session = snapshot.session ?? { buffer: [], totalCalls: 0, totalErrors: 0 };
    if (Array.isArray(session.buffer))
        session.buffer = session.buffer.slice(-MAX_RESTORE_ITEMS);
    targets.sessionTracker.restore(session);
    const scene = snapshot.scene ?? { changes: [], nextId: 1, counts: {} };
    if (Array.isArray(scene.changes))
        scene.changes = scene.changes.slice(-MAX_RESTORE_ITEMS);
    targets.sceneChangeLog.restore(scene);
    const milestones = snapshot.progress?.milestones ?? [];
    targets.progressTracker.restoreFromMilestones(Array.isArray(milestones) ? milestones.slice(-MAX_RESTORE_ITEMS) : []);
    return { restored: true, age_ms: age };
}
// ────────────────────────────────────────────────────────────────────────────
// File I/O
// ────────────────────────────────────────────────────────────────────────────
export function saveSnapshot(snapshot, filePath) {
    let tempDir = null;
    let tmp = null;
    try {
        const p = filePath ?? defaultStatePath();
        const existing = inspectSnapshotPath(p);
        if (!existing && pathExists(p)) {
            return false;
        }
        if (existing && !existing.isFile()) {
            return false;
        }
        const dir = path.dirname(p);
        fs.mkdirSync(dir, { recursive: true });
        // Write to a unique temp directory in the same parent to avoid predictable tmp names.
        tempDir = fs.mkdtempSync(path.join(dir, `${path.basename(p)}.`));
        tmp = path.join(tempDir, path.basename(p));
        fs.writeFileSync(tmp, JSON.stringify(snapshot), {
            encoding: "utf-8",
            mode: 0o600,
            flag: "wx",
        });
        fs.renameSync(tmp, p);
        return true;
    }
    catch {
        return false;
    }
    finally {
        if (tmp && fs.existsSync(tmp)) {
            try {
                fs.unlinkSync(tmp);
            }
            catch { /* ignore cleanup */ }
        }
        if (tempDir && fs.existsSync(tempDir)) {
            try {
                fs.rmdirSync(tempDir);
            }
            catch { /* ignore cleanup */ }
        }
    }
}
export function loadSnapshot(filePath) {
    try {
        const p = filePath ?? defaultStatePath();
        const stats = inspectSnapshotPath(p);
        if (!stats || !stats.isFile() || stats.size > MAX_STATE_FILE_BYTES)
            return null;
        const raw = fs.readFileSync(p, "utf-8");
        const parsed = JSON.parse(raw);
        return sanitizeLoadedSnapshot(parsed);
    }
    catch {
        return null;
    }
}
export function deleteSnapshot(filePath) {
    try {
        const p = filePath ?? defaultStatePath();
        const stats = inspectSnapshotPath(p);
        if (stats?.isFile())
            fs.unlinkSync(p);
    }
    catch {
        // Ignore — cleanup is best-effort
    }
}
// ────────────────────────────────────────────────────────────────────────────
// SessionPersistence — debounced writer + startup restorer
// ────────────────────────────────────────────────────────────────────────────
export class SessionPersistence {
    sources;
    filePath;
    timer = null;
    debounceMs;
    constructor(sources, opts) {
        this.sources = sources;
        this.filePath = opts?.filePath ?? defaultStatePath();
        this.debounceMs = opts?.debounceMs ?? DEBOUNCE_MS;
    }
    /**
     * Try to restore state from disk.
     * Returns result object indicating success/failure + age.
     */
    tryRestore(opts) {
        const snapshot = loadSnapshot(this.filePath);
        if (!snapshot)
            return { restored: false, reason: "no_file" };
        return applySnapshot(snapshot, this.sources, opts);
    }
    /**
     * Schedule a debounced write.  Safe to call after every tool dispatch —
     * only one write happens within the debounce window.
     */
    scheduleSave() {
        if (this.timer)
            clearTimeout(this.timer);
        this.timer = setTimeout(() => {
            this.timer = null;
            this.saveNow();
        }, this.debounceMs);
    }
    /** Force an immediate write (e.g. on graceful shutdown). */
    saveNow() {
        return saveSnapshot(collectSnapshot(this.sources), this.filePath);
    }
    /** Cancel any pending write and remove the state file. */
    dispose() {
        if (this.timer) {
            clearTimeout(this.timer);
            this.timer = null;
        }
        deleteSnapshot(this.filePath);
    }
    /** Expose for tests. */
    get stateFilePath() {
        return this.filePath;
    }
}
//# sourceMappingURL=session-persistence.js.map