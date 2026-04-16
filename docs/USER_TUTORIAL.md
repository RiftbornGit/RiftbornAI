# RiftbornAI User Tutorial

This document explains what RiftbornAI can actually do today, how to ask for it, and where to look next when you want deeper domain guidance.

RiftbornAI is not just a tool registry. It is a governed Unreal Editor copilot that can:

- inspect the current editor and viewport
- mutate the project through registered UE 5.7 tools
- verify results with screenshots, diagnostics, and compile/runtime checks
- iterate until the requested task is actually done or a real blocker is found

It works through two front doors:

- the built-in Unreal Editor copilot panel
- an external MCP client such as Codex, Claude Code, Cursor, or VS Code

Both front doors drive the same underlying tool surface.

---

## How To Use This Document

This file is the user-facing map of the product.

Use it for:

- understanding the major jobs RiftbornAI can help with
- learning how to phrase requests so the copilot is effective
- finding the right follow-up docs when a task becomes deep or domain-specific

Do not treat it as a raw list of every tool in the repo. RiftbornAI is too large for that to be useful in a first-line tutorial.

If you need exact parameters or tool names:

- ask RiftbornAI to `describe_tool`
- ask for `list_all_tools`
- read [UE57_SUPER_SYSTEMS_TOOLS.md](UE57_SUPER_SYSTEMS_TOOLS.md)

---

## Start With Natural Language

You usually do not need to remember tool names.

Good requests:

- "What is in this level right now?"
- "Create a flat 500m landscape and light it for golden hour."
- "Make this cliff material feel deeper with a heightmap-based effect."
- "Find the Blueprint compile errors blocking PIE and fix them."
- "Set up a simple third-person playable character and test it."

When you do want exact tool details, ask RiftbornAI:

- "Describe the tool create_silpom_material."
- "What are the parameters for create_landscape?"
- "List the tools that help with Niagara."

The tool-help surface is there for teaching and precision. The normal workflow is still plain English.

---

## How To Ask Well

RiftbornAI works best when the request is outcome-first and grounded in the real mutation.

Strong asks:

- "Create a flat landscape centered at 0,0, add a basic ground material, and stop before foliage."
- "Fix the Blueprint compile errors blocking PIE and tell me what changed."
- "Create a rocky cliff material with silPOM from these textures and assign it to the selected mesh."
- "Playtest this encounter area and tell me whether cover spacing feels fair."
- "Audit replication on this actor and tell me what is missing for multiplayer."

Weak asks:

- "Make the game better."
- "Do some Unreal stuff."
- "Use a bunch of material tools."
- "Make it AAA."

The more useful pattern is:

1. say what should change
2. say what object or domain it applies to
3. say what proof you want

Examples:

- "Make this cave feel colder and wetter, then show me a screenshot."
- "Create a simple enemy spawner Blueprint and verify it compiles."
- "Set up a quest popup widget and test that it appears in PIE."

---

## What RiftbornAI Can Do

### 1. Observe The Editor And Current Scene

RiftbornAI can inspect the live editor before it changes anything.

Common uses:

- understand what is already in the level
- capture screenshots
- inspect actors, assets, and editor context
- detect GUI blockers, modal dialogs, and compile issues

Typical examples:

- "Observe the current level and tell me what is missing."
- "Take a screenshot and tell me whether the lighting is balanced."
- "Check whether a popup or compile dialog is blocking PIE."

Representative tools:

- `observe_ue_project`
- `analyze_scene_screenshot`
- `get_editor_gui_state`
- `get_output_log_context`
- `get_compile_diagnostics`

### 2. Build Terrain, Environment, And Atmosphere

RiftbornAI can create and shape playable worlds, not just place random actors.

Common uses:

- create landscapes
- apply terrain materials
- attach grass and foliage systems
- add sky, fog, sunlight, water, and atmosphere
- build blockouts and traversal spaces

Typical examples:

- "Create a perfectly flat landscape centered at 0,0 and make it ready for sculpting."
- "Build a rocky valley with fog, warm sunset light, and scattered pine trees."
- "Add a riverbed and make the scene feel colder and wetter."

