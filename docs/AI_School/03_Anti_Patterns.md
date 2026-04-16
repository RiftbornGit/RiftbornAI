# 03 — Anti-Patterns (What NOT To Do)

Every mistake listed here was actually made by an AI agent on this project. These are not hypothetical — they caused crashes, produced ugly results, or wasted hours of work.

## NEVER: Use Primitive Shapes as Vegetation

**What happened:** Agent spawned Cone meshes as "trees" and Plane meshes as "grass."

**Why it's wrong:**
- A cone is not a tree. No one has ever looked at a cone and thought "forest."
- A flat plane is invisible from the side — it has zero thickness
- Primitive shapes have no material UVs designed for vegetation textures
- This is placeholder art that teaches you nothing about the final result

**Do instead:** Use real assets from Megaplants, Quixel, or Fab. If no assets exist yet, don't fake it with primitives — say "I need a grass mesh asset" and solve THAT problem.

## NEVER: Spawn Individual Actors for Foliage

**What happened:** Agent spawned 500 SkeletalMeshActor instances, one per tree.

**Why it's wrong:**
- 500 individual actors = 500 draw calls, 500 tick updates, 500 collision checks
- At 5000 trees this WILL tank framerate to single digits
- UE5's instanced foliage system exists specifically to solve this problem
- Individual actors don't benefit from GPU instancing, LOD, or culling optimizations

**Do instead:** Use `AInstancedFoliageActor` with `UFoliageType`. ONE actor manages ALL instances. GPU instancing renders thousands of trees in a single draw call.

## NEVER: Manually Place Grass

**What happened:** Agent called `scatter_foliage` to manually distribute 1000 grass instances at random positions.

**Why it's wrong:**
- Manual grass placement doesn't respond to landscape material layers
- Grass ends up on rock, in water, on cliff faces — places it shouldn't be
- Manual placement can't dynamically update when you edit the terrain
- Landscape Grass Type does this AUTOMATICALLY and CORRECTLY

**Do instead:** Create a `LandscapeGrassType` asset, connect it to the landscape material via `LandscapeGrassOutput` node. Grass spawns automatically wherever the grass texture layer exists.

## NEVER: Bypass UE Systems

**What happened:** Agent tried to import textures via Python's `AssetImportTask` from a worker thread, causing a TaskGraph recursion crash.

**Why it's wrong:**
- UE5's asset pipeline (Interchange) has thread safety requirements
- Calling editor APIs from non-game-thread contexts causes undefined behavior
- The asset import system is designed for the main thread / editor tick

**Do instead:** Copy texture files to the Content directory and let UE auto-detect and import them. Or use game-thread-safe APIs.

## NEVER: Create Transient Assets for Persistent Content

**What happened:** Agent created `UFoliageType_InstancedStaticMesh` in `GetTransientPackage()`, which means it vanishes when you restart the editor.

**Why it's wrong:**
- Transient objects don't save to disk
- Your foliage types disappear on editor restart
- The Foliage Mode palette can't reference transient assets

**Do instead:** Create proper assets saved to the Content directory using `AssetToolsHelpers::GetAssetTools()->CreateAsset()`.

## NEVER: Use execute_python for Landscape Deletion

**What happened:** Agent used `execute_python` to call `destroy_actor` on landscape actors, causing a GUID crash on level reload.

**Why it's wrong:**
- Landscape actors have internal GUID references that persist in the level package
- Destroying them via script leaves dangling GUID references
- Next level load crashes with GUID assertion failure

**Do instead:** Delete landscape actors through the editor UI, or create a new level entirely.

## NEVER: Skip the Material System

**What happened:** Agent applied a single flat green color to a landscape instead of creating a proper multi-layer landscape material.

**Why it's wrong:**
- Real terrain is never one color — it varies by slope, height, wetness, wear
- Without a proper landscape material, Landscape Grass Type can't work
- Without layer blending, there's no way to paint paths, rocky areas, or dirt patches

**Do instead:** Create a landscape material with at minimum 3 layers: grass (flat areas), rock (steep slopes), dirt (transitions/paths). Use slope detection for automatic blending. For roads and trails, use `draw_landscape_path` — do NOT manually call `paint_landscape_layer` in a loop to approximate a road shape.

## NEVER: Paint a Road by Calling paint_landscape_layer in a Loop

**What happened:** Agent tried to simulate a road by calling `paint_landscape_layer` 20+ times with overlapping circles along a line.

**Why it's wrong:**

- Each call is a separate weight-map read/write cycle — 20x slower than one `draw_landscape_path` call
- Overlapping circles create visible scalloped edges instead of a smooth path
- No height sculpting (road bed), no edge falloff, no Catmull-Rom smoothing

**Do instead:** Use `draw_landscape_path` with a polyline. One call handles the entire raster with proper distance-based falloff, optional height delta, and grass refresh.

## NEVER: Ignore the Layer Stack

**What happened:** Agent tried to add trees before the terrain had a material, grass, or atmosphere.

**Why it's wrong:**
- Trees planted on a grey checkerboard look terrible and give no visual feedback
- Without grass and ground cover, the base of every tree is bare and ugly
- Without atmosphere, everything looks flat and lifeless
- You can't judge composition, density, or coverage without the full stack

**Do instead:** Build BOTTOM-UP: terrain shape → terrain material → automatic grass → ground cover → trees → atmosphere. Each layer depends on the ones below it.

## NEVER: Build Buildings by Stacking spawn_actor Cubes

**What happened:** Agent tried to create a building by spawning 4 wall cubes + 1 floor cube + 1 roof cube as separate static mesh actors.

**Why it's wrong:**

- 6 actors for one building = 6 draw calls, 6 collision bodies, 6 entries in the outliner
- No windows, no doors, no proper wall thickness — just flat faces
- Impossible to UV-map or apply materials coherently across separate actors
- Doesn't scale: 20 buildings = 120 actors of hollow visual quality

**Do instead:** Use `create_building_from_floor_plan` — one call, one mesh, proper walls with thickness, boolean-cut windows and doors, floor and ceiling slabs, saved as a Nanite UStaticMesh. For bulk city generation, use `create_city_block`.

## NEVER: Scatter Objects Without Physics Settling

**What happened:** Agent scattered 200 rocks above a hillside at random positions, all floating in mid-air.

**Why it's wrong:**

- Rocks placed without ground-snapping float or intersect terrain
- Random Y-axis rotations look artificial on slopes — real rocks settle and lean
- Manual position correction for 200 objects is impractical

**Do instead:** Spawn objects slightly above the terrain, then call `physics_settle` with 2-3 seconds of simulation. Gravity and collision naturally settle each object into a physically-plausible rest position.

## NEVER: Generate What You Should Download

**What happened:** Agent tried to procedurally generate tree meshes in C++ code.

**Why it's wrong:**
- Procedural trees look procedural — they lack the organic detail of scanned/sculpted assets
- Professional tree assets represent months of artist work per species
- Megascans, Megaplants, and Fab have thousands of free production-quality assets
- The time spent writing procedural generation code is always better spent importing real assets

**Do instead:** Download real assets from Fab, Quixel, or Megascans. AI can generate TEXTURES (grass cards, leaf atlases) but not complex 3D vegetation models.

## The Golden Rule

**If UE5 has a built-in system for it, use that system.** Don't write custom code to replace what Epic spent years building. Your job is to DRIVE these systems with the right data, not to replace them.
