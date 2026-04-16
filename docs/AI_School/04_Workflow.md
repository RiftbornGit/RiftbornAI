# 04 — The Correct Environment Workflow

Build environments BOTTOM-UP. Each layer depends on the layers below it. Do NOT skip layers or build out of order.

## Phase 1: Terrain Foundation

### Step 1.1 — Create or Load Level
- Create a new level or open existing one
- Save immediately after creation

### Step 1.2 — Create Landscape
- Use `create_landscape` with appropriate size (2017 or 4033 for large worlds)
- Set scale appropriate for your world (100x100x200 for mountainous terrain)
- Use `diamond_square` algorithm for natural-looking initial heightmap

### Step 1.3 — Sculpt Major Features

- Raise: mountain ranges, hills, ridges
- Lower: valleys, river beds, lake basins
- Smooth: transition areas, gentle slopes
- Flatten: areas for buildings, paths, clearings
- Work LARGE to SMALL: mountain first, then hills, then details

### Step 1.4 — Apply Erosion (optional but recommended)

- Use `hydraulic_erosion` to make terrain look geologically real
- Typical: 50000 iterations, erosion_rate=0.3, deposition_rate=0.3
- For non-destructive workflow: use `apply_terrain_stack` to chain noise → erosion → smooth → terrace as re-applyable operations

### Step 1.4 — Verify Terrain
- Move camera to multiple viewpoints
- Check: does the terrain have interesting silhouettes from all angles?
- Check: are there flat areas for gameplay, steep areas for drama?
- Check: is there a natural flow (valleys leading somewhere, ridges creating boundaries)?

## Phase 2: Landscape Material

### Step 2.1 — Gather Textures
For each layer you need Albedo + Normal + Roughness textures:
- **Grass layer** — Green grass tileable texture (Quixel Megascans or AI-generated)
- **Rock layer** — Rocky cliff face texture
- **Dirt layer** — Bare earth/soil texture
- **Snow layer** (optional) — White snow texture
- Import all textures to `/Game/Environment/Textures/`

### Step 2.2 — Create Master Landscape Material
- Create material asset: `M_Landscape_Master`
- Add `LandscapeLayerBlend` node with layers: Grass, Rock, Dirt
- For each layer: connect Albedo → BaseColor, Normal → Normal, Roughness → Roughness
- Add `LandscapeLayerCoords` for tiling control

### Step 2.3 — Add Slope Auto-Blend
- Add `WorldAlignedNormal` node → extract Z component
- Use `Lerp` to blend between Grass (flat) and Rock (steep)
- Threshold: ~0.7 for starting rock blend, ~0.3 for fully rock
- This makes rock appear automatically on steep slopes

### Step 2.4 — Add Height Auto-Blend (optional)
- Add `AbsoluteWorldPosition` → extract Z
- Above treeline height: blend to snow/alpine texture
- Below water level: blend to sand/gravel texture

### Step 2.5 — Create Landscape Layer Info Assets
- For each layer: right-click in paint mode → Create Layer Info
- Choose "Weight-Blended" for standard layers
- Assign to the landscape

### Step 2.6 — Apply Material to Landscape
- Select landscape → Details → set Landscape Material to `M_Landscape_Master`
- Paint layer weights if needed (or rely on auto-blend)

### Step 2.7 — Verify Material
- Screenshot from multiple angles
- Check: does rock appear on steep slopes automatically?
- Check: do layer transitions look natural (not sharp lines)?
- Check: are textures tiling at an appropriate scale?

## Phase 3: Automatic Grass

### Step 3.1 — Obtain Grass Mesh
- Download a proper grass card static mesh from Fab or Quixel
- OR create one in Blender: 3 crossed vertical planes at 0/60/120 degrees with UVs
- Must be a proper game-ready mesh, NOT a flat plane
- Apply grass texture + opacity mask + normal map as material (Two-Sided Foliage shading)

### Step 3.2 — Create Landscape Grass Type Asset
- Content Browser → Add New → Foliage → Landscape Grass Type
- Add Grass Varieties:
  - Variety 1: Main grass — mesh, density 400, scale 0.6-1.2, cull 5000
  - Variety 2: Tall grass — same mesh larger scale 1.0-1.8, lower density 50, cull 4000
  - Variety 3: Flowers (if available) — flower mesh, density 20, scale 0.4-0.8
