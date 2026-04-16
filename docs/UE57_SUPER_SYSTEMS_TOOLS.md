# UE 5.7 Super Systems Tools

This document is for AI agents using RiftbornAI through MCP or the in-editor copilot.

It covers the real UE 5.7-backed system layers currently wired into RiftbornAI's governed tool surface. Use the exact registered names shown here.

## Ground Rules

- These tools are real only through the governed Unreal tool surface. Do not invent aliases.
- `PRODUCTION` means default-visible. `BETA` means callable but not yet production-hardened.
- Prefer `observe_ue_project` / `analyze_scene_screenshot` loops around mutating tools.
- Treat the live registry, generated MCP bindings, and governance routes as the source of truth.
- Do not promise runtime behavior that depends on project setup you have not verified.
- When working inside asset editors, prefer editor-native context tools before generic fallback routes like screenshots or `execute_python`.

## Wave 1 Systems

### Editor-Native Eyes And Hands

UE 5.7 surfaces:
- `UAssetEditorSubsystem`
- `IMaterialEditor`
- `UMaterialEditingLibrary`
- `ULevelSequenceEditorBlueprintLibrary`
- `UControlRigBlueprintEditorLibrary`
- `FNiagaraEditorModule`

Tools:
- `get_editor_focus_state`
- `focus_asset_editor`
- `focus_editor_tab`
- `get_blueprint_editor_context`
- `list_blueprint_graphs`
- `list_blueprint_nodes`
- `find_blueprint_nodes`
- `get_blueprint_compile_diagnostics`
- `assert_blueprint_compiles`
- `focus_blueprint_node`
- `add_blueprint_function`
- `remove_blueprint_node`
- `replace_blueprint_node`
- `set_blueprint_pin_default`
- `rename_blueprint_variable`
- `reparent_blueprint`
- `batch_compile_blueprints`
- `repair_blueprint_compile_errors`
- `get_material_editor_context`
- `list_material_expressions`
- `inspect_material_expression`
- `assert_material_compiles`
- `set_material_expression_property`
- `delete_material_expression`
- `replace_material_expression`
- `add_material_function_call`
- `connect_material_attributes`
- `reparent_material_instance`
- `batch_compile_materials`
- `recompile_material_asset`
- `layout_material_asset_expressions`
- `get_sequencer_editor_context`
- `list_sequence_bindings`
- `assert_sequence_binding_exists`
- `assert_sequencer_selection`
- `add_selected_actors_to_active_sequence`
- `get_control_rig_editor_context`
- `list_control_rig_controls`
- `assert_control_rig_selection`
- `select_control_rig_control`
- `get_niagara_editor_context`
- `get_niagara_stack_context`
- `list_niagara_modules`
- `assert_niagara_compiles`
- `get_pcg_editor_context`
- `list_pcg_nodes`
- `inspect_pcg_node`
- `assert_pcg_graph_valid`

Use this layer when the task is "what editor is actually focused right now?" or when the work is happening inside Blueprint, Material, Sequencer, Control Rig, or Niagara editors rather than the level viewport.

Default-surface note:
- `get_blueprint_editor_context`, `get_material_editor_context`, `get_sequencer_editor_context`,
  `get_control_rig_editor_context`, `get_niagara_editor_context`, and `get_pcg_editor_context`
  are on the default-visible lane after live asset-editor proof was restored.

Recommended flow:
- `get_editor_focus_state` first
- `focus_asset_editor` when a specific asset editor must be brought forward
- then the matching domain context tool
- then `focus_blueprint_node` or `select_control_rig_control` when the task needs an exact editor-local target
- then the matching graph/hierarchy/module helper when it is registered
- then use the Blueprint mutation-depth lane for exact graph edits instead of generic retries:
  `add_blueprint_function`, `remove_blueprint_node`, `replace_blueprint_node`,
  `set_blueprint_pin_default`, `rename_blueprint_variable`, `reparent_blueprint`,
  `batch_compile_blueprints`, and `repair_blueprint_compile_errors`
- then use the Material mutation-depth lane for exact graph edits and instance-parent changes:
  `set_material_expression_property`, `delete_material_expression`,
  `replace_material_expression`, `add_material_function_call`,
  `connect_material_attributes`, `reparent_material_instance`,
  and `batch_compile_materials`
