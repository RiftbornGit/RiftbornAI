# RiftbornAI School

These are domain-specific training tracks for AI agents working inside RiftbornAI.

The design-system and tool-reference docs describe supported surfaces. The School teaches how to think, sequence work, avoid common traps, and verify quality. Read the smallest track set that matches the domain you are about to mutate.

## Start Here

- **[TASK_INTAKE_AND_PREFLIGHT.md](TASK_INTAKE_AND_PREFLIGHT.md)** — How to identify the real mutation, clear editor blockers, choose the implementation lane, and define proof before any substantial work starts.
- **[TRACK_SELECTION.md](TRACK_SELECTION.md)** — How to choose the smallest correct set of tracks for mixed-domain work.
- **[VERIFICATION_LADDER.md](VERIFICATION_LADDER.md)** — What counts as real proof before a task can be called done.
- **[TASK_PLAYBOOKS.md](TASK_PLAYBOOKS.md)** — Reusable multi-domain recipes for common feature slices such as abilities, traversal, playable areas, cinematics, and destructibles.
- **[COVERAGE_BOUNDARIES.md](COVERAGE_BOUNDARIES.md)** — Why some design-system names are not yet documented as public AI School lanes.

## Modeling Track

**Mandatory before creating or editing dynamic-mesh geometry, profile-generated meshes, mesh topology, UV/normal cleanup, collision generation for modeled assets, or LOD/Nanite hardening passes.**

1. **[Modeling/01_Form_Silhouette_And_Modularity.md](Modeling/01_Form_Silhouette_And_Modularity.md)** — How modeled assets should be driven by function, silhouette, modularity, and collision promise.
2. **[Modeling/02_UE5_Geometry_Script_And_Mesh_Hardening_Systems.md](Modeling/02_UE5_Geometry_Script_And_Mesh_Hardening_Systems.md)** — Dynamic mesh authoring, Geometry Script operations, cleanup, export, collision, LOD, and Nanite on the current supported surface.
3. **[Modeling/03_Anti_Patterns.md](Modeling/03_Anti_Patterns.md)** — Failure modes to avoid when using the modeling surface.
4. **[Modeling/04_Workflow.md](Modeling/04_Workflow.md)** — A practical modeling workflow from shape intent to cleanup, export, and proof.
5. **[Modeling/05_Tool_Guide.md](Modeling/05_Tool_Guide.md)** — Which RiftbornAI tools to use for mesh generation, editing, cleanup, export, and hardening.

## Environment Track

**Mandatory before any environment, landscape, foliage, or world-building work.**

1. **[01_Nature_And_Ecosystems.md](01_Nature_And_Ecosystems.md)** — How real environments work. Vegetation layers, biomes, terrain, water, light.
2. **[02_UE5_Environment_Systems.md](02_UE5_Environment_Systems.md)** — The proper UE5 systems for environment creation. Landscape materials, Landscape Grass Type, Foliage Mode, PCG, PVE.
3. **[03_Anti_Patterns.md](03_Anti_Patterns.md)** — What not to do. Real project failure modes and wasted-motion patterns.
4. **[04_Workflow.md](04_Workflow.md)** — The correct step-by-step workflow for building environments.
5. **[05_Tool_Guide.md](05_Tool_Guide.md)** — Which RiftbornAI tools to use for each environment task.

## VFX / Niagara Track

**Mandatory before creating or editing gameplay VFX, ambient Niagara systems, or effect-driven feedback.**

1. **[VFX/01_Gameplay_Feedback_And_Readability.md](VFX/01_Gameplay_Feedback_And_Readability.md)** — How players read VFX and how effects support gameplay.
2. **[VFX/02_UE5_Niagara_Systems.md](VFX/02_UE5_Niagara_Systems.md)** — How Niagara maps to effect layering, parameters, and editor verification.
3. **[VFX/03_Anti_Patterns.md](VFX/03_Anti_Patterns.md)** — Failure modes to avoid when AI authors or edits VFX.
4. **[VFX/04_Workflow.md](VFX/04_Workflow.md)** — A practical VFX build order from gameplay brief to in-world verification.
5. **[VFX/05_Tool_Guide.md](VFX/05_Tool_Guide.md)** — Which Niagara, editor, vision, and verification tools to use for each VFX job.

