# 05 — RiftbornAI Tool Guide for Environment Work

Which tools to use for each task. If a tool doesn't exist yet, note it and build it before proceeding.

## IMPORTANT: Generic-First Approach

Before searching for a specialized tool, check if a **generic tool** already covers your need:

| Need | Generic Tool | Example |
|---|---|---|
| Set any property | `set_component_property_typed` | `(actor="Sun", component="LightComponent", property="Intensity", value="5.0")` |
| Inspect anything | `inspect_actor` | Returns full JSON of all properties, components, transforms |
| Spawn anything | `spawn_actor` | Takes any class name or Blueprint path |
| Custom operation | `execute_python` | `import unreal; unreal.EditorLevelLibrary.get_all_level_actors()` |

**Only use specialized tools when they provide real computation** (erosion, ecosystem growth, material graph wiring, landscape sculpting). A tool that just calls one UE API function is redundant — use the generic instead.

## Terrain

| Task | Tool | Notes |
|---|---|---|
| Create landscape | `create_landscape` | size=2017 or 4033, diamond_square algorithm |
| Sculpt terrain | `sculpt_landscape` | modes: raise, lower, smooth, flatten |
| Paint layers | `paint_landscape_layer` | Requires landscape material with layer blend |
| Draw road/path | `draw_landscape_path` | Polyline → weight-map raster + optional height sculpt. Grass auto-disappears if material is wired. |
| Add paint layer | `add_landscape_layer` | Register a new layer before painting it |
| Verify terrain | `observe_ue_project` | Check actor census, viewport framing, and scene state |

## Materials

| Task | Tool | Notes |
|---|---|---|
| Create material | `create_material` | Creates base material asset |
| Add texture samplers | `add_material_expression` | type=TextureSample, set texture_path |
| Connect nodes | `connect_material_nodes` | source_node_id, source_output, material_input |
| Set reflected material property | `set_object_property_typed` | Use only after inspecting the material's reflected property names. There is no dedicated shading-model helper on the current public surface. |
| Set material on actor | `set_actor_material` | actor_label + material_path |
| Generate AI textures | External workflow or project assets | Not part of the shipped editor/MCP surface |

## Roads, Paths & Rivers

| Task | Tool | Notes |
|---|---|---|
| Draw a dirt road | `draw_landscape_path` | layer_name=Dirt, width=450, height_delta=-20, falloff_power=2 |
| Draw a river bed | `draw_landscape_path` | layer_name=Dirt, width=900, height_delta=-120, falloff_power=1.5 |
| Draw a grass trail | `draw_landscape_path` | layer_name=Dirt, width=180, strength=0.5, height_delta=-5 |
| Draw a paved road | `draw_landscape_path` | layer_name=Rock, width=700, height_delta=5, falloff_power=4 |
| Paint layers radially | `paint_landscape_layer` | For clearings, areas around buildings, etc. |

**Important:** For grass to disappear along the path, the landscape material must have a `LandscapeGrassOutput` node wired to the Grass layer's weight. Use `attach_landscape_grass_to_material` (or `enable_landscape_auto_grass`) to set this up. The `draw_landscape_path` tool with `refresh_grass=true` then handles the rest.

**Correct workflow order:** `create_landscape` → `create_landscape_material` (with layers) → `set_landscape_material` → `add_landscape_layer` (register each layer on the landscape) → `paint_landscape_layer` (base coverage) → `create_landscape_grass_type` + `add_grass_variety` → `attach_landscape_grass_to_material` → `draw_landscape_path` (roads/trails).

## Grass (Automatic)

| Task | Tool | Status |
|---|---|---|
| Create Landscape Grass Type | `create_landscape_grass_type` | Native asset creation with density, scale, and cull controls |
| Add more grass varieties | `add_grass_variety` | Append extra meshes for ferns, weeds, and litter |
| Attach grass output to material | `enable_landscape_auto_grass` | Creates or updates `LandscapeGrassOutput` wiring |
| Connect custom material nodes | `add_material_expression` + `connect_material_nodes` | Use when you need hand-authored graph control |
| Create grass card mesh | `create_grass_card_mesh` | Builds a crossed-plane static mesh asset for auto-grass and foliage |

## Foliage (Painted/Scattered)