- then editor diagnostics or reflected control tools if more precision is needed
- then `get_world_outliner_context` or `assert_object_property_equals` when the task needs grounded level-selection or reflected property verification

### Editor Control Core

UE 5.7 surfaces:
- `FSlateNotificationManager`
- `FMessageLogModule`
- `IMessageLogListing`
- `UEditorAssetSubsystem`
- `FEditorFileUtils`
- `ISourceControlProvider`
- `UPackageTools`
- `UObject::ProcessEvent`

Tools:
- `list_editor_windows`
- `focus_editor_window`
- `minimize_editor_window`
- `restore_editor_window`
- `resize_editor_window`
- `close_editor_window`
- `get_output_log_context`
- `get_message_log_context`
- `drain_log_alerts`
- `get_notification_center_state`
- `get_modal_blockers`
- `assert_no_modal_blockers`
- `assert_output_log_clean`
- `assert_asset_dirty_state`
- `assert_actor_selection`
- `assert_editor_focus`
- `get_world_outliner_context`
- `assert_object_property_equals`
- `get_compile_diagnostics`
- `list_object_properties`
- `get_object_property_typed`
- `set_object_property_typed`
- `call_reflected_function`
- `save_asset`
- `save_dirty_assets`
- `checkout_asset`
- `revert_asset`

Use this layer when the agent needs deterministic control loops rather than broad scene planning:
- exact editor window inventory, focus swaps, minimize/restore, resize, and safe close refusal on the root frame
- blocker checks before any editor mutation
- compile/log verification after tool execution
- continuous warning/error polling with a cursor instead of ad hoc snapshot reads
- exact reflected property/function control on live `UObject` instances
- explicit save and source-control operations instead of implicit persistence

Default-surface note:
- `get_notification_center_state`, `assert_asset_dirty_state`, `assert_actor_selection`,
  `assert_editor_focus`, and `get_world_outliner_context` are on the default-visible lane after
  live control-core proof was restored.
- `assert_output_log_clean` remains callable but beta-gated until the output-log warning floor is
  less sensitive to unrelated session noise.

Recommended flow:
- `list_editor_windows` or `get_editor_gui_state` first when window routing matters
- `focus_editor_window` before operating on a non-root asset editor window
- `minimize_editor_window` / `restore_editor_window` / `resize_editor_window` for deterministic layout control
- `close_editor_window` only for non-root, non-modal windows after the target window id is verified
- `get_modal_blockers` or `assert_no_modal_blockers`
- `get_output_log_context` / `get_message_log_context` / `drain_log_alerts` / `get_compile_diagnostics`
- `list_object_properties` before `set_object_property_typed` or `call_reflected_function`
- `assert_*` verification tools after mutations
- `save_asset` or `save_dirty_assets` only when the mutation is confirmed

### Build, Validation, And Runtime Verification

UE 5.7 surfaces:
- `ILiveCodingModule`
- `IHotReloadModule`
- `FPlatformProcess::CreateProc`
- `UEditorValidatorSubsystem`
- `FPlatformMemory::GetStats`
- `RHIGetTextureMemoryStats`
- `UReplicationSystem`

Tools:
- `diagnose_crash`
- `get_build_status`
- `get_build_events`
- `get_build_errors`
- `hot_reload_cpp`
- `trigger_live_coding`
- `run_ubt`
- `cook_project`
- `package_project`
- `validate_for_packaging`
- `get_cook_stats`
- `run_quick_playtest`
- `validate_assets`
- `validate_asset_quick`
- `validate_world_state`
- `validate_blueprint_health`
- `get_memory_stats`
- `get_performance_report`
- `inspect_actor_replication`
- `audit_net_replication`

Use this layer when the task is not "change the world" but "prove the world is shippable":
- crash-log diagnosis after editor, PIE, compile, or tool-execution failures
- build and compile status before and after code changes
- packaging-readiness checks before cook/package
- bulk or targeted asset validation before certification review
- quick PIE smoke verification after gameplay changes
- runtime memory and frame-cost inspection
- replication inspection and project-wide replication audit