## GAS / Ability Track

**Mandatory before creating or editing gameplay abilities, gameplay effects, attribute sets, gameplay tags, or actor-level GAS wiring.**

1. **[GAS/01_Ability_Design_And_State_Model.md](GAS/01_Ability_Design_And_State_Model.md)** — How abilities should be reasoned about as game-state contracts, not isolated buttons.
2. **[GAS/02_UE5_GAS_Systems.md](GAS/02_UE5_GAS_Systems.md)** — How ASC, abilities, effects, attributes, and tags map onto UE5 and RiftbornAI's supported tool surface.
3. **[GAS/03_Anti_Patterns.md](GAS/03_Anti_Patterns.md)** — Failure modes to avoid when authoring GAS assets and wiring them to actors.
4. **[GAS/04_Workflow.md](GAS/04_Workflow.md)** — A practical GAS build order from design brief to actor assignment and runtime verification.
5. **[GAS/05_Tool_Guide.md](GAS/05_Tool_Guide.md)** — Which RiftbornAI tools to use for tags, abilities, effects, attributes, and verification.

## Blueprint / Graph Track

**Mandatory before creating or editing Blueprint classes, event graphs, Blueprint components or variables, or Blueprint gameplay wiring.**

1. **[Blueprint/01_Architecture_And_Communication.md](Blueprint/01_Architecture_And_Communication.md)** — How to structure Blueprint classes and communication so graphs stay maintainable.
2. **[Blueprint/02_UE5_Blueprint_Systems.md](Blueprint/02_UE5_Blueprint_Systems.md)** — Blueprint graph types, editor-native context tools, mutation lanes, and current supported tool surface.
3. **[Blueprint/03_Anti_Patterns.md](Blueprint/03_Anti_Patterns.md)** — Failure modes to avoid when authoring Blueprint logic.
4. **[Blueprint/04_Workflow.md](Blueprint/04_Workflow.md)** — A practical Blueprint workflow from responsibility definition to compile and playtest.
5. **[Blueprint/05_Tool_Guide.md](Blueprint/05_Tool_Guide.md)** — Which RiftbornAI tools to use for Blueprint authoring, introspection, repair, and verification.

## Audio / MetaSound Track

**Mandatory before creating or editing MetaSound assets, placing world audio emitters, tuning ambience, or making acoustic/reverb decisions.**

1. **[Audio/01_Game_Audio_And_Mix_Intent.md](Audio/01_Game_Audio_And_Mix_Intent.md)** — How game audio supports readability, space, and feel.
2. **[Audio/02_UE5_Audio_And_Metasound_Systems.md](Audio/02_UE5_Audio_And_Metasound_Systems.md)** — MetaSound assets, preview flow, emitter placement, and the current supported RiftbornAI audio surface.
3. **[Audio/03_Anti_Patterns.md](Audio/03_Anti_Patterns.md)** — Failure modes to avoid when authoring procedural audio and ambience.
4. **[Audio/04_Workflow.md](Audio/04_Workflow.md)** — A practical audio workflow from role map to in-world verification.
5. **[Audio/05_Tool_Guide.md](Audio/05_Tool_Guide.md)** — Which RiftbornAI tools to use for MetaSound, emitter placement, acoustics, and playtest verification.

## UI / Widget Track

**Mandatory before creating or editing Widget Blueprints, HUD shells, menus, overlays, prompts, or controller-navigation flows.**

1. **[UI/01_Interface_Hierarchy_And_Player_Focus.md](UI/01_Interface_Hierarchy_And_Player_Focus.md)** — How UI directs attention, supports decisions, and stays readable under gameplay pressure.
2. **[UI/02_UE5_Widget_And_UI_Verification_Systems.md](UI/02_UE5_Widget_And_UI_Verification_Systems.md)** — Widget Blueprint authoring, tree inspection, layout audit, navigation, and live PIE verification.
3. **[UI/03_Anti_Patterns.md](UI/03_Anti_Patterns.md)** — Failure modes to avoid when authoring HUDs, menus, and interaction flows.
4. **[UI/04_Workflow.md](UI/04_Workflow.md)** — A practical UI workflow from surface definition to layout audit and live interaction proof.
5. **[UI/05_Tool_Guide.md](UI/05_Tool_Guide.md)** — Which RiftbornAI tools to use for widget creation, hierarchy inspection, navigation, and UI verification.