| Task | Tool | Notes |
|---|---|---|
| Paint foliage in an area | `paint_foliage` | Programmatic foliage paint mode using `AInstancedFoliageActor::FoliageTrace` for logical support placement on trees, shrubs, rocks, and biome props. Do not use this for automatic grass. |
| Repair floating foliage | `ground_foliage_to_landscape` | Reattach existing foliage to landscape or nearest valid support surface; slight burial is allowed via `surface_offset` |
| Scatter foliage on landscape | `scatter_foliage` | Wide-area instanced scattering with surface-aware placement |
| Inspect reusable foliage assets | `list_assets` | Check foliage-type assets under project content before painting or simulation |
| Clear foliage | `remove_foliage_instance` | Remove placed foliage instances from the current world |
| List available meshes | `list_assets` | Check /Game/Megaplant_Library/ |

**Grounding rule:** use `paint_foliage` or `ground_foliage_to_landscape` for trees, rocks, bushes, and biome props that must sit on real support surfaces. Slight burial is fine. Floating is not. Use `create_landscape_grass_type` plus `add_grass_variety` for grass and tiny ground cover.

## Foliage Ecosystem (Growth / Death / Decay)

| Task | Tool | Notes |
|---|---|---|
| Create growth simulator | `create_procedural_foliage_spawner` | Builds a `UProceduralFoliageSpawner` from one or more foliage types |
| Spawn ecosystem volume | `spawn_procedural_foliage_volume` | Places a `AProceduralFoliageVolume`, assigns the spawner, and configures valid support surfaces |
| Grow / re-seed the ecosystem | `resimulate_procedural_foliage` | Re-runs the procedural simulation so species spread, compete, and repopulate |
| Inspect ecosystem state | `inspect_procedural_foliage` | Reports volume bounds, support rules, spawner config, and current or preview counts |
| Age living foliage | `set_foliage_instance_lifecycle` | Writes per-instance age/health/rot custom data and can rescale from age curves |
| Promote hero trees to runtime actors | `promote_foliage_to_dynamic_actors` | Converts foliage instances into real actors for falling, decay, and physics |
| Change tree state | `set_dynamic_foliage_actor_state` | State machine for `living`, `dead_standing`, `fallen`, `rotting` with optional physics impulse |

**Correct workflow order for dynamic forests:** `create_procedural_foliage_spawner` → `spawn_procedural_foliage_volume` → `resimulate_procedural_foliage` → `inspect_procedural_foliage` → `set_foliage_instance_lifecycle` → `promote_foliage_to_dynamic_actors` for trees that need to fall or rot visibly.

Reality:
- `create_procedural_foliage_spawner` is a real UE 5.7 authoring surface, but the engine only exposes `GetFoliageTypes()` publicly. RiftbornAI therefore writes foliage entries through reflected struct-property access, so keep it in `BETA` and verify results.

## Trees (Megaplants)

| Task | Tool | Notes |
|---|---|---|
| List available trees | `list_assets` | path=/Game/Megaplant_Library |
| Spawn PVE tree | `spawn_actor` | Use PVE blueprint path as actor_class |
| Place tree as foliage | `scatter_foliage` | Use SK_ mesh paths |

## Atmosphere

| Task | Tool | Notes |
|---|---|---|
| One-shot outdoor stack | `setup_outdoor_lighting` | Creates sun, sky, fog, clouds, skylight, and PPV together |
| Forest atmosphere presets | `apply_forest_atmosphere_preset` | Reusable canopy, mist, and shaft presets |
| Localized fog pockets | `set_fog_volume` | Native LocalFogVolume creation and retuning |
| Spawn sky atmosphere | `spawn_actor` | actor_class=SkyAtmosphere |
| Spawn volumetric clouds | `spawn_actor` | actor_class=VolumetricCloud |
| Spawn sky light | `spawn_actor` | actor_class=SkyLight |
| Spawn height fog | `spawn_actor` | actor_class=ExponentialHeightFog |
| Create directional light (sun) | `create_light` | light_type=Directional, intensity=10 |
| Configure sun | `set_component_property` | adjust DirectionalLight component intensity, temperature, or color |
| Spawn post-process volume | `spawn_actor` | actor_class=PostProcessVolume |

## Water

| Task | Tool | Notes |
|---|---|---|
| Create water body | `create_water_body` | type=Lake/River/Ocean; lakes/ponds use closed water splines, rivers use river spline metadata, oceans use extents |

## Architecture & Procedural Geometry