Reality:
- `hot_reload_cpp`, `trigger_live_coding`, `run_ubt`, `cook_project`, and `package_project` are long-running project operations and should be called with explicit intent, not opportunistically.
- `get_performance_report` is the honest combined lane: runtime FPS/memory/scene-cost snapshot plus tool-execution telemetry.

### Widget / UMG Authoring And PIE UI Verification

UE 5.7 surfaces:
- `UWidgetTree::GetAllWidgets`
- `FKismetEditorUtilities::CompileBlueprint`
- `FWidgetBlueprintEditorUtils::VerifyWidgetRename`
- `FWidgetBlueprintEditorUtils::RenameWidget`
- `UWidgetBlueprintLibrary::GetAllWidgetsOfClass`
- `FSlateApplication`

Tools:
- `get_widget_editor_context`
- `list_widget_tree`
- `compile_widget_blueprint`
- `rename_widget`
- `assert_widget_visible_in_pie`
- `run_ui_flow_test`
- `capture_ui_state`

Use this layer when the task is "author or verify UI like a real team" instead of "just mutate a widget asset":
- inspect the active Widget Blueprint editor before changing hierarchy
- enumerate the authored widget tree with real parent/slot structure
- compile widget blueprints on the documented safe lane instead of relying on generic blueprint compile loops
- perform real UMG rename operations through the widget editor path so bindings and preview state stay honest
- assert live PIE visibility and capture current focus/widget state during UI verification
- run small scripted UI flows in PIE and verify the expected end state

### High-Value Underexposed Families

These are real UE 5.7-backed tool families that agents often miss because they are not obvious from generic prompts alone.

Promote to agent planning:
- World-state branching and rollback:
  `capture_level_snapshot`, `restore_level_snapshot`, `list_level_snapshots`, `capture_destruction_snapshot`, `restore_destruction_snapshot`
- Authored reality switching:
  `create_level_variant_sets_asset`, `create_level_variant_sets_actor`, `add_variant_set`, `add_variant`, `bind_actor_to_variant`, `capture_variant_property`, `switch_variant_by_name`
- Operator-facing control surfaces:
  `create_remote_control_preset`, `expose_actor_to_remote_control`, `expose_property_to_remote_control`, `expose_function_to_remote_control`, `apply_remote_control_color_wheel_delta`, `apply_remote_control_color_grading_delta`
- Affordance networks:
  `register_smart_object_actor`, `set_smart_object_actor_enabled`, `inspect_smart_object_actor`, `set_smart_object_slot_enabled`, `claim_smart_object_slot`, `occupy_smart_object_slot`, `release_smart_object_slot`, `send_smart_object_slot_event`
- Authored decision-table evaluation:
  `evaluate_chooser_table`, `evaluate_chooser_table_multi`, `evaluate_proxy_table`, `evaluate_proxy_asset`
- Cinematic render graph setup:
  `create_movie_graph_config`, `queue_trailer_render_job`

Keep discoverable but do not make prompt-visible defaults:
- `spawn_mass_crowd_spawner`, `run_mass_crowd_spawner`, `set_mass_crowd_lane_state`
Reason: real UE API, but heavily dependent on valid ZoneGraph and Mass setup.
- `create_contextual_anim_bindings_for_two_actors`, `calculate_contextual_anim_warp_points`, `activate_contextual_anim_warp_targets`
Reason: real UE API, but depends on authored contextual animation assets and role-compatible actors.
- `make_learning_agents_imitation_trainer`, `begin_learning_agents_training`, `iterate_learning_agents_training`, `end_learning_agents_training`
Reason: real UE API, but session-scoped and too fragile for generic agent guidance.

### Architecture Compiler

Tools:
- `create_building_from_floor_plan`
- `create_staircase`
- `create_spline_architecture`

Use when you want authored geometry from structured layout inputs rather than generic prop placement.

### City Time Machine / Destruction Historian

UE 5.7 surfaces:
- `ULevelSnapshotsEditorFunctionLibrary::TakeLevelSnapshotAndSaveToDisk`
- `ULevelSnapshotsFunctionLibrary::ApplySnapshotToWorld`

Tools:
- `capture_level_snapshot`
- `restore_level_snapshot`
- `list_level_snapshots`
- `capture_destruction_snapshot`
- `restore_destruction_snapshot`

