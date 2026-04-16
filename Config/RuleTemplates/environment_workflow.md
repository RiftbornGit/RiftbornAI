# Environment Building Rules

## Terrain First
Build landscapes before placing any actors. Sculpt terrain, paint material layers,
then let Landscape Grass Types handle ground cover automatically. Never place grass
manually — it won't track terrain changes.

## Foliage Instancing
Use AInstancedFoliageActor with UFoliageType for vegetation. Never spawn individual
actors per tree/bush — 500 individual actors means 500 draw calls. One instanced
foliage actor handles thousands with GPU instancing and automatic LOD.

## Real Assets Only
Use production-quality meshes from Megaplants, Quixel, Fab, or your asset library.
Never substitute primitive shapes (cones, planes, spheres) for vegetation or
natural objects. If an asset doesn't exist yet, flag it — don't fake it.

## Persistent Assets
Save all foliage types, materials, and landscape layers as persistent assets in
the Content directory. Never create them in GetTransientPackage() — transient
objects vanish on editor restart.

## Layer-Driven Materials
Landscape materials should use LandscapeLayerBlend with painted layers. Connect
grass types via LandscapeGrassOutput so vegetation responds to material layers
automatically.