- Enable: Random Rotation, Align to Surface
- Disable: Cast Shadow (for grass performance), Collision

### Step 3.3 — Connect to Landscape Material
- Open `M_Landscape_Master`
- Add `LandscapeGrassOutput` node
- Add `LandscapeLayerSample` nodes (one per layer)
- Connect Grass layer sample → Grass Output grass input
- Grass will ONLY spawn where grass texture layer is painted/auto-blended

### Step 3.4 — Verify Grass
- Grass should appear automatically on all grassy areas
- Check: is grass absent from rock faces and cliff edges?
- Check: is density appropriate (not too sparse, not too thick)?
- Check: does grass fade out at appropriate distance?

## Phase 3B: Roads, Paths & River Beds

Roads and paths should be drawn AFTER grass is set up (Phase 3) so that painting a road layer immediately causes grass to disappear along the path. This is the correct point in the build order — terrain and materials exist, grass is wired, and painting a road layer automatically normalizes the grass weight down to 0 at the road samples.

### Step 3B.1 — Draw Paths

- Use `draw_landscape_path` for each road, trail, or river bed
- Set `layer_name` to the road layer (e.g., "Dirt" for dirt roads, "Rock" for stone)
- Set `height_delta` negative for sunken road beds (-20cm for roads, -120cm for rivers)
- Set `width`, `strength`, `falloff_power` appropriate for the path type
- Set `refresh_grass=true` so grass instances are flushed immediately
- The landscape material's weight-blend normalization handles the grass/road transition automatically

### Step 3B.2 — Verify Paths

- Screenshot from bird's-eye and player-eye heights
- Check: does grass disappear cleanly along the road?
- Check: is the height depression visible and natural-looking?
- Check: do road edges blend softly into surrounding terrain (not hard-cut)?
- Check: do multiple paths crossing look acceptable at intersections?

## Phase 4: Ground Cover (Foliage Mode)

### Step 4.1 — Download Ground Cover Assets
- Ferns, wildflowers, fallen leaves, small rocks from Fab
- Create FoliageType assets for each

### Step 4.2 — Paint in Foliage Mode
- Open Foliage Mode (Shift+3)
- Add FoliageTypes to palette
- Paint with appropriate density
- Follow nature rules: ferns in shaded areas, flowers in clearings, leaves under trees

## Phase 5: Trees

### Step 5.1 — Use Megaplants or Download Tree Assets
- This project has: Oak, Beech, Alder, Hornbeam, Willow, Hazel, Aspen
- Create FoliageType assets pointing to tree skeletal meshes or PVE presets

### Step 5.2 — Place Trees
- Foliage Mode painting for artistic control
- OR PCG Graph for procedural distribution with ecosystem rules
- Follow nature: denser in valleys, sparser on ridges, clusters not uniform grid

## Phase 6: Rocks & Props (PCG)

### Step 6.1 — Create PCG Graph
- Surface Sampler on landscape
- Filter by slope (rocks on steep areas)
- Density noise for natural distribution
- Transform randomizer for variation
- Mesh spawner with rock mesh variants

## Phase 6B: Architecture (if building a settlement/city)

### Step 6B.1 — Generate Buildings

- Use `create_building_from_floor_plan` for individual buildings with custom outlines
- Use `create_city_block` for bulk city generation with road grids + buildings
- Use `create_staircase` for staircases connecting elevation changes
- Use `create_spline_architecture` for fences, walls, railings along paths

### Step 6B.2 — Add Destruction (optional)

- Use `make_destructible` on buildings/props that should be breakable
- Use `physics_settle` to scatter rubble/debris naturally on terrain

## Phase 7: Water

### Step 7.1 — Add Water Bodies

- Use `create_procedural_river` for rivers (sculpts channel + paints layer + spawns water body in one call)
- Water Body Lake in terrain depressions via `create_water_body`
- Adjust shoreline blending and wave settings

## Phase 8: Atmosphere

### Step 8.1 — Lighting
- DirectionalLight (sun): intensity ~10 lux, warm color 4800-5500K, cast shadows
- SkyLight: captures sky color for ambient fill
- Set sun angle for desired time of day (golden hour: pitch -15 to -25)