Use to branch or restore authored world states for era swaps, siege versions, and destruction passes.

### Living Crowd Director

UE 5.7 surfaces:
- `AMassSpawner`
- `UMassCrowdSubsystem`
- `UZoneGraphSubsystem`

Tools:
- `spawn_mass_crowd_spawner`
- `run_mass_crowd_spawner`
- `inspect_mass_crowd_state`
- `set_mass_crowd_lane_state`

Reality:
- This is only useful when the map has valid ZoneGraph and Mass setup.

### Niagara Data-Channel VFX Brain

UE 5.7 surfaces:
- `UNiagaraDataChannelLibrary::WriteToNiagaraDataChannel_WithContext`
- `UNiagaraDataChannelLibrary::ReadFromNiagaraDataChannel_WithContext`

Tools:
- `write_niagara_data_channel_batch`
- `read_niagara_data_channel_batch`

Use to drive smoke, ash, crowd heat, torch intensity, traffic pulses, or festival signals into Niagara without per-actor glue.

### Generative Soundscape Composer

UE 5.7 surfaces:
- `UMetaSoundBuilderSubsystem`
- `UMetaSoundSourceBuilder`

Tools:
- `create_soundscape_metasound_source`

Reality:
- This scaffolds a real MetaSound Source with controllable inputs.
- It does not auto-compose a finished soundscape graph without follow-up graph authoring.

### Shot Director / Trailer Generator

UE 5.7 surfaces:
- `UMovieGraphConfig`
- `UMoviePipelineEditorBlueprintLibrary::CreateJobFromSequence`
- `UMoviePipelineExecutorJob::SetGraphPreset`

Tools:
- `create_movie_graph_config`
- `queue_trailer_render_job`

Use to scaffold graph-driven MRQ renders around real `LevelSequence` assets.

### Data-Layer City Orchestrator

UE 5.7 surfaces:
- `UDataLayerInstanceWithAsset`
- `UDataLayerManager::SetDataLayerInstanceRuntimeState`

Tools:
- `create_city_state_data_layers`
- `set_city_state_runtime`

Good fit:
- `City_Day`
- `City_Night`
- `City_Festival`
- `City_Siege`
- `City_Aftermath`

### PCG + Geometry Hybrid Foundry

UE 5.7 surfaces:
- `UPCGStaticMeshToDynamicMeshSettings`
- `UPCGBooleanOperationSettings`
- `UPCGSaveDynamicMeshToAssetSettings`
- `UPCGSpawnDynamicMeshSettings`

Tools:
- `create_geometry_foundry_graph`
- `spawn_geometry_foundry_volume`

Reality:
- This is a real graph scaffold, not a full procedural authoring suite by itself.

### Roman City District Builder

Tools:
- `create_roman_column`
- `create_roman_arch`
- `create_roman_wall_bay`
- `create_roman_street_grid`
- `compose_roman_forum`
- `compose_roman_insula_block`
- `compose_roman_district`
- `audit_roman_district`
- `dress_roman_district`
- `review_roman_district`

Use as the high-level district layer on top of the geometry, layout, dressing, and verification substrate.

### Dynamic Foliage Ecosystem

UE 5.7 surfaces:
- `UFoliageType`
- `AInstancedFoliageActor::FoliageTrace`
- `AInstancedFoliageActor::CheckCollisionWithWorld`
- `UProceduralFoliageSpawner`
- `UProceduralFoliageComponent`
- `AProceduralFoliageVolume`
- `UInstancedStaticMeshComponent::SetCustomDataValue`
- `UPrimitiveComponent::SetSimulatePhysics`

Tools:
- `paint_foliage`
- `ground_foliage_to_landscape`
- `create_landscape_grass_type`
- `add_grass_variety`
- `set_foliage_properties`
- `list_foliage_types`
- `remove_foliage`
- `create_procedural_foliage_spawner`
- `spawn_procedural_foliage_volume`
- `resimulate_procedural_foliage`
- `inspect_procedural_foliage`
- `set_foliage_instance_lifecycle`
- `promote_foliage_to_dynamic_actors`
- `set_dynamic_foliage_actor_state`