| Task | Tool | Notes |
|---|---|---|
| Build a house from a floor plan | `create_building_from_floor_plan` | JSON outline → walls, floor, ceiling, windows, doors → Nanite mesh |
| Generate a staircase | `create_staircase` | type=straight/spiral/curved, configurable step count/height/width |
| Build a fence/wall/railing | `create_spline_architecture` | Mesh repeated at intervals + spline-mesh fill between posts |
| Generate a city district | `create_city_block` | Road grid + buildings in one call, configurable grid size and heights |

## Surface & Materials

| Task | Tool | Notes |
|---|---|---|
| Add depth to a flat texture | `apply_surface_depth` | Luminance→WPO displacement, works best on brick/stone/tile |
| Create material from a photo | `create_material_from_photo` | Photo → PBR material with albedo + roughness |

## Destruction & Physics

| Task | Tool | Notes |
|---|---|---|
| Make an actor breakable | `make_destructible` | Static mesh → Chaos geometry collection with damage threshold |
| Settle physics objects | `physics_settle` | PIE sim → snap actors to rest positions (scatter rocks, furniture) |

## Terrain Simulation

| Task | Tool | Notes |
|---|---|---|
| Erode terrain naturally | `hydraulic_erosion` | 10000-100000 droplets, configurable erosion/deposition rates |
| Non-destructive terrain ops | `apply_terrain_stack` | JSON array of ops: noise, erosion, smooth, terrace |

## Procedural Geometry

| Task | Tool | Notes |
|---|---|---|
| Generate a cave/overhang | `create_cave` | SDF + Marching Cubes, types: entrance/arch/tunnel/overhang |
| Generate a tree mesh | `create_procedural_tree` | L-system branching, configurable species params |
| Organic architecture | `create_sdf_architecture` | Compose SDF volumes with smooth Ricci blending |

## Environment Intelligence

| Task | Tool | Notes |
|---|---|---|
| AI scene critique | `critique_scene` | Screenshot + AI rubric scoring, optional auto-fix suggestions |
| Auto-generate reverb | `analyze_acoustics` | Line-trace room → Sabine equation → UReverbEffect + AAudioVolume |
| Age surfaces | `apply_weathering` | Moss on north-facing slopes, dirt in depressions, scales with age_years |
| Record player path | `record_replay` | PIE camera recording as JSON keyframes for replay-based editing |
| Set weather state | `set_weather` | MaterialParameterCollection: wetness/rain/snow/fog + puddle decals |

## Natural Ecosystem

| Task | Tool | Notes |
|---|---|---|
| Place trees by ecology rules | `scatter_ecology_trees` | Poisson disk sampling + eco-zone classification (valley/slope/ridge/riparian/wetland). Reads heightmap for slope + detects water bodies. JSON zone→mesh mapping: `{"valley":"/Game/Trees/SM_Oak","slope":"/Game/Trees/SM_Pine","riparian":"/Game/Trees/SM_Willow"}` |
| Auto-generate ferns/shrubs under canopy | `generate_understory` | Ray casts upward to detect canopy occlusion. Shade plants under trees, sun plants in gaps. Separate mesh lists for shade vs sun. sample_spacing=200 controls grid density |
| Apply seasonal color shift | `apply_season` | Creates/updates MPC_Season with LeafColorShift, GrassDensityScale, SnowCoverage, BranchBareness. Presets: spring, summer, autumn, winter, or float 0.0-1.0 for continuous blend. Materials must reference MPC_Season to react |
| Grow vines on tree trunks | `grow_vines_on_surfaces` | Finds vertical surfaces via multi-angle traces, spawns UCableComponent. Presets: climbing_ivy (tight, many), hanging_vine (droopy), moss_curtain (thin, short), tropical_liana (thick, long) |
| Scatter leaf litter near trees | `scatter_ground_debris` | Reads existing foliage instances to find tree positions, scatters debris meshes near trunks. Drip-line clustering (denser at 70% of scatter_radius). Ground-snaps + slope rejection |
| Grass bends when player walks through | `setup_foliage_interaction` | Creates MPC_FoliageInteraction + wires material WPO. Up to 4 interactors. Params: interaction_radius=200, bend_strength=80, squish_amount=0.3. Sets WPO Disable Distance for VSM safety. Runtime: set MPC InteractorPos_0 = character pos each tick |
| Snow/grass trails persist after walking | `setup_deformation_trails` | Creates RT2D deformation map + stamp material + MPC. Material samples RT for downward WPO. trail_persistence=0.95 for slow fade. Runtime: draw stamp material to RT at character position each tick via DrawMaterialToRenderTarget |

**Correct ecosystem build order:**

