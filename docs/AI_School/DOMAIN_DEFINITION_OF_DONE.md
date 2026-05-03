# AI School Domain Definition Of Done

Use this after `TASK_INTAKE_AND_PREFLIGHT.md` and `VERIFICATION_LADDER.md`.

This document turns the verification ladder into concrete stop rules for the
primary authoring domains called out in the 90-day execution board. The goal is
to stop "compile passed" from being treated as finished work when the real
failure mode lives in runtime, world context, restore behavior, or performance.

## How To Use These Templates

For each task:

1. identify the primary mutation domain
2. load the matching track and supporting tracks
3. use the domain template below as the minimum done bar
4. add stricter proof only when the task is riskier than the template baseline

These templates are not permission to skip track-specific workflow docs. They
are the finish-line contract once the work has been authored.

## Blueprint

**Main failure mode**

Graph structure compiles, but the gameplay behavior is still wrong in PIE.

**Required proof**

- structure proof: `get_editor_focus_state`, `focus_asset_editor`, `get_blueprint_editor_context`, `list_blueprint_graphs`, `find_blueprint_nodes`
- compile proof: `get_blueprint_compile_diagnostics`, `assert_blueprint_compiles`, `validate_blueprint_health`
- runtime proof: `run_quick_playtest`
- rollback or review proof when surgery is non-trivial: `create_blueprint_snapshot`, `compare_blueprint_snapshots`

**Minimum done bar**

- the correct Blueprint and graph were inspected before mutation
- compile diagnostics are clean or intentionally explained
- runtime behavior was checked in PIE with the actual gameplay path
- destructive graph edits have a snapshot or equivalent before/after evidence

**Not done if**

- the Blueprint only compiles but the gameplay path was not exercised
- the wrong graph or wrong asset might have been edited
- the repair changed graph structure without a snapshot or comparison artifact

## LevelDesign

**Main failure mode**

The space looks reasonable in screenshots but traversal, sightlines, or cover
spacing break once the player moves through it.

**Required proof**

- structure proof: `observe_ue_project`, `capture_viewport_sync`, `look_at_and_capture`
- navigation proof: `build_navmesh`, `get_navmesh_status`
- runtime proof: `run_quick_playtest`
- map-state proof when iteration is branching: `save_level`, `save_level_as`

**Minimum done bar**

- the blockout or revised layout was reviewed from gameplay-relevant angles
- navmesh was rebuilt after traversal-affecting changes and its status checked
- the space was traversed in play, not only inspected from the editor camera
- verified progress was saved intentionally after proof

**Not done if**

- navmesh was not rebuilt after geometry changed
- the layout was judged only from still images or top-down framing
- movement through the space was not checked in runtime

## VFX

**Main failure mode**

The Niagara system compiles and looks good in isolation, but is unreadable,
noisy, or too expensive in gameplay context.

**Required proof**

- structure proof: `get_niagara_editor_context`, `get_niagara_stack_context`, `list_niagara_modules`
- compile proof: `assert_niagara_compiles`
- asset preview proof: `preview_niagara`
- in-world proof: `spawn_niagara_at_location` or `spawn_niagara_attached`, `look_at_and_capture`, `analyze_scene_screenshot`, `observe_ue_project`
- runtime proof for gameplay-facing effects: `run_quick_playtest`
- performance proof for near-final or gameplay-heavy effects: `get_performance_report`
- rollback proof for shared systems: `save_niagara_snapshot`

**Minimum done bar**

- the system compiles clean before world verification
- the effect was previewed, then checked at gameplay camera distance in world
- gameplay-facing effects were reviewed during motion or combat timing
- expensive or important effects had a performance snapshot before sign-off

**Not done if**

- compile passed but the effect was never reviewed in-world
- readability was checked only from close-up asset preview
- final sign-off happened without performance evidence on a gameplay effect

## GAS

**Main failure mode**

The ability and effect assets exist, but costs, cooldowns, stacking, actor
assignment, or replicated gameplay truth are wrong at runtime.

**Required proof**

- asset proof: `get_gas_assets`, `get_gameplay_tags`
- authoring proof: `create_gameplay_effect`, `configure_gameplay_effect`, `configure_ge_stacking`, `create_gameplay_ability`, `create_attribute_set`
- compile proof: `assert_blueprint_compiles`, `validate_blueprint_health`
- actor assignment proof: `add_ability_to_actor`, `set_actor_gameplay_tags`
- runtime proof: `run_quick_playtest`
- contract proof when multiplayer truth matters: `audit_net_replication`, `inspect_actor_replication`

**Minimum done bar**

- tags, attributes, abilities, and effects were verified as the intended asset set
- the authored Blueprints compile cleanly
- the ability or effect was assigned to a real actor with an ASC
- the gameplay path was exercised in PIE and repeated use confirmed costs, cooldowns, and stacking
- multiplayer-sensitive work includes replication review, not only single-player playtest