Use this layer when foliage must be both visually grounded and biologically plausible:
- landscape grass for tiny ground cover
- foliage paint for support-aware trees, shrubs, rocks, and biome props, not automatic grass
- procedural foliage for growth, spread, shade competition, and reseeding
- promoted dynamic actors for falling trees and visible decay

Reality:
- Slight burial into soil is supported and often desirable.
- Floating foliage is a bug and should be repaired with `ground_foliage_to_landscape`.
- Tree growth and reseeding are real through procedural foliage.
- Tree falling and rotting are real through promoted actors plus lifecycle state tools, not through instanced foliage alone.
- UE 5.7 exposes `UProceduralFoliageSpawner::GetFoliageTypes()` but no public setter, so spawner authoring still uses reflected struct-property writes and remains `BETA`.

## Wave 2 Systems

### Variant Universe Switchboard

UE 5.7 surfaces:
- `UVariantManagerBlueprintLibrary::CreateLevelVariantSetsAsset`
- `UVariantManagerBlueprintLibrary::CreateLevelVariantSetsActor`
- `UVariantManagerBlueprintLibrary::AddVariantSet`
- `UVariantManagerBlueprintLibrary::AddVariant`
- `UVariantManagerBlueprintLibrary::AddActorBinding`
- `UVariantManagerBlueprintLibrary::CaptureProperty`
- `ALevelVariantSetsActor::SwitchOnVariantByName`

Tools:
- `create_level_variant_sets_asset`
- `create_level_variant_sets_actor`
- `add_variant_set`
- `add_variant`
- `bind_actor_to_variant`
- `capture_variant_property`
- `switch_variant_by_name`

Use to author multiple realities of the same map: day/night, intact/ruined, festival/siege, wealthy/poor district states.

### Remote-Control World Cockpit

UE 5.7 surfaces:
- `URemoteControlFunctionLibrary::ExposeActor`
- `URemoteControlFunctionLibrary::ExposeProperty`
- `URemoteControlFunctionLibrary::ExposeFunction`
- `URemoteControlFunctionLibrary::ApplyColorWheelDelta`
- `URemoteControlFunctionLibrary::ApplyColorGradingWheelDelta`

Tools:
- `create_remote_control_preset`
- `expose_actor_to_remote_control`
- `expose_property_to_remote_control`
- `expose_function_to_remote_control`
- `apply_remote_control_color_wheel_delta`
- `apply_remote_control_color_grading_delta`

Use when you want a live operator surface for weather, alert level, gates, grading, fog, or lighting state.

### Smart-Object Society Compiler

UE 5.7 surfaces:
- `USmartObjectSubsystem::RegisterSmartObjectActor`
- `USmartObjectSubsystem::SetSmartObjectActorEnabled`
- `USmartObjectSubsystem::SetSlotEnabled`
- `USmartObjectSubsystem::MarkSlotAsClaimed`
- `USmartObjectSubsystem::MarkSlotAsOccupied`
- `USmartObjectSubsystem::MarkSlotAsFree`
- `USmartObjectSubsystem::SendSlotEvent`

Tools:
- `register_smart_object_actor`
- `set_smart_object_actor_enabled`
- `inspect_smart_object_actor`
- `set_smart_object_slot_enabled`
- `claim_smart_object_slot`
- `occupy_smart_object_slot`
- `release_smart_object_slot`
- `send_smart_object_slot_event`

Use to turn a city into a real affordance network instead of static decoration.
Claim handles are session-scoped by engine/tool design. Re-claim after editor restart, tool-session reset, or slot invalidation.

### Motion-Matching Combat Director

UE 5.7 surfaces:
- `UPoseSearchDatabase`
- `UE::PoseSearch::FAsyncPoseSearchDatabasesManagement::RequestAsyncBuildIndex`

Tools:
- `create_pose_search_database`
- `add_animation_to_pose_search_database`
- `build_pose_search_database_index`
- `inspect_pose_search_database`

Reality:
- This is database authoring and inspection.
- It still depends on real animation clips and a real motion-matching animation graph.

### Control Rig Fabricator

UE 5.7 surfaces:
- `UControlRigBlueprintFactory`
- `URigHierarchyController`
- `URigVMController`