## Animation Track

**Mandatory before creating or editing Animation Blueprints, blend spaces, montages, IK-retarget assets, or motion-matching databases.**

1. **[Animation/01_Motion_Intent_And_Readability.md](Animation/01_Motion_Intent_And_Readability.md)** — How animation communicates intent, weight, transitions, and gameplay state.
2. **[Animation/02_UE5_Animation_And_Retargeting_Systems.md](Animation/02_UE5_Animation_And_Retargeting_Systems.md)** — Animation Blueprints, blend spaces, montages, retargeting, pose search, and the supported RiftbornAI tool surface.
3. **[Animation/03_Anti_Patterns.md](Animation/03_Anti_Patterns.md)** — Failure modes to avoid when authoring animation logic and asset pipelines.
4. **[Animation/04_Workflow.md](Animation/04_Workflow.md)** — A practical animation workflow from motion-role choice to runtime verification.
5. **[Animation/05_Tool_Guide.md](Animation/05_Tool_Guide.md)** — Which RiftbornAI tools to use for locomotion, one-shots, retargeting, and animation proof.

## Input Track

**Mandatory before creating or editing Enhanced Input actions, mapping contexts, key bindings, or control-scheme layouts.**

1. **[Input/01_Player_Intent_And_Control_Grammar.md](Input/01_Player_Intent_And_Control_Grammar.md)** — How input should map player intent to game verbs clearly and consistently.
2. **[Input/02_UE5_Enhanced_Input_Systems.md](Input/02_UE5_Enhanced_Input_Systems.md)** — Enhanced Input actions, contexts, mapping authoring, beta-gated modifier/trigger depth, and current supported RiftbornAI lanes.
3. **[Input/03_Anti_Patterns.md](Input/03_Anti_Patterns.md)** — Failure modes to avoid when building control schemes and mapping contexts.
4. **[Input/04_Workflow.md](Input/04_Workflow.md)** — A practical input workflow from action taxonomy to runtime verification.
5. **[Input/05_Tool_Guide.md](Input/05_Tool_Guide.md)** — Which RiftbornAI tools to use for input assets, mappings, and live control proof.

## Cinematics Track

**Mandatory before creating or editing Level Sequences, cinematic cameras, shot tracks, or render-output passes.**

1. **[Cinematics/01_Shot_Intent_And_Visual_Storytelling.md](Cinematics/01_Shot_Intent_And_Visual_Storytelling.md)** — How shots communicate story, orientation, emphasis, and pacing.
2. **[Cinematics/02_UE5_Sequencer_And_Camera_Systems.md](Cinematics/02_UE5_Sequencer_And_Camera_Systems.md)** — Sequencer, camera actors, sequence bindings, editor-context inspection, and the current supported RiftbornAI cinematic surface.
3. **[Cinematics/03_Anti_Patterns.md](Cinematics/03_Anti_Patterns.md)** — Failure modes to avoid when authoring cutscenes and shot sequences.
4. **[Cinematics/04_Workflow.md](Cinematics/04_Workflow.md)** — A practical cinematic workflow from shot list to Sequencer verification and render.
5. **[Cinematics/05_Tool_Guide.md](Cinematics/05_Tool_Guide.md)** — Which RiftbornAI tools to use for sequences, cameras, bindings, review, and rendering.

## Physics Track

**Mandatory before creating or editing collision setups, physics constraints, traces, Chaos destruction assets, or destructible gameplay props.**