`create_landscape` → `simulate_ecosystem` (paint layers) → `scatter_ecology_trees` (place trees) → `generate_understory` (ferns/shrubs) → `scatter_ground_debris` (leaf litter) → `grow_vines_on_surfaces` (vines) → `apply_season` (seasonal color) → `setup_foliage_interaction` (runtime bending) → `setup_deformation_trails` (snow/grass trails)

**Foliage interaction architecture:**

```text
setup_foliage_interaction (editor-time):
  Creates MPC_FoliageInteraction → InteractorPos_0..3, InteractionRadius, BendStrength
  Wires material WPO: distance(vertex, interactor) → radial push + height mask + squish

setup_deformation_trails (editor-time):
  Creates RT_DeformationTrail (R8, 1024x1024)
  Creates M_TrailStamp (radial gradient)
  Creates MPC_DeformationTrails → TrailCenter, CaptureSize
  Wires material WPO: sample RT at world-position UV → downward push

Runtime Blueprint (user creates):
  Event Tick → Set MPC InteractorPos_0 = GetActorLocation()
  Event Tick → DrawMaterialToRenderTarget(RT, M_TrailStamp, at player pos)
```

## Photorealism (Ecosystem Growth, Instance Variation, Lighting)

| Task | Tool | Notes |
|---|---|---|
| Grow a realistic forest from seed | `simulate_ecosystem_growth` | Computes D8 flow accumulation + TWI moisture + solar exposure from heightmap. Simulates seed dispersal, light competition, growth curves, age-based mortality. species_config JSON defines per-species mesh/tolerance/age/dispersal |
| Make each foliage instance look unique | `setup_instance_variation` | Fills per-instance custom data (hue shift, roughness, age, noise seed). Wires an instanced-static-mesh material to read PerInstanceCustomData → Lerp BaseColor toward aged brown, Add roughness variation, Noise micro-detail |
| Add leaf translucency (light through leaves) | `setup_advanced_lighting_realism` | Creates USubsurfaceProfile + assigns MSM_SubsurfaceProfile shading model. Green-gold glow when backlit |
| Add dappled forest shadows | `setup_advanced_lighting_realism` | Creates MD_LightFunction material (animated Voronoi noise) + assigns to sun directional light |
| Enable god rays from all lights | `setup_advanced_lighting_realism` | Calls SetVolumetricScatteringIntensity on every ULightComponent in scene |
| Add water caustic patterns | `setup_advanced_lighting_realism` | Creates MD_DeferredDecal caustic material + places ADecalActor near typed UE Water bodies |
| Analyze soil chemistry | `compute_soil_map` | Derives depth/fertility/pH from heightmap. Paints landscape layers: fertile→Grass, barren→Rock, wet→Dirt. Feeds into simulate_ecosystem_growth species suitability |

**Photorealism build order:**

`compute_soil_map` (analyze soil) → `simulate_ecosystem_growth` (grow forest using soil data) → `setup_instance_variation` (make each tree unique) → `setup_advanced_lighting_realism` (SSS + shadows + volumetric + caustics)

## Interactable World (Mining, Chopping, Digging)

| Task | Tool | Notes |
|---|---|---|
| Make rock/ore minable | `make_resource_node` | Static mesh → Chaos GeometryCollection + HP tags + debris removal. Stays STATIC until hit. resource_type=stone/ore/iron_ore/gold_ore/crystal. drop_items=stone_chunk,rare_gem |
| Make trees cuttable | `make_tree_harvestable` | Promotes foliage instances → Movable actors with HP + wood tags. After HP=0, call `set_dynamic_foliage_actor_state(state=fallen)` for physics fall |
| Dig hole in terrain | `dig_terrain` | Lowers landscape heightmap in bowl shape. depth=100, radius=200, falloff=2.0. Optional paint_layer=Dirt. Editor/PIE only |

**Interactable world architecture (event-driven, NOT continuous physics):**

```text
STATIC UNTIL HIT (zero physics cost)
    ↓ player hits with tool
DAMAGE EVENT → parse HP tag → subtract damage → update tag
    ↓ HP reaches 0
DESTRUCTION:
  Resource nodes → Chaos fracture activates → pieces fly → auto-despawn → drop loot
  Trees → set_dynamic_foliage_actor_state(fallen) → physics fall → harvestable log
  Terrain → dig_terrain lowers heightmap → optional dirt layer paint
```

## Verification

