# 02 — UE5 Environment Systems

UE5 has purpose-built systems for every layer of an environment. Using the right system for each task is not optional — it's the difference between a game that runs at 60fps with millions of grass blades and one that crashes with 500 actors.

## System Overview

| Nature Layer | UE5 System | NOT This |
|---|---|---|
| Terrain shape | Landscape + Sculpting | Static mesh floor |
| Terrain surface | Landscape Material (auto-blend) | Single flat color |
| Grass & ground cover | Landscape Grass Type (automatic) | Manually placed actors |
| Bushes & flowers | Foliage Mode (painted instances) | Individual StaticMeshActors |
| Trees | Foliage Mode + PVE/Megaplants | SkeletalMeshActor per tree |
| Rocks & debris | PCG (Procedural Content Generation) | Hand-placed one by one |
| Water | Water Body actors | Blue plane mesh |
| Sky & atmosphere | SkyAtmosphere + VolumetricCloud | Skybox texture |
| Fog | ExponentialHeightFog | Post-process only |

## Landscape Material (Auto-Blend)

The landscape material is the BRAIN of your terrain. It decides what the ground looks like everywhere AND what vegetation spawns where.

### Architecture
```
Master Landscape Material
  |
  ├── LandscapeLayerBlend node (blends texture layers)
  |     ├── Layer: Grass    → Albedo, Normal, Roughness textures
  |     ├── Layer: Rock     → Albedo, Normal, Roughness textures
  |     ├── Layer: Dirt     → Albedo, Normal, Roughness textures
  |     └── Layer: Snow     → Albedo, Normal, Roughness textures
  |
  ├── Slope Detection (automatic)
  |     WorldAlignedNormal.Z → lerp between grass and rock
  |     Steep = rock, Flat = grass (no manual painting needed)
  |
  ├── Height Detection (automatic)
  |     WorldPosition.Z → lerp to snow above treeline
  |
  └── LandscapeGrassOutput node
        ├── Grass layer weight → GrassType_Meadow (auto-spawns grass)
        ├── Dirt layer weight  → GrassType_Debris (auto-spawns leaves/twigs)
        └── Rock layer weight  → (no grass on rock)
```

### Key Nodes
- **LandscapeLayerBlend** — Blends between texture layers, supports height blending at boundaries
- **LandscapeLayerSample** — Outputs the weight (0-1) of a layer at each point, used to drive grass spawning
- **LandscapeGrassOutput** — Takes layer weights as input, spawns grass meshes automatically
- **WorldAlignedNormal** — Gets the surface normal in world space for slope detection
- **AbsoluteWorldPosition** — Gets world Z height for altitude-based blending

### Material Layer Info
Each layer needs a **Landscape Layer Info** asset:
- Created by right-clicking the layer name in the Landscape paint mode
- Choose "Weight-Blended Layer (normal)" for most layers
- Choose "Non Weight-Blended Layer" for special layers like puddles

## Landscape Grass Type (Automatic Grass)

This is how grass works in professional games. You do NOT manually place grass.

### How It Works
1. Create a **Landscape Grass Type** asset (Content Browser → Foliage → Landscape Grass Type)
2. Add one or more **Grass Varieties** to it
3. Each variety specifies:
   - **Grass Mesh** — A static mesh (proper grass card, NOT a flat plane)
   - **Grass Density** — Instances per 1000x1000 unit area
   - **Start/End Cull Distance** — Where grass fades in/out
   - **Scale X/Y/Z** — Min and max scale range
   - **Random Rotation** — Usually enabled
   - **Align to Surface** — Usually enabled for grass
   - **Use Landscape Lightmap** — For baked lighting
4. In your landscape material, add a **LandscapeGrassOutput** node
5. Connect **LandscapeLayerSample** nodes to each grass input
6. The engine AUTOMATICALLY spawns grass wherever that landscape layer is painted

### Grass Mesh Requirements
A proper grass mesh is:
- **3 crossed vertical planes** at 0, 60, 120 degrees (star pattern)
- About 30-50cm wide, 20-80cm tall (scaled by the FoliageType)
- UVs mapped to the grass texture atlas
- Material: **Two-Sided Foliage** shading model, Masked blend mode
- Uses **SimpleGrassWind** material function for wind animation
- Has proper LOD (simplified at distance)

The mesh is NOT:
- A flat horizontal plane
- A single plane (visible from only one direction)
- An engine primitive (Cube, Sphere, Cone)
- A high-poly sculpted model