Tools:
- `create_control_rig_asset`
- `create_control_rig_from_skeletal_mesh`
- `add_control_rig_bone`
- `add_control_rig_null`
- `add_control_rig_control`
- `add_control_rig_variable_node`
- `add_control_rig_branch_node`
- `add_control_rig_comment_node`
- `link_control_rig_pins`
- `set_control_rig_pin_default`

Use to author rig assets and graph structure directly inside the editor.

### Auto-Rig And Retarget Factory

UE 5.7 surfaces:
- `UIKRigController`
- `UIKRetargeterController`

Tools:
- `set_ik_retarget_root`
- `add_ik_retarget_chain`
- `auto_map_retarget_chains`
- `create_retarget_pose`
- `set_current_retarget_pose`
- `set_retarget_pose_rotation_offset`

Use for bulk rig/retarget authoring rather than hand-editing each asset.

## Wave 3 Systems

Smart-object society remains part of this stack and is already documented under Wave 2. The tools below are the additional UE 5.7-backed systems added for the next authoring layer.

### Chooser Brain

UE 5.7 surfaces:
- `UChooserFunctionLibrary::EvaluateChooser`
- `UChooserFunctionLibrary::EvaluateChooserMulti`
- `UProxyTableFunctionLibrary::EvaluateProxyTable`
- `UProxyTableFunctionLibrary::EvaluateProxyAsset`

Tools:
- `evaluate_chooser_table`
- `evaluate_chooser_table_multi`
- `evaluate_proxy_table`
- `evaluate_proxy_asset`

Use to evaluate authored choice logic without hardcoding branches in Blueprints or tool prompts.

Reality:
- The tools now resolve `UChooserTable` assets through the typed UE 5.7 API surface instead of stringly class-name checks.
- They still assume the chooser/proxy assets already exist and are authored correctly.

### Live Balance Fabric

UE 5.7 surfaces:
- `UDataRegistrySubsystem::GetPossibleDataRegistryIdList`
- `UDataRegistrySubsystem::GetDisplayTextForId`
- `UDataRegistrySubsystem::EvaluateDataRegistryCurve`
- `UDataRegistrySubsystem::RegisterSpecificAsset`

Tools:
- `list_data_registry_ids`
- `get_data_registry_id_display_text`
- `evaluate_data_registry_curve`
- `register_data_registry_asset`

Use when balancing should flow through Data Registry identifiers and curves instead of scattered constants.

### Contextual Interaction Composer

UE 5.7 surfaces:
- `UContextualAnimUtilities::BP_CreateContextualAnimSceneBindingsForTwoActors`
- `UContextualAnimUtilities::BP_SceneBindings_CalculateWarpPoints`
- `UContextualAnimUtilities::BP_SceneBindings_AddOrUpdateWarpTargetsForBindings`

Tools:
- `create_contextual_anim_bindings_for_two_actors`
- `calculate_contextual_anim_warp_points`
- `activate_contextual_anim_warp_targets`
- `inspect_contextual_anim_bindings`

Use to wire synchronized multi-actor interactions around real Contextual Animation assets.

Reality:
- The bindings handle is reconstructable from the scene asset path and actor labels, so it can be rebuilt if transient editor state is lost.
- The underlying UE Contextual Animation plugin is still experimental in UE 5.7.

### Learning Agents Auto-Tuner

UE 5.7 surfaces:
- `ULearningAgentsImitationTrainer::MakeImitationTrainer`
- `ULearningAgentsImitationTrainer::BeginTraining`
- `ULearningAgentsImitationTrainer::IterateTraining`
- `ULearningAgentsImitationTrainer::EndTraining`

Tools:
- `inspect_learning_agents_manager`
- `make_learning_agents_imitation_trainer`
- `begin_learning_agents_training`
- `iterate_learning_agents_training`
- `end_learning_agents_training`

Reality:
- This is real training orchestration against UE 5.7 Learning Agents.
- It still depends on a valid manager, interactor, policy, and recording asset setup in the project.
- Trainer handles can recreate the transient trainer object, but active training itself is still session-scoped. After an editor restart, call `begin_learning_agents_training` again before `iterate` or `end`.
- RiftbornAI constructs imitation trainers through `ULearningAgentsCommunicatorLibrary::MakeSharedMemoryTrainingProcess` plus `ULearningAgentsImitationTrainer::MakeImitationTrainer` with UE's shipped behavior-cloning trainer module.