1. **[Physics/01_Force_Collision_And_Physical_Readability.md](Physics/01_Force_Collision_And_Physical_Readability.md)** — How physics should communicate affordance, resistance, danger, and consequence.
2. **[Physics/02_UE5_Collision_Constraint_And_Destruction_Systems.md](Physics/02_UE5_Collision_Constraint_And_Destruction_Systems.md)** — Collision presets, trace tools, constraints, Geometry Collections, destruction snapshots, and current supported RiftbornAI physics lanes.
3. **[Physics/03_Anti_Patterns.md](Physics/03_Anti_Patterns.md)** — Failure modes to avoid when authoring collision, constraints, and destruction behavior.
4. **[Physics/04_Workflow.md](Physics/04_Workflow.md)** — A practical physics workflow from affordance definition to runtime proof and rollback.
5. **[Physics/05_Tool_Guide.md](Physics/05_Tool_Guide.md)** — Which RiftbornAI tools to use for collision, traces, constraints, destruction, and verification.

## Streaming Track

**Mandatory before creating or editing World Partition data layers, streaming-source behavior, streaming-state layouts, or texture-streaming diagnostics.**

1. **[Streaming/01_Spatial_Loading_And_Player_Presence.md](Streaming/01_Spatial_Loading_And_Player_Presence.md)** — How streaming should support player presence, scale, and state changes without breaking continuity.
2. **[Streaming/02_UE5_World_Partition_And_Data_Layer_Systems.md](Streaming/02_UE5_World_Partition_And_Data_Layer_Systems.md)** — World Partition inspection, data-layer authoring, streaming sources, city-state layers, and the current supported RiftbornAI streaming surface.
3. **[Streaming/03_Anti_Patterns.md](Streaming/03_Anti_Patterns.md)** — Failure modes to avoid when splitting worlds into cells, layers, and streaming triggers.
4. **[Streaming/04_Workflow.md](Streaming/04_Workflow.md)** — A practical streaming workflow from world-state design to assignment, inspection, and residency checks.
5. **[Streaming/05_Tool_Guide.md](Streaming/05_Tool_Guide.md)** — Which RiftbornAI tools to use for World Partition, data layers, streaming sources, and diagnostics.

## Networking Track

**Mandatory before creating or editing replication groups, replication-filter behavior, actor replication policy assumptions, or network-readiness audits.**

1. **[Networking/01_Authority_Visibility_And_Network_Truth.md](Networking/01_Authority_Visibility_And_Network_Truth.md)** — How multiplayer systems should separate authority, visibility, and bandwidth cost.
2. **[Networking/02_UE5_Replication_Audit_And_Iris_Group_Systems.md](Networking/02_UE5_Replication_Audit_And_Iris_Group_Systems.md)** — Replication audits, actor-level replication inspection, Iris replication groups, and the current supported RiftbornAI networking surface.
3. **[Networking/03_Anti_Patterns.md](Networking/03_Anti_Patterns.md)** — Failure modes to avoid when planning replication behavior and grouping.
4. **[Networking/04_Workflow.md](Networking/04_Workflow.md)** — A practical networking workflow from authority model to audit, grouping, and runtime-proof planning.
5. **[Networking/05_Tool_Guide.md](Networking/05_Tool_Guide.md)** — Which RiftbornAI tools to use for replication audits, actor inspection, and Iris group control.

## Save / Load Track

**Mandatory before creating or editing world-state rollback flows, editor checkpoints, snapshot-based persistence passes, or save/load verification plans.**

1. **[SaveLoad/01_Persistence_State_And_Restore_Intent.md](SaveLoad/01_Persistence_State_And_Restore_Intent.md)** — How persistence should define what state matters, what can be restored, and what continuity the player expects.
2. **[SaveLoad/02_UE5_Snapshot_Checkpoint_And_State_Digest_Systems.md](SaveLoad/02_UE5_Snapshot_Checkpoint_And_State_Digest_Systems.md)** — Level Snapshots, scene checkpoints, world-state digests, and the current supported RiftbornAI save/load surface.
3. **[SaveLoad/03_Anti_Patterns.md](SaveLoad/03_Anti_Patterns.md)** — Failure modes to avoid when treating snapshots as persistence without defining restore semantics.
4. **[SaveLoad/04_Workflow.md](SaveLoad/04_Workflow.md)** — A practical save/load workflow from state definition to checkpointing, restoration, and verification.
5. **[SaveLoad/05_Tool_Guide.md](SaveLoad/05_Tool_Guide.md)** — Which RiftbornAI tools to use for snapshots, checkpoints, digests, and restoration proof.