### Performance
- Landscape Grass Type uses **HISM** (Hierarchical Instanced Static Mesh) — GPU instancing
- Can handle millions of instances across the landscape
- Culling is automatic (configured per grass type)
- LOD transitions happen automatically
- This is orders of magnitude faster than individual actors

## Foliage Mode (Painted Instances)

For vegetation that needs artistic control (trees, bushes, flowers), use the editor Foliage Mode.

### How It Works
1. Open Foliage Mode: **Shift+3** or click the leaf icon in the Modes panel
2. Add **FoliageType** assets to the palette:
   - Created via Content Browser → Foliage → Static Mesh Foliage
   - Set the mesh, scale range, density, cull distance, collision settings
3. **Paint** by clicking and dragging on the landscape
4. Configure brush: size, density, erase mode
5. All instances are managed by **AInstancedFoliageActor** — one actor, thousands of instances

### FoliageType Settings That Matter
- **Density** — How many per area when painting
- **Scale X/Y/Z** — Random scale range (use 0.8-1.2 for natural variation)
- **Random Yaw** — ALWAYS enabled for vegetation
- **Align to Normal** — Enable for grass/flowers, disable for large trees
- **Ground Slope Angle** — Max slope where this can be placed (40° for grass, 20° for trees)
- **Cull Distance** — Start and end fade distance
- **Cast Shadow** — Trees yes, grass usually no (performance)
- **Collision** — Trees yes (players walk around them), grass no

### Programmatic Foliage Placement
Via C++ (our `scatter_foliage` tool):
```cpp
AInstancedFoliageActor* IFA = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(World);
UFoliageType_InstancedStaticMesh* FoliageType = NewObject<UFoliageType_InstancedStaticMesh>(...);
FoliageType->SetStaticMesh(Mesh);
FFoliageInfo* Info = nullptr;
IFA->AddFoliageType(FoliageType, &Info);
FFoliageInstance Instance;
Instance.Location = SurfaceHitPoint;
Instance.Rotation = FRotator(0, RandomYaw, 0);
Instance.DrawScale3D = FVector3f(RandomScale);
Info->AddInstance(FoliageType, Instance);
```

## PCG (Procedural Content Generation)

For large-scale scattering of rocks, debris, props — anything that follows rules.

### Core Concepts
- **PCG Graph** — Visual graph that defines scattering logic
- **Surface Sampler** — Generates points on surfaces (landscape, meshes)
- **Density Filter** — Controls how many points survive based on noise, textures
- **Mesh Spawner** — Places static meshes at surviving points
- **Difference/Intersect** — Boolean operations between point sets

### Typical Rock Scattering Setup
```
Surface Sampler (landscape)
  → Filter by Slope (steep only)
  → Density Noise (perlin noise for natural distribution)
  → Transform Randomizer (random scale 0.5-3.0, random yaw)
  → Mesh Spawner (rock mesh variants)
```

## PVE (Procedural Vegetation Editor) — UE5.7

For creating high-quality tree assets inside UE.

### Key Points
- Graph-based tool for designing tree structure
- Outputs **Nanite Skeletal Assemblies** (high-quality LOD-free rendering)
- Works with **Megaplants** assets from Fab
- Trees have wind animation built in
- Enable `r.Nanite.AllowAssemblies=1` in project settings

### Available Megaplant Species (in this project)
- Tree_English_Oak, Tree_European_Beech, Tree_Black_Alder
- Tree_Hornbeam, Tree_Goat_Willow, Tree_Common_Hazel, Tree_European_Aspen
- Shrub_Greasewood
- Each has PVE preset + multiple skeletal mesh variants (A, B, C, D)

## Water Bodies

- **Water Body Lake** — Flat water surface in a depression
- **Water Body River** — Spline-based flowing water
- **Water Body Ocean** — Infinite water plane at a set height
- Each has: wave settings, material, shoreline blending, underwater effects

## Atmosphere Stack

Bottom to top:
1. **ExponentialHeightFog** — Ground-level haze, volumetric fog for god rays
2. **SkyAtmosphere** — Rayleigh/Mie scattering, realistic sky color
3. **VolumetricCloud** — 3D clouds with proper shadowing
4. **DirectionalLight** — The sun, drives all shadow and color temperature
5. **SkyLight** — Ambient fill from the sky (captures sky color)
6. **PostProcessVolume** — Color grading, bloom, exposure, vignette