| Task | Tool | Notes |
|---|---|---|
| Full scene data | `observe_ue_project` | Viewport, actor census, screenshot, optional vision summary |
| Lighting analysis | `analyze_scene_screenshot` | Visual lighting quality assessment |
| Color analysis | `analyze_scene_screenshot` | Ask explicitly about palette, warmth, and harmony |
| Density check | `observe_ue_project` + `analyze_scene_screenshot` | Use actor census plus screenshot review |
| Screenshot | `take_screenshot` | For visual inspection |
| Multi-angle review | `set_viewport_location` + `capture_viewport_sync` | Move camera and review from multiple angles |
| Performance check | `get_performance_report` | FPS, draw calls, triangles |

## Current Status

These tools now exist in the plugin:

1. **create_landscape_material** — Native weight-painted landscape material creation.
2. **create_landscape_grass_type** — Native Landscape Grass Type asset creation.
3. **create_grass_card_mesh** — Native crossed-plane grass card static mesh creation.
4. **setup_outdoor_lighting** / **apply_forest_atmosphere_preset** — One-shot atmosphere setup equivalents.
5. **paint_foliage** — Programmatic area-paint equivalent for foliage placement.
6. **draw_landscape_path** — Polyline → weight-map + height raster with per-point overrides and endpoint taper.
7. **create_procedural_river** — Channel sculpt + riverbed paint + AWaterBodyRiver in one call.
8. **paint_biome** — Multi-layer landscape painting in one call.
9. **create_building_from_floor_plan** — Floor plan polyline → building mesh via GeometryCore booleans.
10. **create_staircase** — Parametric stair mesh via FStairGenerator.
11. **create_spline_architecture** — Fence/wall/railing via mesh repetition along a polyline.
12. **create_city_block** — Road grid + buildings orchestrated in one call.
13. **apply_surface_depth** — Albedo → WPO displacement material.
14. **create_material_from_photo** — Photo → PBR material.
15. **make_destructible** — Static mesh → Chaos geometry collection.
16. **physics_settle** — PIE sim → snap actors to rest positions.
17. **hydraulic_erosion** — Particle-based erosion simulation on landscape heightmap.
18. **apply_terrain_stack** — Non-destructive terrain operation stack (noise, erosion, smooth, terrace).
19. **create_cave** — SDF composition + Marching Cubes → cave/arch/tunnel as Nanite mesh.
20. **create_procedural_tree** — L-system branching → unique tree mesh as Nanite UStaticMesh.
21. **create_sdf_architecture** — SDF primitives with Ricci blend → organic Nanite architecture.
22. **critique_scene** — AI art director evaluates composition/lighting/color/density.
23. **analyze_acoustics** — Line-trace room → Sabine RT60 reverb → UReverbEffect + AAudioVolume.
24. **apply_weathering** — Age surfaces by N years: moss on slopes, dirt in depressions.
25. **record_replay** — PIE camera path → JSON keyframes for replay-based editing.
26. **set_weather** — Dynamic weather via UMaterialParameterCollection: rain, snow, fog, wind.
27. **wire_blueprint_gameplay** — JSON → real K2 nodes wired in Blueprint event graph.
28. **create_level_sequence** — Create the playable `ULevelSequence` container before binding tracks, cameras, and cuts.
29. **create_pcg_graph** — Create the `UPCGGraph` asset scaffold before authoring its nodes and execution context.
30. **generate_collision_mesh** — Auto-generate simple collision (box/convex/multi-convex) on static mesh.
31. **auto_lod_chain** — Auto-generate LOD1-3 via IMeshReduction with screen-size thresholds.
32. **forge_texture_from_material** — Render material to permanent UTexture2D via render targets.
33. **sculpt_volumetric_clouds** — Shape cloud formations via UVolumetricCloudComponent presets.
34. **string_cables_between_points** — Physics-simulated UCableComponent between positions.
35. **populate_interior** — Line-trace room geometry → contextual furniture/fixture placement.
36. **optimize_world_partition** — Analyze actor distribution, recommend streaming grid sizes.
37. **scatter_ecology_trees** — Poisson disk + eco-zone classification for ecology-aware tree placement.
38. **generate_understory** — Canopy occlusion ray cast → shade/sun plant auto-placement.
39. **apply_season** — MPC_Season: seasonal color/density/snow/bareness via MaterialParameterCollection.
40. **grow_vines_on_surfaces** — UCableComponent vines climbing vertical surfaces (4 presets).
41. **scatter_ground_debris** — Leaf litter/twigs near tree bases using foliage instance proximity.
42. **setup_foliage_interaction** — MPC + WPO proximity bending: grass bends when characters walk through.
43. **setup_deformation_trails** — RT2D persistent trail system for snow paths, grass trails, tire tracks.
44. **make_resource_node** — Static mesh → Chaos destructible with HP, resource tags, debris config.
45. **make_tree_harvestable** — Foliage → cuttable dynamic actors with HP, wood drops, fall physics.
46. **dig_terrain** — Lower landscape heightmap at a point for excavation holes (editor/PIE).
47. **simulate_ecosystem_growth** — Multi-century growth sim with D8 flow, TWI, competition, mortality.
48. **setup_instance_variation** — Per-instance custom data + PerInstanceCustomData material wiring.
49. **setup_advanced_lighting_realism** — SSS profiles, light functions, per-light volumetric, caustic decals.
50. **compute_soil_map** — Soil depth/fertility/pH from heightmap + landscape layer painting.