**Not done if**

- the asset set exists but no actor-level runtime proof was run
- costs or cooldowns changed without repeated-use playtest
- replication-sensitive ability work skipped network truth inspection

## UI

**Main failure mode**

The widget compiles and looks fine in the designer, but is invisible, unfocusable,
or broken during actual interaction in PIE.

**Required proof**

- structure proof: `get_widget_editor_context`, `list_widget_tree`
- compile and layout proof: `compile_widget_blueprint`, `verify_widget_blueprint_layout`
- visibility proof: `assert_widget_visible_in_pie`
- runtime state proof: `capture_ui_state`
- navigation proof for interactive screens: `set_widget_navigation_rule`, `get_widget_navigation_state`, `simulate_widget_navigation_in_pie`
- interaction proof for critical flows: `run_ui_flow_test`

**Minimum done bar**

- the widget tree and active editor context were inspected before restructure
- compile and layout audit passed after meaningful changes
- the widget was proven visible in PIE
- interactive screens have explicit navigation proof with real input simulation
- critical paths were run end to end, not just visually reviewed

**Not done if**

- the screen was only checked in the designer
- visibility in PIE was not proven
- interactive navigation or confirm/cancel flows were not tested

## Networking

**Main failure mode**

Replication cost looks acceptable in static audit, but actor-level authority,
relevance, or delivery truth is still wrong for the actual gameplay path.

**Required proof**

- audit proof: `audit_net_replication`
- actor truth proof: `inspect_actor_replication`
- grouping proof when Iris relevance is part of the task: `create_replication_group`, `add_actor_to_replication_group`, `set_replication_group_filter_status`
- runtime proof when a local public lane exists for the feature: pair the audit with the owning domain's runtime proof, usually `run_quick_playtest`

**Minimum done bar**

- project-wide audit was run for the affected classes or feature area
- at least one concrete actor or class instance was inspected for replication truth
- Iris grouping changes were proven against a valid replication context, not just authored spec
- multiplayer-facing feature work does not stop at static audit if the owning gameplay path can be exercised

**Not done if**

- only project-wide audit was run and no actor-level truth was inspected
- grouping tools were used without a valid replication context
- a multiplayer gameplay feature was called done without pairing networking proof with owning-domain runtime proof

## Save/Load

**Main failure mode**

The snapshot or checkpoint was captured, but restoration either does not return
the intended state or the evidence is too weak to prove what changed.

**Required proof**

- capture proof: `capture_level_snapshot` or `save_scene_checkpoint`
- state evidence: `get_world_state_digest`
- restoration proof: `restore_level_snapshot` or `restore_scene_checkpoint`
- save-state stabilization when needed: `save_level`, `save_level_as`, `save_asset`, `save_dirty_assets`, `load_level`

**Minimum done bar**

- the restore target was captured deliberately, not implicitly
- a digest or richer checkpoint artifact exists around the mutation
- restoration was actually performed and checked, not assumed from capture success
- claims stay within the supported editor-world snapshot and checkpoint lane

**Not done if**

- a checkpoint exists but no restore was attempted
- the task claims full player-save semantics instead of editor rollback semantics
- state evidence was skipped even though rollback confidence matters

## Performance

**Main failure mode**

The feature is functionally correct, but it pushes frame time, draw calls,
streaming pressure, or effect cost outside the acceptable envelope.

**Required proof**

- workload framing: `run_quick_playtest` or the owning domain's runtime flow
- performance evidence: `get_performance_report`
- visual corroboration when the issue is scene-heavy: `observe_ue_project`, `capture_viewport_sync`
- rollback evidence before risky tuning: `save_scene_checkpoint`, `restore_scene_checkpoint`, `get_world_state_digest`

**Minimum done bar**

- performance was measured under a representative runtime or review context
- the captured report is attached to the sign-off path, not inferred from intuition
- destructive optimization or regression investigation can be rolled back
- performance sign-off is tied to the owning domain workflow instead of isolated profiling theater

**Not done if**

- there is no performance report for a heavy or near-final task
- measurement happened in a context that does not resemble the real usage path
- optimization work changed state without a rollback artifact

## Cross-Domain Rule

If a task crosses domains, the finish line is the union of the primary-domain
template and the contract domain that can still fail.

Examples:

- Blueprint ability work: `Blueprint` + `GAS`
- HUD for ability cooldowns: `UI` + `GAS`
- playable arena slice: `LevelDesign` + `Performance`
- replicated ability or UI state: owning domain + `Networking`
- destructive iteration with rollback promises: owning domain + `Save/Load`

## Operational Rule

A workflow should not stop at compile success when this document says runtime,
restore, or performance proof is mandatory for the failure mode being changed.
