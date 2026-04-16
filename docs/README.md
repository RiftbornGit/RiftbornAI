# RiftbornAI Documentation Index

Canonical documentation map for RiftbornAI, the governed Unreal Engine 5.7 editor plugin and MCP/copilot surface.

- Plugin type: UE editor plugin
- Shipped product boundary: AI drives a live Unreal Editor session through governed tools, via MCP or the built-in copilot
- Canonical product definition: [CORE_PRODUCT_CONTRACT.md](CORE_PRODUCT_CONTRACT.md)

---

## Start Here

| Document | Description |
|----------|-------------|
| **[GETTING_STARTED.md](GETTING_STARTED.md)** | Installation, setup, first tool calls, and key workflows |
| **[USER_TUTORIAL.md](USER_TUTORIAL.md)** | User-facing guide to what RiftbornAI can do today, with examples like silPOM materials, Blueprint repair, playtesting, and environment building |
| **[CORE_PRODUCT_CONTRACT.md](CORE_PRODUCT_CONTRACT.md)** | Authoritative shipped product definition and execution model |
| **[REPO_STRUCTURE.md](REPO_STRUCTURE.md)** | Repository layout, ownership boundaries, and placement rules |
| **[UE57_SUPER_SYSTEMS_TOOLS.md](UE57_SUPER_SYSTEMS_TOOLS.md)** | UE 5.7 editor-context and advanced tool reference |

## Product And Architecture

| Document | Description |
|----------|-------------|
| [ARCHITECTURE_MAP.md](ARCHITECTURE_MAP.md) | System architecture, module layout, and data flow |
| [OPEN_SOURCE_BOUNDARY.md](OPEN_SOURCE_BOUNDARY.md) | Current repository and licensing boundary |
| [CPP_MODULES.md](CPP_MODULES.md) | Active C++ module reference for the shipped editor plugin boundary |
| [CONFIGURATION_REFERENCE.md](CONFIGURATION_REFERENCE.md) | UE settings, bridge config, and environment variables |
| [DEVELOPMENT_GUIDE.md](DEVELOPMENT_GUIDE.md) | Build instructions and day-to-day development workflows |

## Tooling And Integration

| Document | Description |
|----------|-------------|
| [TOOL_SYSTEM.md](TOOL_SYSTEM.md) | Tool categories, registration, and execution pipeline |
| [TOOL_IMPL_CATALOG.md](TOOL_IMPL_CATALOG.md) | Tool implementation patterns across ToolImpl and Tools modules |
| [MCP_SERVER.md](MCP_SERVER.md) | MCP server architecture, stdio transport, and schema handling |
| [BRIDGE_PROTOCOL.md](BRIDGE_PROTOCOL.md) | HTTP bridge protocol and tool execution envelope |
| [TOOL_REGISTRY_AUDIT.md](TOOL_REGISTRY_AUDIT.md) | Tool registration and shipped-surface audit guidance |

## Runtime And Systems

| Document | Description |
|----------|-------------|
| [AGENT_SYSTEM.md](AGENT_SYSTEM.md) | Agent loop and execution model details that are still on the active product path |
| [LLM_PROVIDERS.md](LLM_PROVIDERS.md) | Supported providers and failover policy |
| [AWARENESS_SUBSYSTEMS.md](AWARENESS_SUBSYSTEMS.md) | Spatial, temporal, performance, and gameplay awareness subsystems |
| [DESIGN_SYSTEMS.md](DESIGN_SYSTEMS.md) | Design-side systems: PCG, VFX, GAS, and related authoring flows |

## Governance And Quality

| Document | Description |
|----------|-------------|
| [GOVERNANCE_AND_SECURITY.md](GOVERNANCE_AND_SECURITY.md) | Risk tiers, proofs, confirmation rules, and governance model |
| [CI_AND_TESTING.md](CI_AND_TESTING.md) | Test framework, proof bundles, and CI gates |

## Repository Root And Legal Docs

| Document | Description |
|----------|-------------|
| [../README.md](../README.md) | Public project overview and first-contact product framing |
| [../TOOL_REGISTRY.md](../TOOL_REGISTRY.md) | Generated registry of registered tools and readiness metadata |
| [../CHANGELOG.md](../CHANGELOG.md) | Release notes and historical changes |
| [../CONTRIBUTING.md](../CONTRIBUTING.md) | Contribution rules and workflow guidance |
| [../SECURITY.md](../SECURITY.md) | Security disclosure policy |
| [../EULA.md](../EULA.md) | Licensing and commercial-use notice |
| [../COMMERCIAL_LICENSE.md](../COMMERCIAL_LICENSE.md) | Commercial license guidance and contact path |