### VCam Director Desk

Status:
- The VCam tool family is currently withdrawn from the shipped/default RiftbornAI surface.
- UE 5.7 does expose the underlying VCam APIs, but RiftbornAI does not currently ship a concrete
  `VCamToolsModule` with registered tool handlers behind those names.
- Until that module is restored and live-certified, do not advertise `list_vcam_components`,
  `inspect_vcam_component`, `add_vcam_modifier`, `remove_vcam_modifier`, or
  `set_vcam_modifier_context_class` as real callable production tools.

### Take Recorder Factory

UE 5.7 surfaces:
- `UTakeRecorderSubsystem::SetRecordIntoLevelSequence`
- `UTakeRecorderSubsystem::AddSourceForActor`
- `UTakeRecorderSubsystem::StartRecording`
- `UTakeRecorderSubsystem::StopRecording`

Tools:
- `set_take_recorder_target_sequence`
- `add_take_recorder_actor_source`
- `start_take_recording`
- `stop_take_recording`
- `inspect_take_recorder_state`

Use to capture real performance, gameplay, or previs takes into authored Level Sequence assets.

### Live Link Performance Router

UE 5.7 surfaces:
- `ULiveLinkBlueprintLibrary::GetLiveLinkSubjects`
- `ULiveLinkBlueprintLibrary::EvaluateLiveLinkFrameWithSpecificRole`
- `ULiveLinkBlueprintLibrary::SetLiveLinkSubjectEnabled`
- `ULiveLinkBlueprintLibrary::PauseSubject`
- `ULiveLinkBlueprintLibrary::UnpauseSubject`

Tools:
- `list_live_link_subjects`
- `evaluate_live_link_frame`
- `set_live_link_subject_enabled`
- `pause_live_link_subject`
- `unpause_live_link_subject`

Use to inspect and route live performance streams instead of treating Live Link as an opaque external feed.

### Replication Contract Layer

UE 5.7 surfaces:
- `UReplicationSystem::CreateGroup`
- `UReplicationSystem::AddToGroup`
- `UReplicationSystem::SetGroupFilterStatus`
- `UReplicationSystem::SetCullDistanceOverride`
- `UReplicationSystem::ForceNetUpdate`

Tools:
- `create_replication_group`
- `add_actor_to_replication_group`
- `set_replication_group_filter_status`
- `set_actor_cull_distance_override`
- `force_actor_net_update`
- `inspect_actor_replication`

Reality:
- This is real Iris replication policy control.
- It only works when a valid replication system is active for the current world or PIE/net context.

### World Rule Fabric

UE 5.7 surfaces:
- `UWorldConditionSchema::AddContextDataDesc`
- `FWorldConditionQuery::DebugInitialize`
- `FWorldConditionQuery::Activate`
- `FWorldConditionQuery::IsTrue`
- `FWorldConditionQuery::Deactivate`

Tools:
- `evaluate_actor_gameplay_tag_world_rule`
- `evaluate_actor_distance_world_rule`
- `evaluate_world_rule_bundle`

Use when world-state gating should be explicit, inspectable, and reusable instead of hidden inside ad hoc Blueprint branches.

### Neural-Morph Performance Layer

UE 5.7 surfaces:
- `UMLDeformerAsset`
- `UNeuralMorphModel`
- `UMLDeformerComponent`

Tools:
- `create_neural_morph_asset`
- `configure_neural_morph_asset`
- `inspect_neural_morph_asset`
- `attach_neural_morph_component`

Reality:
- This gives you real ML Deformer / Neural Morph asset and component control.
- Training data quality is still the hard part outside the tool surface.

### USD Live Twin

UE 5.7 surfaces:
- `AUsdStageActor::SetRootLayer`
- `AUsdStageActor::SetStageState`
- `AUsdStageActor::SetNaniteTriangleThreshold`
- `AUsdStageActor::SetTime`
- `AUsdStageActor::GetGeneratedAssets`

Tools:
- `spawn_usd_stage_actor`
- `set_usd_stage_root_layer`
- `configure_usd_stage_import`
- `set_usd_stage_time`
- `inspect_usd_stage_actor`