Representative tools:

- `create_landscape`
- `create_day_night_cycle`
- `apply_post_process_style`
- landscape, foliage, and water authoring tools in the default production lane

If you are doing serious environment work, also read the Environment track in [AI_School](AI_School/01_Nature_And_Ecosystems.md).

### 3. Create Materials, Textures, And Surface Effects

RiftbornAI has a large material authoring surface. It can create materials, assign them, wire landscape helpers, and build more specialized effects.

Common uses:

- create or assign PBR materials
- import or generate textures
- build terrain and landscape material helpers
- create more advanced surface effects such as silhouette-aware parallax materials

Typical examples:

- "Create a wet mossy cliff material and assign it to this mesh."
- "Generate a gravel texture and import it into /Game/Generated/Textures."
- "Build a silhouette-corrected POM material from this height map."

Representative tools:

- `generate_texture`
- `assign_mesh_material`
- `create_vme_material`
- `create_silpom_material`

### silPOM: What It Is

`create_silpom_material` builds a silhouette-corrected Parallax Occlusion Mapping material.

In plain terms:

- regular POM creates the illusion of depth inside the surface
- silPOM also pushes the mesh silhouette so the outer contour better matches the height map
- it is useful for cliffs, bark, stone, roots, broken masonry, and other surfaces where profile depth matters

When it is a good fit:

- you have a height or displacement texture
- the surface should read as thick or layered from shallow viewing angles
- you want more depth without paying for a fully modeled high-poly mesh

When it is not the right fit:

- tiny props where the cost is not justified
- flat painted materials with no meaningful height structure
- cases where actual modeled geometry is the better answer

Key parameters you will usually care about:

- `height_texture`
- `albedo_texture`
- `normal_texture`
- `parallax_scale`
- `silhouette_strength`
- `uv_mode`
- `pom_steps`

Example ask:

- "Create a silPOM cliff material named M_Cliff_SilPOM in /Game/Materials using this height map and normal map, with moderate silhouette strength."

### 4. Author Blueprints And Gameplay Scaffolding

RiftbornAI can create, edit, compile, inspect, and repair Blueprints through governed editor workflows.

Common uses:

- create new Blueprints
- add components, variables, and functions
- inspect compile diagnostics
- refresh broken nodes
- repair common graph failures that block playtesting

Typical examples:

- "Create a Blueprint interactable door with a mesh, collision, and open event."
- "Open this Blueprint, compile it, and tell me what is broken."
- "Fix the Blueprint errors that are blocking Play In Editor."

Representative tools:

- Blueprint creation and graph-editing tools
- `compile_blueprint`
- `get_blueprint_compile_diagnostics`
- `assert_blueprint_compiles`

For deeper Blueprint work, read the Blueprint track in [AI_School](AI_School/Blueprint/01_Architecture_And_Communication.md).

### 5. Build Characters, Gameplay Systems, And Runtime Content

RiftbornAI can scaffold gameplay-facing content such as characters, input, GAS, AI behavior, animation, and related systems.

Common uses:

- create or wire playable characters
- set default pawns and controllers
- set up input
- build or inspect GAS assets
- work with animation, Control Rig, and motion systems

Typical examples:

- "Create a basic third-person playable character and make it the default pawn."
- "Audit the current replication setup for this actor."
- "Set up a simple dash ability and verify it works in play."

This is where the domain tracks matter most. If the task is serious, load the matching AI School track before big edits:

- [GAS](AI_School/GAS/01_Ability_Design_And_State_Model.md)
- [Animation](AI_School/Animation/01_Motion_Intent_And_Readability.md)
- [Input](AI_School/Input/01_Player_Intent_And_Control_Grammar.md)
- [Networking](AI_School/Networking/01_Authority_Visibility_And_Network_Truth.md)

### 6. Create VFX, Audio, UI, Cinematics, And PCG

RiftbornAI is not limited to geometry and Blueprints. It can also help with presentation systems.

Common uses:

- Niagara VFX authoring and validation
- MetaSound and world audio setup
- widget and HUD work
- Level Sequence and camera setup
- PCG graph inspection and authoring

Typical examples:

- "Create ambient dust motes in this sunbeam and verify the Niagara system compiles."
- "Add a windy forest ambience with proper spatial falloff."
- "Create a simple quest popup widget and test its visibility in PIE."
- "Bind this actor into the current sequence and verify the binding."

Recommended follow-up docs:

- [VFX track](AI_School/VFX/01_Gameplay_Feedback_And_Readability.md)
- [Audio track](AI_School/Audio/01_Game_Audio_And_Mix_Intent.md)
- [UI track](AI_School/UI/01_Interface_Hierarchy_And_Player_Focus.md)
- [Cinematics track](AI_School/Cinematics/01_Shot_Intent_And_Visual_Storytelling.md)

### 7. Playtest, Verify, And Diagnose Problems

One of RiftbornAI's strongest advantages is that it can verify instead of stopping at asset creation.

Common uses:

- start and stop PIE
- inspect runtime problems
- catch Blueprint compile blockers
- run diagnostics
- audit performance, visibility, and project health

Typical examples:

- "Start PIE, watch for blockers, and continue through the Blueprint error popup if it appears."
- "Playtest this area and tell me what feels broken."
- "Give me a project insights report and rank the biggest risks."

Representative tools:

- `start_pie`
- `stop_pie`
- `run_diagnostic`
- project insights, performance, and verification tools

---

## Capability Atlas

The sections above cover the most common first-use workflows. This section broadens the picture so users understand how large the product actually is.

### 8. Import, Search, And Organize Assets

RiftbornAI can help bring content into the project and then reason about what is already there.

Common uses:

- import textures, meshes, and audio
- search for assets by path or type
- inspect project structure
- assign materials or fix missing references

Typical examples:

- "Import this texture set into /Game/Materials/Rock_Cliff."
- "Search the project for all Niagara systems related to sparks."
- "Find the assets that look like they belong to this broken material setup."

This is useful both for clean authoring and for recovery work when old content drifted or asset references broke.

### 9. Modeling, Geometry Script, And Mesh Hardening

RiftbornAI can operate on modeled content, not just place finished assets.

Common uses:

- generate or edit dynamic mesh geometry
- build blockout or utility meshes
- export dynamic meshes to static assets
- apply collision, LOD, UV, and Nanite hardening passes

Typical examples:

- "Create a simple stone arch blockout and export it as a static mesh."
- "Generate collision for this mesh and verify it behaves correctly."
- "Create a grass card mesh for foliage use."

Recommended follow-up docs:

- [Modeling track](AI_School/Modeling/01_Form_Silhouette_And_Modularity.md)
- [C++ Architecture track](AI_School/CppArchitecture/01_Class_Boundaries_And_Responsibilities.md) for deeper authoring logic around generated gameplay classes and components

### 10. Level Design, Blockout, Navigation, And Flow

RiftbornAI is not only an art helper. It can help structure playable spaces.

Common uses:

- create maps and blockouts
- build navigable arenas and traversal spaces
- inspect player flow
- audit cover layout and combat readability
- build or validate navmesh

Typical examples:

- "Generate a small arena blockout with cover clusters and clear flanking routes."
- "Audit cover layout in this encounter area."
- "Build navmesh and tell me whether traversal is broken anywhere obvious."

Recommended follow-up docs:

- [Level Design track](AI_School/LevelDesign/01_Player_Flow_And_Spatial_Readability.md)

### 11. Physics, Collision, Traces, And Destruction

RiftbornAI can help with physical interaction systems and collision correctness.

Common uses:

- inspect and configure collision
- run traces to validate interaction assumptions
- set up constraints and rigid-body behavior
- work with Chaos destruction and Geometry Collections

Typical examples:

- "Make this door block the player but overlap projectiles."
- "Run traces and tell me whether this chest lid is hittable."
- "Set up a breakable prop and verify the destruction behavior."

Recommended follow-up docs:

- [Physics track](AI_School/Physics/01_Force_Collision_And_Physical_Readability.md)

### 12. Streaming, World Partition, And Data Layers

RiftbornAI can inspect and modify large-world residency and state partitioning.

Common uses:

- create or inspect data layers
- configure streaming sources
- analyze World Partition state
- validate that world regions and actor groupings make sense

Typical examples:

- "Show me the current data layers affecting this area."
- "Set up a streaming source around the player start."
- "Audit whether this level's partitioning looks coherent."

Recommended follow-up docs:

- [Streaming track](AI_School/Streaming/01_Spatial_Loading_And_Player_Presence.md)

### 13. Networking, Replication, And Multiplayer Readiness

RiftbornAI can help with multiplayer-facing inspection and setup.

Common uses:

- audit replication setup
- inspect actor-level replication policy
- configure non-Iris replication properties
- start multi-client PIE for testing
- work with Iris grouping where supported

Typical examples:

- "Audit this actor's replication settings."
- "Start multi-client PIE and verify that this actor stays relevant at range."
- "Tell me what in this setup is likely to break in multiplayer."

Recommended follow-up docs:

- [Networking track](AI_School/Networking/01_Authority_Visibility_And_Network_Truth.md)

### 14. Save, Load, Snapshots, And Restore

RiftbornAI has useful editor-world persistence helpers even when the task is not a full player-facing save system.

Common uses:

- create snapshots
- create checkpoints
- compare state before and after changes
- restore editor-world state during iteration

Typical examples:

- "Checkpoint this scene before we make destructive changes."
- "Capture a state digest before the optimization pass."
- "Restore the previous scene state if the result looks worse."

Recommended follow-up docs:

- [Save/Load track](AI_School/SaveLoad/01_Persistence_State_And_Restore_Intent.md)

### 15. Localization And String Tables

RiftbornAI can help with localized text infrastructure, not just visual/gameplay content.

Common uses:

- create String Tables
- add localization entries
- organize keys more intentionally
- inspect text-related structure before translation work scales

Typical examples:

- "Create a String Table for quest text."
- "Add localized keys for this interaction prompt set."
- "Help me clean up unstable or poorly named localization keys."

Recommended follow-up docs:

- [Localization track](AI_School/Localization/01_Text_Intent_Keys_And_Cultural_Clarity.md)

### 16. C++ Authoring, Build, And Blueprint Conversion

RiftbornAI can help beyond Blueprints. It can also work on Unreal C++ gameplay code and related verification loops.

Common uses:

- generate or modify gameplay C++ classes
- run builds and diagnose compile failures
- inspect whether a Blueprint is a good candidate for C++ conversion
- keep Blueprint and C++ boundaries honest

Typical examples:

- "Create a reusable ActorComponent for this interaction logic."
- "Build the project and tell me what broke."
- "Convert this Blueprint logic into a C++ gameplay class if that is the correct boundary."

Recommended follow-up docs:

- [C++ Architecture track](AI_School/CppArchitecture/01_Class_Boundaries_And_Responsibilities.md)

### 17. Performance, Project Health, And Production Readiness

RiftbornAI also helps when the question is not "how do I build this?" but "is this healthy?"

Common uses:

- inspect FPS, memory, draw-call, and scene complexity signals
- produce project insight reports
- audit readiness, tool coverage, and repair opportunities
- surface issues in Blueprints, materials, assets, and gameplay structure

Typical examples:

- "Give me a project insights report."
- "Analyze draw-call pressure in this level."
- "Find the highest-risk issues before we continue building."

These workflows are especially useful before a major refactor, before merging AI-generated content, or before shipping a feature slice.

---

## Example Requests By Domain

If you want a quicker prompt cookbook, start here.

Environment:

- "Create a 500m flat landscape, add a dirt-and-grass material, and stop before foliage."
- "Make this valley moodier with colder fog and a lower sun angle."

Materials:

- "Create a silPOM bark material from these textures and assign it to the selected mesh."
- "Generate a gravel texture, import it, and build a simple material instance."

Blueprints:

- "Create a Blueprint interactable pickup with mesh, overlap sphere, and pickup event."
- "Compile this Blueprint and repair the node issues blocking PIE."

Gameplay:

- "Create a simple third-person character and set it as the default pawn."
- "Set up a dash ability and verify it works during playtest."

VFX and audio:

- "Create ambient dust Niagara in this room and verify the system compiles."
- "Add a cave drip ambience with believable spatial falloff."

UI:

- "Create a quest prompt widget with title, body text, and confirm button."
- "Test whether this widget actually appears and can be navigated in PIE."

Cinematics:

- "Create a short reveal shot around this statue and bind the camera properly."

Networking:

- "Audit replication for this actor and tell me what is missing for multiplayer."

Performance:

- "Analyze scene complexity and list the most expensive visible actors."

Recovery:

- "Find what is blocking PIE and fix the Blueprint or modal issues first."
- "Inspect the output log and tell me the real root cause, not the downstream spam."

---

## What RiftbornAI Does Not Magically Solve

RiftbornAI is strong, but it is not honest to present it as unlimited.

Things to keep in mind:

- some tools are production-ready, some are beta-gated, and some are internal-only
- not every domain helper is guaranteed to exist in the current build
- a generated asset is not the same as a shipped design
- complex tasks still need verification in the real editor and often in PIE

If you are unsure whether a specific tool exists or is ready:

- ask for `list_all_tools`
- ask for `describe_tool`
- check [UE57_SUPER_SYSTEMS_TOOLS.md](UE57_SUPER_SYSTEMS_TOOLS.md)

---

## How Big Tasks Usually Work

The most successful RiftbornAI sessions are not single blind tool calls. They follow a loop.

Typical pattern:

1. inspect the current state
2. decide the real mutation
3. make a bounded change
4. verify it structurally or visually
5. iterate

Example:

- "Observe this encounter area."
- "Identify the biggest readability problem."
- "Fix only that problem."
- "Start PIE and tell me whether the result is actually better."

This is why the product feels larger than a normal Unreal helper plugin. It is not just a library of actions. It is built around observe -> plan -> execute -> verify.

---

## Suggested First Workflows

If you are new to RiftbornAI, these are the best first exercises:

### A. Scene Read + Polish

1. Ask RiftbornAI to observe the level.
2. Ask it to identify the biggest visual problem.
3. Ask it to fix lighting, fog, or material coherence.
4. Ask it to verify with a new screenshot.

### B. Material Upgrade With silPOM

1. Import or point at a height map and supporting textures.
2. Ask RiftbornAI to create a `silPOM` material.
3. Assign it to a rock or cliff mesh.
4. Ask for a screenshot from a shallow angle to verify the profile depth.

### C. Blueprint Repair Pass

1. Ask RiftbornAI to compile the target Blueprint or prepare PIE.
2. Let it inspect diagnostics.
3. Ask it to refresh, repair, or remove broken graph links.
4. Verify with compile plus PIE.

### D. Environment Build Slice

1. Create a landscape.
2. Add a terrain material.
3. Attach grass or foliage logic.
4. Add atmosphere and key lighting.
5. Playtest traversal and readability.

### E. Project Health Pass

1. Ask RiftbornAI to inspect project health or generate insights.
2. Let it rank the highest-risk issues.
3. Pick one issue category such as Blueprint compile failures or material breakage.
4. Fix that category.
5. Re-run the report.

---

## Where To Go Next

- Start with [GETTING_STARTED.md](GETTING_STARTED.md) if you need setup and first-run flow.
- Read [UE57_SUPER_SYSTEMS_TOOLS.md](UE57_SUPER_SYSTEMS_TOOLS.md) if you want the deeper tool reference.
- Use the matching [AI School](AI_School/README.md) track when the task becomes domain-heavy.
- Use `describe_tool` when you want parameter-level help for a specific capability.

If you only remember one thing, remember this:

Ask for outcomes, not button presses.

Good:

- "Make this cliff read with deeper height and a better silhouette."

Less useful:

- "Call random material tools until something happens."