## Package And Subsystem READMEs

| Document | Description |
|----------|-------------|
| [../mcp-server/README.md](../mcp-server/README.md) | Package-local MCP server setup and runtime behavior |
| [../Bridge/README.md](../Bridge/README.md) | Python bridge boundary and responsibilities |
| [../ci/README.md](../ci/README.md) | CI gate inventory and repo-facing validation scripts |
| [../Tests/README.md](../Tests/README.md) | Shared test-boundary rules for repo-level tests |
| [../Generators/README.md](../Generators/README.md) | JSON generator-plan boundary and execution model |
| [../Scripts/README.md](../Scripts/README.md) | Human-operated repository maintenance scripts |
| [../services/README.md](../services/README.md) | Reserved boundary for future hosted services |
| [examples/README.md](examples/README.md) | Example prompts and starter workflows |

## Specialized References And Session Notes

| Document | Description |
|----------|-------------|
| [UE5_CPP_API_REFERENCE.md](UE5_CPP_API_REFERENCE.md) | UE-facing API reference for tool authors and maintainers |
| [TEP_SPEC.md](TEP_SPEC.md) | Frozen Tool Execution Proof format reference |
| [SESSION_FIXES_2026_04_14.md](SESSION_FIXES_2026_04_14.md) | Log-aware repair session note for a major tool hardening pass |
| [../Bridge/toolbook/CONTRACT_GENERATION_REPORT.md](../Bridge/toolbook/CONTRACT_GENERATION_REPORT.md) | Generated contract report; useful as evidence, not as product narrative |

## AI School

### Environment Track

| Document | Description |
|----------|-------------|
| [AI_School/01_Nature_And_Ecosystems.md](AI_School/01_Nature_And_Ecosystems.md) | How real ecosystems work |
| [AI_School/02_UE5_Environment_Systems.md](AI_School/02_UE5_Environment_Systems.md) | Landscape, foliage, PCG, and material systems |
| [AI_School/03_Anti_Patterns.md](AI_School/03_Anti_Patterns.md) | Common mistakes to avoid |
| [AI_School/04_Workflow.md](AI_School/04_Workflow.md) | Recommended build order |
| [AI_School/05_Tool_Guide.md](AI_School/05_Tool_Guide.md) | Which RiftbornAI tools to use for each job |

### VFX / Niagara Track

| Document | Description |
|----------|-------------|
| [AI_School/VFX/01_Gameplay_Feedback_And_Readability.md](AI_School/VFX/01_Gameplay_Feedback_And_Readability.md) | How players read gameplay VFX and what makes an effect legible |
| [AI_School/VFX/02_UE5_Niagara_Systems.md](AI_School/VFX/02_UE5_Niagara_Systems.md) | Niagara systems, user parameters, editor context, and compile/inspect flow |
| [AI_School/VFX/03_Anti_Patterns.md](AI_School/VFX/03_Anti_Patterns.md) | Common VFX failure modes and what to do instead |
| [AI_School/VFX/04_Workflow.md](AI_School/VFX/04_Workflow.md) | Recommended build order for gameplay and ambient effects |
| [AI_School/VFX/05_Tool_Guide.md](AI_School/VFX/05_Tool_Guide.md) | Which RiftbornAI tools to use for Niagara authoring and verification |

### GAS / Ability Track

| Document | Description |
|----------|-------------|
| [AI_School/GAS/01_Ability_Design_And_State_Model.md](AI_School/GAS/01_Ability_Design_And_State_Model.md) | How to design abilities as gameplay contracts with costs, targeting, timing, and state changes |
| [AI_School/GAS/02_UE5_GAS_Systems.md](AI_School/GAS/02_UE5_GAS_Systems.md) | ASC, abilities, effects, attributes, gameplay tags, and current RiftbornAI support |
| [AI_School/GAS/03_Anti_Patterns.md](AI_School/GAS/03_Anti_Patterns.md) | Common GAS authoring and wiring mistakes to avoid |
| [AI_School/GAS/04_Workflow.md](AI_School/GAS/04_Workflow.md) | Recommended build order for ability-system work |
| [AI_School/GAS/05_Tool_Guide.md](AI_School/GAS/05_Tool_Guide.md) | Which RiftbornAI tools to use for GAS authoring and runtime verification |