## Localization Track

**Mandatory before creating or editing String Table assets, localization entry keys, or project text-localization structure.**

1. **[Localization/01_Text_Intent_Keys_And_Cultural_Clarity.md](Localization/01_Text_Intent_Keys_And_Cultural_Clarity.md)** — How localized text should preserve intent, context, and readability across cultures.
2. **[Localization/02_UE5_String_Table_And_Culture_Systems.md](Localization/02_UE5_String_Table_And_Culture_Systems.md)** — String Tables, entry authoring, optional culture/table inspection helpers, and the current supported RiftbornAI localization surface.
3. **[Localization/03_Anti_Patterns.md](Localization/03_Anti_Patterns.md)** — Failure modes to avoid when treating localization as raw string replacement.
4. **[Localization/04_Workflow.md](Localization/04_Workflow.md)** — A practical localization workflow from key design to table authoring and verification.
5. **[Localization/05_Tool_Guide.md](Localization/05_Tool_Guide.md)** — Which RiftbornAI tools to use for string-table creation, entry updates, and localization-safe verification.

## Level Design / Blockout Track

**Mandatory before creating or editing maps, combat spaces, room layouts, corridors, arenas, dungeons, or blockout geometry.**

1. **[LevelDesign/01_Player_Flow_And_Spatial_Readability.md](LevelDesign/01_Player_Flow_And_Spatial_Readability.md)** — How players read space, landmarks, pacing, cover, and route choice.
2. **[LevelDesign/02_UE5_Level_And_Blockout_Systems.md](LevelDesign/02_UE5_Level_And_Blockout_Systems.md)** — Level assets, blockout tools, dungeon planning, navmesh, world partition, and the supported RiftbornAI surface.
3. **[LevelDesign/03_Anti_Patterns.md](LevelDesign/03_Anti_Patterns.md)** — Failure modes to avoid when shaping traversable space.
4. **[LevelDesign/04_Workflow.md](LevelDesign/04_Workflow.md)** — A practical workflow from layout intent to playtest and revision.
5. **[LevelDesign/05_Tool_Guide.md](LevelDesign/05_Tool_Guide.md)** — Which RiftbornAI tools to use for level layout, blockout, navigation, and verification.

## C++ Architecture Track

**Mandatory before creating or editing Unreal C++ gameplay classes, source files, module-facing class boundaries, or Blueprint-to-C++ conversions.**

1. **[CppArchitecture/01_Class_Boundaries_And_Responsibilities.md](CppArchitecture/01_Class_Boundaries_And_Responsibilities.md)** — How to choose between Actor, Character, Component, GameMode, and Subsystem responsibilities.
2. **[CppArchitecture/02_UE5_CPP_Authoring_And_Build_Systems.md](CppArchitecture/02_UE5_CPP_Authoring_And_Build_Systems.md)** — The supported RiftbornAI C++ generation, source-edit, build, reload, and conversion lanes.
3. **[CppArchitecture/03_Anti_Patterns.md](CppArchitecture/03_Anti_Patterns.md)** — Failure modes to avoid when generating or refactoring UE C++ code.
4. **[CppArchitecture/04_Workflow.md](CppArchitecture/04_Workflow.md)** — A practical workflow from class-boundary choice to compile and runtime verification.
5. **[CppArchitecture/05_Tool_Guide.md](CppArchitecture/05_Tool_Guide.md)** — Which RiftbornAI tools to use for code generation, editing, conversion, build, and validation.

## Rules

- Read the full track before mutating assets in that domain.
- For mixed tasks, read the smallest correct set of tracks. Example: a flaming forest shrine needs both Environment and VFX, not every track in the School.
- Tool guides are the last document, not the first. Principles and workflow come before tool calls.
- Verify every major change in context, not just in an isolated editor preview.
- If a tool name is uncertain, use `list_all_tools` or `describe_tool` instead of guessing.