## Blueprint Automation

| Task | Tool | Notes |
|---|---|---|
| Wire event graph | `wire_blueprint_gameplay` | JSON nodes/connections → real K2 nodes. Supports: event, call, variable_get/set, branch, custom_event |
| Compile blueprint | Handled automatically | Tool compiles after wiring |

## Cinematic Sequences

| Task | Tool | Notes |
|---|---|---|
| Create cinematic | `create_level_sequence` | Creates the `ULevelSequence` asset that the rest of the cinematic lane binds and edits |
| Camera motion | Define keys in shots[] | Each key: frame, location [x,y,z], rotation [pitch,yaw,roll] |
| Actor animation | Define in bindings[] | Bind level actors to sequence with transform tracks |

## PCG Graphs

| Task | Tool | Notes |
|---|---|---|
| Build PCG graph | `create_pcg_graph` | Creates the `UPCGGraph` asset scaffold. Attach it to a PCG Volume after node authoring. |
| Scatter on terrain | surface_sampler node | points_per_sqm, looseness, point_steepness |
| Spawn meshes | mesh_spawner node | Assign mesh in PCG editor after creation |
| Randomize transform | transform_points node | offset, rotation, scale min/max ranges |
| Filter by density | density_filter node | lower_bound, upper_bound, invert |

## Mesh Optimization

| Task | Tool | Notes |
|---|---|---|
| Simple collision | `generate_collision_mesh` | box, fitted, convex, multi_convex quality presets |
| LOD generation | `auto_lod_chain` | Reduction factor per LOD level, auto screen sizes |
| Bake material | `forge_texture_from_material` | Procedural material → permanent texture on disk |

## World Enhancement

| Task | Tool | Notes |
|---|---|---|
| Shape clouds | `sculpt_volumetric_clouds` | 7 presets or custom parameters |
| String cables | `string_cables_between_points` | Power lines, vines, ropes between points |
| Furnish room | `populate_interior` | Line-trace → spatial analysis → prop markers |
| Streaming analysis | `optimize_world_partition` | Actor density, hotspots, grid recommendations |

## Asset Paths Reference

### Megaplant Trees (in this project)
```
/Game/Megaplant_Library/Tree_English_Oak/Tree_English_Oak_01/PVE_English_Oak_01
/Game/Megaplant_Library/Tree_European_Beech/Tree_European_Beech_01/PVE_European_Beech_01
/Game/Megaplant_Library/Tree_Black_Alder/Tree_Black_Alder_01/PVE_Black_Alder_01
/Game/Megaplant_Library/Tree_Hornbeam/Tree_Hornbeam_01/PVE_Hornbeam_01
/Game/Megaplant_Library/Tree_Goat_Willow/Tree_Goat_Willow_01/PVE_Goat_Willow_01
/Game/Megaplant_Library/Tree_Common_Hazel/Tree_Common_Hazel_01/PVE_Common_Hazel_01
/Game/Megaplant_Library/Tree_European_Aspen/Tree_European_Aspen_01/PVE_European_Aspen_01
/Game/Megaplant_Library/Shrub_Greasewood/Shrub_Greasewood_01/PVE_Greasewood_01
```

### Engine Meshes
```
/Engine/BasicShapes/Plane    — DO NOT use for foliage (lies flat)
/Engine/BasicShapes/Cube     — DO NOT use for vegetation
```

### AI-Generated Textures (in this project)
```
/Game/Foliage/Generated/AI_Grass/AI_Grass_Color
/Game/Foliage/Generated/AI_Grass/AI_Grass_Normal
/Game/Foliage/Generated/AI_Grass/AI_Grass_Opacity
```