### Blueprint / Graph Track

| Document | Description |
|----------|-------------|
| [AI_School/Blueprint/01_Architecture_And_Communication.md](AI_School/Blueprint/01_Architecture_And_Communication.md) | How to structure Blueprint classes, responsibilities, and communication paths |
| [AI_School/Blueprint/02_UE5_Blueprint_Systems.md](AI_School/Blueprint/02_UE5_Blueprint_Systems.md) | Blueprint graph types, editor-native surfaces, and current supported tool lanes |
| [AI_School/Blueprint/03_Anti_Patterns.md](AI_School/Blueprint/03_Anti_Patterns.md) | Common Blueprint authoring mistakes and why they create graph debt |
| [AI_School/Blueprint/04_Workflow.md](AI_School/Blueprint/04_Workflow.md) | Recommended build order for Blueprint classes and graph edits |
| [AI_School/Blueprint/05_Tool_Guide.md](AI_School/Blueprint/05_Tool_Guide.md) | Which RiftbornAI tools to use for Blueprint authoring, repair, and verification |

### Audio / MetaSound Track

| Document | Description |
|----------|-------------|
| [AI_School/Audio/01_Game_Audio_And_Mix_Intent.md](AI_School/Audio/01_Game_Audio_And_Mix_Intent.md) | How game audio supports readability, mood, mix separation, and player feedback |
| [AI_School/Audio/02_UE5_Audio_And_Metasound_Systems.md](AI_School/Audio/02_UE5_Audio_And_Metasound_Systems.md) | MetaSound assets, graph flow, preview, emitter placement, and current supported audio tools |
| [AI_School/Audio/03_Anti_Patterns.md](AI_School/Audio/03_Anti_Patterns.md) | Common procedural-audio and ambience mistakes to avoid |
| [AI_School/Audio/04_Workflow.md](AI_School/Audio/04_Workflow.md) | Recommended build order for MetaSound and world-audio work |
| [AI_School/Audio/05_Tool_Guide.md](AI_School/Audio/05_Tool_Guide.md) | Which RiftbornAI tools to use for audio authoring, placement, acoustics, and verification |

### UI / Widget Track

| Document | Description |
|----------|-------------|
| [AI_School/UI/01_Interface_Hierarchy_And_Player_Focus.md](AI_School/UI/01_Interface_Hierarchy_And_Player_Focus.md) | How UI surfaces direct attention, reduce decision cost, and stay readable in play |
| [AI_School/UI/02_UE5_Widget_And_UI_Verification_Systems.md](AI_School/UI/02_UE5_Widget_And_UI_Verification_Systems.md) | Widget Blueprint authoring, hierarchy inspection, layout audit, navigation, and supported UI verification lanes |
| [AI_School/UI/03_Anti_Patterns.md](AI_School/UI/03_Anti_Patterns.md) | Common HUD, menu, and navigation mistakes to avoid |
| [AI_School/UI/04_Workflow.md](AI_School/UI/04_Workflow.md) | Recommended build order for widget structure, interaction flow, and live verification |
| [AI_School/UI/05_Tool_Guide.md](AI_School/UI/05_Tool_Guide.md) | Which RiftbornAI tools to use for widget creation, compile, navigation, and UI proof |

### Animation Track

| Document | Description |
|----------|-------------|
| [AI_School/Animation/01_Motion_Intent_And_Readability.md](AI_School/Animation/01_Motion_Intent_And_Readability.md) | How animation communicates intent, state, anticipation, recovery, and gameplay readability |
| [AI_School/Animation/02_UE5_Animation_And_Retargeting_Systems.md](AI_School/Animation/02_UE5_Animation_And_Retargeting_Systems.md) | Animation Blueprints, blend spaces, montages, retargeting, pose search, and current supported animation tools |
| [AI_School/Animation/03_Anti_Patterns.md](AI_School/Animation/03_Anti_Patterns.md) | Common animation-pipeline and runtime-motion mistakes to avoid |
| [AI_School/Animation/04_Workflow.md](AI_School/Animation/04_Workflow.md) | Recommended build order for locomotion, one-shot actions, retargeting, and proof |
| [AI_School/Animation/05_Tool_Guide.md](AI_School/Animation/05_Tool_Guide.md) | Which RiftbornAI tools to use for AnimBP authoring, retargeting, motion matching setup, and verification |

### Input Track