### Step 8.2 — Sky & Fog
- SkyAtmosphere actor for realistic sky
- VolumetricCloud actor for 3D clouds
- ExponentialHeightFog: fog density 0.01-0.03, enable volumetric fog

### Step 8.3 — Post-Process
- PostProcessVolume (infinite unbound)
- Slight bloom (0.3-0.5)
- Auto-exposure or manual exposure
- Color grading: slight desaturation for realism, warm shadows, cool highlights

## Phase 8B: Acoustics & Weather (if applicable)

### Step 8B.1 — Auto-Generate Reverb

- Use `analyze_acoustics` at the center of each enclosed space (buildings, caves, tunnels)
- The tool auto-creates UReverbEffect + AAudioVolume from room geometry
- Check: cathedral-sized rooms get long RT60, closets get short dry reverb

### Step 8B.2 — Set Weather (if dynamic weather desired)

- Use `set_weather` with a preset (clear, light_rain, heavy_rain, snow, fog)
- Creates a `MPC_RiftbornWeather` MaterialParameterCollection that all materials can read
- Materials must reference the collection's `Wetness` parameter to respond
- Places puddle decals on flat terrain when wetness > 0.3

### Step 8B.3 — Apply Weathering (if aged look desired)

- Use `apply_weathering` with age_years to add moss/dirt to the landscape
- Moss grows on north-facing slopes, dirt accumulates in depressions
- Scale: age_years=50 for light weathering, 500 for heavy ruins

## Phase 9: Verify Everything

### Step 9.1 — AI Scene Critique

- Use `critique_scene` for an AI art director evaluation
- Scores composition, lighting, color, density, atmosphere, detail 1-10
- Set auto_fix=true to have the AI suggest specific tool calls for improvements

### Step 9.2 — Record & Replay

- Use `record_replay` to play through the level and record the camera path
- Replay keyframes with `set_viewport_location` to revisit each moment
- Use `analyze_scene_screenshot` at boring keyframes to identify content gaps

### Step 9.3 — Standard Verification

- Fly through the entire scene at player height
- Check transitions between biomes/layers
- Check performance (FPS, draw calls)
- Take screenshots from multiple angles and times of day
- Use `observe_ue_project` for objective scene census + screenshot verification
- Use `analyze_scene_screenshot` for lighting quality check

## Phase 10: Optimization & Polish

### Step 10.1 — Mesh Optimization

- Use `generate_collision_mesh` on imported meshes that use complex-as-simple collision
- Use `auto_lod_chain` on high-poly meshes that lack LODs (marketplace assets often ship without)
- Use `forge_texture_from_material` to bake expensive procedural materials to static textures

### Step 10.2 — Volumetric Clouds

- Use `sculpt_volumetric_clouds` to match the scene mood (storm, sunset, fair_weather)
- Override individual parameters (altitude, height, ground albedo) to fine-tune

### Step 10.3 — Detail Passes

- Use `string_cables_between_points` for power lines, vines, ropes, decorative wires
- Use `populate_interior` for enclosed spaces — creates placement markers for furniture
- Replace markers with real meshes via project assets or a separate asset-ingest workflow

### Step 10.4 — World Partition (Open Worlds)

- Use `optimize_world_partition` to analyze streaming — identifies hotspot cells and oversized actors
- Follow recommendations for grid sizing and HLOD layer configuration

## Phase 11: Blueprint & Cinematic Automation

### Step 11.1 — Blueprint Logic

- Use `wire_blueprint_from_description` to create gameplay logic in Blueprint event graphs
- Wire events (BeginPlay, ActorBeginOverlap) to function calls (PrintString, PlaySound, etc.)
- Supports branches, variables, and custom events

### Step 11.2 — Cinematic Sequences

- Use `compose_cinematic_sequence` to create camera animations and cutscenes
- Define shots with camera positions, keyframes, and timing
- Camera cuts are auto-generated from shot boundaries

### Step 11.3 — PCG Graphs

- Use `compose_pcg_graph` to create procedural generation graphs
- Chain: surface_sampler → transform_points → density_filter → mesh_spawner
- Attach resulting graph to a PCG Volume in the level to generate content
