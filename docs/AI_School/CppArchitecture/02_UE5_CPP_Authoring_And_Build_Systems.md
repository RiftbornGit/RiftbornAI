# 02 — UE5 C++ Authoring And Build Systems

This repo has a real C++ generation and build-verification lane. Use it deliberately.

## Generation Tools Reflect Architecture Choices

The current supported class-generation surface includes:

- `generate_actor_class_from_description` — world entities with transform, tick, components
- `generate_character_class_from_description` — locomotion pawns with movement, mesh, capsule
- `generate_component_class_from_description` — reusable behaviors attached to any actor
- `generate_gamemode_class_from_description` — match rules, spawning policy, scoring
- `generate_subsystem_class_from_description` — scoped services (GameInstance, World, LocalPlayer)

These are not interchangeable.

The tool name you choose should reflect the class boundary you already decided on. If you are unsure of the boundary, resolve that decision first — see 01_Class_Boundaries.

```
┌─────────────────────────────────────────────────────┐
│ Architecture Decision                                │
│ "What role does this code play?"                     │
│                                                      │
│   World entity ──► generate_actor_class              │
│   Locomotion pawn ──► generate_character_class        │
│   Reusable behavior ──► generate_component_class      │
│   Match rules ──► generate_gamemode_class             │
│   Scoped service ──► generate_subsystem_class         │
│   Bounded file edit ──► create/edit_source_file       │
│   Blueprint migration ──► check + convert             │
└─────────────────────────────────────────────────────┘
```

## Direct Source Editing Exists Too

The current source-file tools include:

- `create_source_file`
- `edit_source_file`

Use them when:

- a generator does not fit the exact edit
- you need to patch an existing class directly
- you are making a bounded source change rather than generating a new class from scratch

## Blueprint-To-C++ Is A Distinct Lane

The current conversion surface includes:

- `check_blueprint_convertibility`
- `convert_blueprint_to_cpp`

This matters because conversion is only safe when you first understand what the converter can and cannot cover.

Use `check_blueprint_convertibility` before promising a full migration path.

## Build And Reload Lanes Are Different

The supported build/reload tools include:

- `get_build_status` — check whether the current module state is clean
- `get_build_errors` — retrieve specific compile errors
- `hot_reload_cpp` — explicit module reload for supported changes
- `trigger_live_coding` — incremental patch while editor is running
- `run_ubt` — full UnrealBuildTool invocation

```
┌──────────────────────────────────────────────────────┐
│ Build Selection Logic                                 │
│                                                       │
│   New class or header change?                         │
│     YES ──► run_ubt (full rebuild)                    │
│     NO  ──► Is the editor running?                    │
│               YES ──► trigger_live_coding              │
│               NO  ──► run_ubt                          │
│                                                       │
│   Live Coding failed or produced stale behavior?      │
│     ──► run_ubt (do not retry Live Coding blindly)    │
│                                                       │
│   After any build that returns errors?                │
│     ──► get_build_errors -> diagnose -> fix -> rebuild │
└──────────────────────────────────────────────────────┘
```

### `trigger_live_coding`

Use when the editor is running and the change is a .cpp body edit — function implementations, logic fixes, constant changes. Live Coding patches the running binary without restarting the editor.

Live Coding does NOT support: new classes, new UPROPERTY/UFUNCTION declarations, header-structural changes, module dependency changes.

### `hot_reload_cpp`

Use when you need an explicit reload path for C++ modules on the supported hot-reload lane. Less common than Live Coding but available for specific reload scenarios.

### `run_ubt`

Use for:

- new classes
- header or UHT-sensitive changes (UPROPERTY, UFUNCTION, UCLASS macros)
- changes that Live Coding does not support cleanly
- more reliable full module rebuilds
- first build after generation

Do not treat every compile like a Live Coding problem. When in doubt, `run_ubt` is always correct — Live Coding is a speed optimization, not a replacement.

## Validation And Packaging Exist Beyond Compile

The current project validation lane also includes:

- `validate_for_packaging`
- `cook_project`
- `package_project`

These are not normal iteration tools, but they matter when code changes need shipping-readiness proof.

## Crash And Failure Diagnostics Are First-Class

The repo also exposes:

- `diagnose_crash`

This is especially important after compile, reload, PIE, or other UE-side failures. The workflow is designed around diagnosis before retry.

## Runtime Verification Still Matters

C++ work is not done at successful compile.

For gameplay-facing code, use:

- `run_quick_playtest`

This proves the class still behaves in a running session rather than only satisfying the compiler.

## Key Takeaway

The C++ authoring surface has five layers:

1. class-boundary choice
2. generation or direct source editing
3. build/reload selection
4. diagnostics on failure
5. runtime verification

Skipping any one of those layers is how “compiling code” becomes bad architecture.