| Document | Description |
|----------|-------------|
| [AI_School/Input/01_Player_Intent_And_Control_Grammar.md](AI_School/Input/01_Player_Intent_And_Control_Grammar.md) | How controls should map player intent to stable, learnable game verbs |
| [AI_School/Input/02_UE5_Enhanced_Input_Systems.md](AI_School/Input/02_UE5_Enhanced_Input_Systems.md) | Enhanced Input actions, contexts, mappings, beta-gated modifier depth, and current supported input tools |
| [AI_School/Input/03_Anti_Patterns.md](AI_School/Input/03_Anti_Patterns.md) | Common control-scheme and mapping-context mistakes to avoid |
| [AI_School/Input/04_Workflow.md](AI_School/Input/04_Workflow.md) | Recommended build order for action design, context layout, and runtime proof |
| [AI_School/Input/05_Tool_Guide.md](AI_School/Input/05_Tool_Guide.md) | Which RiftbornAI tools to use for input asset creation, mappings, and validation |

### Cinematics Track

| Document | Description |
|----------|-------------|
| [AI_School/Cinematics/01_Shot_Intent_And_Visual_Storytelling.md](AI_School/Cinematics/01_Shot_Intent_And_Visual_Storytelling.md) | How shots communicate story beats, orientation, emphasis, and rhythm |
| [AI_School/Cinematics/02_UE5_Sequencer_And_Camera_Systems.md](AI_School/Cinematics/02_UE5_Sequencer_And_Camera_Systems.md) | Level Sequences, camera setup, Sequencer context tools, bindings, and current supported cinematic tools |
| [AI_School/Cinematics/03_Anti_Patterns.md](AI_School/Cinematics/03_Anti_Patterns.md) | Common shot-design and sequence-authoring mistakes to avoid |
| [AI_School/Cinematics/04_Workflow.md](AI_School/Cinematics/04_Workflow.md) | Recommended build order for shot planning, sequence binding, and render proof |
| [AI_School/Cinematics/05_Tool_Guide.md](AI_School/Cinematics/05_Tool_Guide.md) | Which RiftbornAI tools to use for sequences, cameras, bindings, review, and rendering |

### Physics Track

| Document | Description |
|----------|-------------|
| [AI_School/Physics/01_Force_Collision_And_Physical_Readability.md](AI_School/Physics/01_Force_Collision_And_Physical_Readability.md) | How collision and physics should communicate solidity, danger, motion, and consequence |
| [AI_School/Physics/02_UE5_Collision_Constraint_And_Destruction_Systems.md](AI_School/Physics/02_UE5_Collision_Constraint_And_Destruction_Systems.md) | Collision presets, traces, constraints, Chaos destruction, rollback snapshots, and current supported physics tools |
| [AI_School/Physics/03_Anti_Patterns.md](AI_School/Physics/03_Anti_Patterns.md) | Common collision, constraint, and destruction mistakes to avoid |
| [AI_School/Physics/04_Workflow.md](AI_School/Physics/04_Workflow.md) | Recommended build order for collision setup, physical behavior, and proof |
| [AI_School/Physics/05_Tool_Guide.md](AI_School/Physics/05_Tool_Guide.md) | Which RiftbornAI tools to use for collision authoring, traces, constraints, destruction, and validation |

### Streaming Track

| Document | Description |
|----------|-------------|
| [AI_School/Streaming/01_Spatial_Loading_And_Player_Presence.md](AI_School/Streaming/01_Spatial_Loading_And_Player_Presence.md) | How streaming should preserve continuity, presence, and world-state clarity as the player moves |
| [AI_School/Streaming/02_UE5_World_Partition_And_Data_Layer_Systems.md](AI_School/Streaming/02_UE5_World_Partition_And_Data_Layer_Systems.md) | World Partition, data layers, streaming sources, and current supported streaming diagnostics and authoring tools |
| [AI_School/Streaming/03_Anti_Patterns.md](AI_School/Streaming/03_Anti_Patterns.md) | Common world-streaming and data-layer mistakes to avoid |
| [AI_School/Streaming/04_Workflow.md](AI_School/Streaming/04_Workflow.md) | Recommended build order for streaming-state design, assignment, and residency proof |
| [AI_School/Streaming/05_Tool_Guide.md](AI_School/Streaming/05_Tool_Guide.md) | Which RiftbornAI tools to use for data layers, streaming sources, partition checks, and diagnostics |

### Networking Track

