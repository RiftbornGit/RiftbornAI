# RiftbornAI v0.1.0 Beta — Status

What's in this release, what's known to misbehave, and what's still missing.
This document is updated per beta release.

**Build**: v0.1.0-UE5.7-Beta
**Engine**: Unreal Engine 5.7 (Editor only)
**Platforms**: Windows (Win64) — Editor binaries are precompiled.
**License**: BSL 1.1 → Apache 2.0 (4-year sunset). See [`LICENSE`](../LICENSE).

---

## What's solid

These surfaces have been used end-to-end in real builds and are safe to
rely on for prototyping work:

- **Beta surface** — shipped Beta builds expose a locked 99-tool workflow set
  spanning environment creation, playable character setup, simple AI,
  VFX+audio, cinematics, and package-readiness loops.

- **Environment building** — landscapes, weight-painted layers, procedural
  foliage, atmospheric fog, water bodies, post-process volumes.
- **Lighting** — directional / point / spot / rect lights, sky atmosphere,
  exponential height fog. Vision loop verifies mood against the prompt.
- **Materials** — PBR graphs from texture maps, instance creation, parameter
  edits, landscape multi-layer blends.
- **Blueprints** — creation, component wiring, variable/event/function
  scaffolding, compilation. Round-trip through the BlueprintGraph API is
  reliable.
- **Characters & gameplay** — third-person setup, navigation, AI perception,
  basic GAS abilities.
- **Cinematics** — Level Sequence creation, camera animation, keyframing,
  playback control.
- **Tool SDK** — third-party plugins can register tools at runtime. See
  [`TOOL_AUTHORING.md`](TOOL_AUTHORING.md) and the reference plugin in
  [`Examples/RiftbornAI-ExampleTool/`](../Examples/RiftbornAI-ExampleTool).

---

## Known limitations

### Platform

- **Windows Editor only.** macOS and Linux editor binaries are not in this
  release; they come in the next minor version.
- **Editor-only.** Plugin is excluded from cooked / packaged builds. The AI
  works at authoring time; produced assets ship with your game normally.

### Engine

- **UE 5.7 only.** Built and tested against 5.7.0+. Will not load in 5.6
  or earlier; binary compatibility for 5.8 is not guaranteed until we
  rebuild against the new SDK.

### Update mechanism

- **In-editor auto-update is now staged.** RiftbornAI polls GitHub Releases
  after startup settles, downloads newer packaged release zips in the
  background, and queues a Windows updater that applies them automatically the
  next time the editor closes. Manual zip replacement remains the fallback
  path.

### Optional modules

Some tools require specific UE plugins to be enabled in the project. The
plugin gates them automatically — they don't appear in the registry if the
module is missing — but if you expect a tool to exist and don't see it,
check whether its dependency is enabled.

| Tool category | Required plugin |
|---|---|
| PCG graph authoring | `PCG`, `PCGGeometryScriptInterop` |
| GAS abilities / effects | `GameplayAbilities` |
| Niagara VFX | `Niagara` (always enabled) |
| StateTree | `StateTreeModule` |
| Pose search / motion matching | `PoseSearch` |
| Control Rig authoring | `ControlRig`, `ControlRigDeveloper` |
| Movie Render Pipeline | `MovieRenderPipelineCore`, `MovieRenderPipelineSettings` |
| Water bodies / splines | `Water` |
| Variant Manager | `VariantManager`, `VariantManagerContent`, `VariantManagerContentEditor` |

---

## Can RiftbornAI Beta (99 tools) do EVERYTHING needed to make a full game?

**No.**

It is an extremely powerful **editor-time co-pilot** that can rapidly build high-quality environments, lighting, materials, characters, Blueprints, basic VFX/GAS/AI, cinematics, and blockouts following AI School best practices. The 99-tool locked beta workflow is excellent for prototyping and authoring many core systems.

**It cannot replace a full production team or do "everything":**

- **Runtime / shipping**: No cooked-game runtime features, no packaged builds, no player-facing save systems, advanced networking (replication, Iris groups), performance optimization, or platform-specific deployment.
- **Advanced systems**: Full UI navigation/UMG flows, complex animation retargeting/pose search, advanced physics/destruction, localization, deep World Partition streaming, complex multiplayer authority, advanced audio mixing, and many GAS/ Niagara/Control Rig edge cases are only partially supported or require significant human design oversight.
- **Design reasoning**: Many domains still rely on the AI School documents for "why" decisions (player readability, affordances, anti-patterns). The current surface automates *execution* well but not all high-level game design or systems integration.
- **Scope**: Windows Editor only, UE 5.7 only, vision quotas/latency issues exist.

**Bottom line**: RiftbornAI Beta dramatically accelerates development and helps you avoid common mistakes, but a full shipped game still requires human direction, deep UE expertise, additional tools/plugins, and final polish/testing. It is best used as a highly capable Game Director assistant, not an autonomous replacement.

---

## Known issues

These are tracked and prioritized for the next release:

- **Vision provider quotas** — large agent sessions can hit Anthropic /
  OpenAI rate limits without graceful backoff. Workaround: split work
  into smaller prompts, or use Ollama for vision.
- **PCG tool latency** — some PCG graph operations take 5–15 s; the editor
  may appear briefly unresponsive while the graph compiles.
- **Long Niagara session warnings** — repeated Niagara system creates in a
  single session occasionally trigger asset-registry contention warnings.
  Restart the editor between long VFX sessions if you hit this.
- **Some tools require Editor focus** — viewport-screenshot tools may
  return blank captures if the editor window is fully occluded by another
  app. Bring the editor to front before vision-loop sequences.

---

## What's not in this beta yet

Planned for follow-up minor releases (no committed dates for v0.1.x):

- macOS and Linux editor binaries.
- In-editor auto-update from GitHub Releases.
- Native UE keyboard-shortcut integration for the copilot panel.
- Multi-project history sync.
- A built-in license-key activation flow (today: license is implicit in
  download / commercial agreement).

---

## How to report issues

GitHub Issues: https://github.com/RiftbornGit/RiftbornAI/issues

Useful in your report:

1. UE editor version (Help → About).
2. Your OS and architecture.
3. The exact prompt you sent.
4. The first `LogRiftbornAI` error in your Output Log.
5. The proof-bundle ID printed by the failing tool call (look for
   `proof_id=...` in the log).

For governance / security concerns specifically, see
[`SECURITY.md`](../SECURITY.md).
