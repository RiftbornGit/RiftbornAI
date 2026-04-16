## AI School Verification Ladder

Do not treat "asset exists" or "compile succeeded" as proof that the work is done.

RiftbornAI work should climb a verification ladder from authoring proof to runtime proof.

## The Ladder

### 1. Structure Proof

First prove that the edited structure is the one you intended to change.

Examples:

- inspect the active editor context before editing
- list the Blueprint graph, widget tree, Niagara modules, PCG nodes, or sequence bindings
- verify that the edited asset is the correct one

This prevents blind edits and wrong-asset mutations.

### 2. Compile Or Asset-Health Proof

Then prove the asset is healthy.

Examples:

- `assert_blueprint_compiles`
- `get_blueprint_compile_diagnostics`
- `assert_niagara_compiles`
- `assert_material_compiles`
- widget compile and layout audit

This is the minimum technical gate, not the final quality bar.

### 3. In-World Proof

Then prove the thing reads correctly in scene context.

Examples:

- `observe_ue_project`
- `capture_viewport_sync`
- `look_at_and_capture`
- placing the asset in the world and reviewing it at gameplay framing

This matters for VFX, audio placement, environment work, cinematics, animation framing, and level readability.

### 4. Runtime Interaction Proof

Then prove it behaves correctly when the game is actually running.

Examples:

- `run_quick_playtest`
- targeted PIE sessions
- UI interaction tests
- movement, ability, or traversal validation under real input

If the feature affects player timing, state changes, or interaction, runtime proof is mandatory.

### 5. Contract Proof

Then prove the cross-cutting contract still holds.

Examples:

- GAS state inspection with `get_gas_abilities`, `get_gas_attributes`, `get_active_effects`
- networking audits and actor replication inspection
- save/load restoration after snapshot or checkpoint capture
- localization key review and deliberate String Table save flow

This is where features stop being isolated assets and become real game systems.

### 6. Performance Or Regression Proof

Use this when the feature is heavy, risky, or near shipping-readiness.

Examples:

- `get_performance_report`
- memory or streaming diagnostics
- snapshots before destructive iteration
- render output proof for cinematics

Not every task needs this step, but expensive tasks usually do.

## Minimum Expected Proof By Domain

- `Environment`: in-world proof
- `Modeling`: mesh stats or structure proof + cleanup/export proof + in-world or collision proof as needed
- `VFX`: compile proof + in-world proof + playtest for gameplay-facing effects
- `GAS`: compile proof + actor assignment proof + runtime interaction proof
- `Blueprint`: compile proof + runtime interaction proof
- `Audio`: preview proof + in-world proof + playtest
- `UI`: compile/layout proof + PIE interaction proof
- `Animation`: asset proof + gameplay-distance visual proof + playtest
- `Input`: mapping proof + playtest
- `Cinematics`: binding proof + review proof + render proof when final output matters
- `Physics`: collision/trace proof + runtime interaction proof
- `Streaming`: live partition/layer proof + residency proof in context
- `Networking`: audit proof + actor-level replication proof + runtime net validation when needed
- `SaveLoad`: capture proof + restoration proof
- `Localization`: key and entry proof + deliberate save proof
- `LevelDesign`: viewport proof + nav or traversal proof + playtest
- `CppArchitecture`: build proof + runtime gameplay proof

## Common Failure Pattern

Most regressions come from stopping halfway up the ladder:

- compile succeeded, but the feature reads badly in-world
- the widget looks correct in the designer, but breaks in PIE
- the ability asset exists, but does not behave correctly on a real actor
- replication audit looks clean, but no one tested the actual gameplay path

## Operational Rule

Before claiming a task is done, state which steps of the ladder were actually completed.

If a step was not run, say so clearly.

That is especially important for:

- runtime playtest
- networking validation
- restoration verification
- performance checks

## Use The Smallest Sufficient Proof Set

Do not make proof theatrical.

Choose the smallest verification set that can actually falsify the main failure mode of the task. But do not skip the ladder stage where that failure mode lives.