| Document | Description |
|----------|-------------|
| [AI_School/Networking/01_Authority_Visibility_And_Network_Truth.md](AI_School/Networking/01_Authority_Visibility_And_Network_Truth.md) | How multiplayer systems should separate authority, visibility, and replication cost |
| [AI_School/Networking/02_UE5_Replication_Audit_And_Iris_Group_Systems.md](AI_School/Networking/02_UE5_Replication_Audit_And_Iris_Group_Systems.md) | Replication audits, actor inspection, Iris groups, and current supported networking tools |
| [AI_School/Networking/03_Anti_Patterns.md](AI_School/Networking/03_Anti_Patterns.md) | Common replication-planning and grouping mistakes to avoid |
| [AI_School/Networking/04_Workflow.md](AI_School/Networking/04_Workflow.md) | Recommended build order for authority planning, replication audit, and Iris grouping |
| [AI_School/Networking/05_Tool_Guide.md](AI_School/Networking/05_Tool_Guide.md) | Which RiftbornAI tools to use for network audits, actor replication inspection, and grouping |

### Save / Load Track

| Document | Description |
|----------|-------------|
| [AI_School/SaveLoad/01_Persistence_State_And_Restore_Intent.md](AI_School/SaveLoad/01_Persistence_State_And_Restore_Intent.md) | How persistence should define meaningful state, continuity, and restore expectations |
| [AI_School/SaveLoad/02_UE5_Snapshot_Checkpoint_And_State_Digest_Systems.md](AI_School/SaveLoad/02_UE5_Snapshot_Checkpoint_And_State_Digest_Systems.md) | Level Snapshots, scene checkpoints, world-state digests, and current supported save/load tools |
| [AI_School/SaveLoad/03_Anti_Patterns.md](AI_School/SaveLoad/03_Anti_Patterns.md) | Common persistence and restore mistakes to avoid |
| [AI_School/SaveLoad/04_Workflow.md](AI_School/SaveLoad/04_Workflow.md) | Recommended build order for checkpointing, restoration, and verification |
| [AI_School/SaveLoad/05_Tool_Guide.md](AI_School/SaveLoad/05_Tool_Guide.md) | Which RiftbornAI tools to use for snapshots, checkpoints, state digests, and restore proof |

### Localization Track

| Document | Description |
|----------|-------------|
| [AI_School/Localization/01_Text_Intent_Keys_And_Cultural_Clarity.md](AI_School/Localization/01_Text_Intent_Keys_And_Cultural_Clarity.md) | How localized text should preserve intent, context, and player comprehension across cultures |
| [AI_School/Localization/02_UE5_String_Table_And_Culture_Systems.md](AI_School/Localization/02_UE5_String_Table_And_Culture_Systems.md) | String Tables, entry authoring, optional culture inspection helpers, and current supported localization tools |
| [AI_School/Localization/03_Anti_Patterns.md](AI_School/Localization/03_Anti_Patterns.md) | Common localization and string-key mistakes to avoid |
| [AI_School/Localization/04_Workflow.md](AI_School/Localization/04_Workflow.md) | Recommended build order for key design, String Table authoring, and verification |
| [AI_School/Localization/05_Tool_Guide.md](AI_School/Localization/05_Tool_Guide.md) | Which RiftbornAI tools to use for String Tables, entries, and localization-safe verification |

### AI School Meta Docs

| Document | Description |
|----------|-------------|
| [AI_School/TASK_INTAKE_AND_PREFLIGHT.md](AI_School/TASK_INTAKE_AND_PREFLIGHT.md) | Defines the preflight for real mutation scope, editor blockers, implementation lane choice, tool-route verification, and proof planning |
| [AI_School/TRACK_SELECTION.md](AI_School/TRACK_SELECTION.md) | Explains how to choose the smallest correct set of AI School tracks for mixed-domain work |
| [AI_School/VERIFICATION_LADDER.md](AI_School/VERIFICATION_LADDER.md) | Defines the shared proof ladder from structure checks to runtime and regression verification |
| [AI_School/TASK_PLAYBOOKS.md](AI_School/TASK_PLAYBOOKS.md) | Provides reusable multi-domain feature recipes for common slices such as abilities, traversal, playable areas, cinematics, destructibles, and text/UI updates |
| [AI_School/COVERAGE_BOUNDARIES.md](AI_School/COVERAGE_BOUNDARIES.md) | Explains why `GameDesignSystem` and `AchievementDesignSystem` are not yet treated as public AI School tracks and how to verify future tool-surface additions |