Use when Unreal is a live staging and inspection layer for USD-authored worlds or DCC pipelines.

### Chaos Field Destruction Painter

UE 5.7 surfaces:
- `UFieldSystemComponent::ApplyLinearForce`
- `UFieldSystemComponent::ApplyRadialForce`
- `UFieldSystemComponent::ApplyUniformVectorFalloffForce`
- `UFieldSystemComponent::AddPersistentField`
- `UFieldSystemComponent::ResetFieldSystem`

Tools:
- `add_field_system_component`
- `apply_linear_force_field`
- `apply_radial_force_field`
- `apply_uniform_force_field`
- `add_persistent_radial_falloff_field`
- `reset_field_system`

Use for force, strain, and persistent destruction-field orchestration.

Honest caveat:
- The Field System path is real, but UE treats parts of it as experimental territory.

### Gameplay Camera Brain

UE 5.7 surfaces:
- `UCameraAsset`
- `UCameraRigAsset`
- `UGameplayCameraComponent`
- `UGameplayCameraComponentBase::ActivatePersistentBaseCameraRig`
- `UGameplayCameraComponentBase::ActivatePersistentGlobalCameraRig`
- `UGameplayCameraComponentBase::ActivatePersistentVisualCameraRig`

Tools:
- `create_gameplay_camera_asset`
- `create_camera_rig_asset`
- `add_gameplay_camera_component`
- `set_gameplay_camera_asset`
- `activate_persistent_camera_rig`

Use to author and activate layered gameplay camera systems instead of only spawning raw camera actors.

Reality:
- RiftbornAI now uses the UE 5.7 GameplayCameras activation entry points that are stable across module boundaries.
- The tool prefers the engine's `UActivateCameraRigFunctions` Blueprint library for player-controller-driven activation, then falls back to the component's `ActivatePersistent*CameraRig` UFUNCTION path through reflection for standalone/component-owned activation.

### Chaos Mover Motion Layer

UE 5.7 surfaces:
- `UChaosCharacterMoverComponent::QueueNextMode`
- `UChaosCharacterMoverComponent::OverrideMovementSettings`
- `UMoverComponent::QueueInstantMovementEffect`
- `IMoverInputProducerInterface::ProduceInput`
- `FChaosCharacterApplyVelocityEffect`
- `FChaosMovementSettingsOverridesRemover`

Tools:
- `add_chaos_character_mover`
- `queue_chaos_mover_mode`
- `launch_chaos_mover`
- `override_chaos_mover_settings`

Use when character locomotion should go through UE 5.7's Mover / ChaosMover stack instead of CharacterMovement-style wrappers.

Reality:
- The launch path uses the public ChaosMover instant-effect type `FChaosCharacterApplyVelocityEffect` plus `UMoverComponent::QueueInstantMovementEffect`.
- Clearing movement-setting overrides goes through a plugin-owned input producer that injects `FChaosMovementSettingsOverridesRemover` through the public Mover input pipeline.
- This keeps the live tool surface on exported Mover / ChaosMover APIs instead of relying on cross-module `ProcessEvent` dispatch.

## Suggested Build Loop

1. Inspect the map with `observe_ue_project`.
2. Build or refine geometry with architecture or Roman-city tools.
3. Branch world states with `create_level_variant_sets_asset` and `switch_variant_by_name`.
4. Compile affordances with Smart Objects and crowd lanes.
5. Drive spectacle with Niagara data channels, lighting, and MetaSound.
6. Add authored rig, retarget, or Neural Morph layers where character fidelity needs it.
7. Use USD stage tools when external scene data should stay live-linked.
8. Use gameplay camera tools and MRQ tools to stage the reveal.
9. Capture snapshots before risky scene-wide changes.
10. Verify visually and iterate.

## Honest Limits

- These systems are real because they are backed by UE 5.7 APIs, not because they are autonomous magic.
- Many of them still depend on project-side setup you must verify: ZoneGraph, Mass, Level Sequences, Smart Object components, skeletal meshes, animation assets, USD files, or camera assets.
- `BETA` means callable and governed, not guaranteed production-safe for every project shape.
- Do not promise end-to-end behavior unless you have verified it in the running editor.