### Modeling Track

| Document | Description |
|----------|-------------|
| [AI_School/Modeling/01_Form_Silhouette_And_Modularity.md](AI_School/Modeling/01_Form_Silhouette_And_Modularity.md) | How modeled assets should be driven by function, silhouette, modularity, and collision promise |
| [AI_School/Modeling/02_UE5_Geometry_Script_And_Mesh_Hardening_Systems.md](AI_School/Modeling/02_UE5_Geometry_Script_And_Mesh_Hardening_Systems.md) | Dynamic mesh authoring, Geometry Script operations, cleanup, export, collision, LOD, and Nanite on the current supported surface |
| [AI_School/Modeling/03_Anti_Patterns.md](AI_School/Modeling/03_Anti_Patterns.md) | Common modeling-surface and topology-authoring mistakes to avoid |
| [AI_School/Modeling/04_Workflow.md](AI_School/Modeling/04_Workflow.md) | Recommended build order for mesh generation, local edits, cleanup, export, and proof |
| [AI_School/Modeling/05_Tool_Guide.md](AI_School/Modeling/05_Tool_Guide.md) | Which RiftbornAI tools to use for modeling, cleanup, export, collision, and LOD/Nanite hardening |

### Level Design / Blockout Track

| Document | Description |
|----------|-------------|
| [AI_School/LevelDesign/01_Player_Flow_And_Spatial_Readability.md](AI_School/LevelDesign/01_Player_Flow_And_Spatial_Readability.md) | How players read routes, landmarks, cover, pacing, and combat space |
| [AI_School/LevelDesign/02_UE5_Level_And_Blockout_Systems.md](AI_School/LevelDesign/02_UE5_Level_And_Blockout_Systems.md) | Map creation, blockout generation, dungeon planning, navmesh, world partition, and current supported tools |
| [AI_School/LevelDesign/03_Anti_Patterns.md](AI_School/LevelDesign/03_Anti_Patterns.md) | Common level-layout and blockout mistakes to avoid |
| [AI_School/LevelDesign/04_Workflow.md](AI_School/LevelDesign/04_Workflow.md) | Recommended build order for level layout, traversal, and verification |
| [AI_School/LevelDesign/05_Tool_Guide.md](AI_School/LevelDesign/05_Tool_Guide.md) | Which RiftbornAI tools to use for maps, blockouts, navigation, and review |

### C++ Architecture Track

| Document | Description |
|----------|-------------|
| [AI_School/CppArchitecture/01_Class_Boundaries_And_Responsibilities.md](AI_School/CppArchitecture/01_Class_Boundaries_And_Responsibilities.md) | How to choose clean gameplay class boundaries and ownership in Unreal C++ |
| [AI_School/CppArchitecture/02_UE5_CPP_Authoring_And_Build_Systems.md](AI_School/CppArchitecture/02_UE5_CPP_Authoring_And_Build_Systems.md) | C++ generation, source-edit, Blueprint-conversion, build, reload, and validation lanes |
| [AI_School/CppArchitecture/03_Anti_Patterns.md](AI_School/CppArchitecture/03_Anti_Patterns.md) | Common Unreal C++ architecture and generation mistakes to avoid |
| [AI_School/CppArchitecture/04_Workflow.md](AI_School/CppArchitecture/04_Workflow.md) | Recommended build order for gameplay C++ work and verification |
| [AI_School/CppArchitecture/05_Tool_Guide.md](AI_School/CppArchitecture/05_Tool_Guide.md) | Which RiftbornAI tools to use for C++ authoring, conversion, build, and runtime proof |

---

## Quick Reference

### Network Ports

| Port | Protocol | Service |
|------|----------|---------|
| 8767 | HTTP | Production bridge for governed tool execution |
| 8765 | TCP | Internal legacy Python lane for diagnostics and recovery only |

### Tool Tiers

| Tier | Default | Description |
|------|---------|-------------|
| PRODUCTION | Visible | Curated shipped editor automation surface |
| BETA | Opt-in | Governed but not part of the default customer contract |
| EXPERIMENTAL | Hidden | Internal or R&D surface |

### Scope Note

This index aims to cover the tracked documentation corpus that defines product truth, setup, boundaries, examples, and maintenance. Generated reports, research artifacts, or internal-only notes can still exist outside this map, but they should not override the documents linked here unless a canonical doc says otherwise.
