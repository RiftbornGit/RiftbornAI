// Copyright RiftbornAI. All Rights Reserved.
// Geometry Script Tools Module — Procedural mesh generation, dynamic geometry

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Geometry Script Tools Module
 *
 * Provides tools for procedural geometry and mesh generation:
 * - create_dynamic_mesh_actor: Spawn a dynamic mesh actor for runtime geometry
 * - generate_box_mesh: Generate a box primitive as a dynamic mesh
 * - generate_sphere_mesh: Generate a sphere primitive as a dynamic mesh
 * - generate_cylinder_mesh: Generate a cylinder primitive as a dynamic mesh
 * - generate_capsule_mesh: Generate a capsule primitive as a dynamic mesh
 * - generate_torus_mesh: Generate a torus primitive as a dynamic mesh
 * - generate_plane_mesh: Generate a plane primitive as a dynamic mesh
 * - generate_linear_stairs_mesh: Generate a straight staircase mesh
 * - generate_curved_stairs_mesh: Generate a curved staircase mesh
 * - generate_revolve_polygon_mesh: Revolve a 2D profile into a lathed 3D mesh
 * - generate_spiral_revolve_polygon_mesh: Revolve a 2D profile upward along a spiral path
 * - generate_swept_polygon_mesh: Sweep a 2D profile along a 3D polyline path
 * - generate_spline_swept_polygon_mesh: Sweep a 2D profile along a spline actor
 * - generate_extruded_polygon_mesh: Generate an extruded polygon on a dynamic mesh
 * - generate_tapered_extruded_polygon_mesh: Generate a tapered extrusion for pyramids, obelisks, pylons, and battered walls
 * - create_grass_card_mesh: Create a crossed-plane foliage card static mesh asset
 * - apply_mesh_boolean: Perform CSG boolean (union, subtract, intersect) on meshes
 * - apply_mesh_self_union: Repair self-intersections and overlapping shells within one mesh
 * - apply_mesh_plane_cut: Cut a mesh by a plane and optionally fill the open boundary
 * - apply_mesh_plane_slice: Slice a mesh into halves with an optional separation gap
 * - apply_mesh_mirror: Mirror a dynamic mesh across a configurable plane
 * - apply_mesh_noise: Apply noise displacement to mesh vertices
 * - apply_mesh_uniform_remesh: Redistribute triangles toward a target density
 * - apply_mesh_uniform_tessellation: Uniformly subdivide a dynamic mesh
 * - apply_mesh_pn_tessellation: Smoothly subdivide a dynamic mesh with PN tessellation
 * - apply_mesh_selective_tessellation: Subdivide a stored selection with concentric refinement
 * - weld_mesh_edges: Weld compatible open boundaries to close cracks
 * - fill_mesh_holes: Fill open boundary loops in a mesh
 * - repair_mesh_degenerate_geometry: Repair or remove tiny/degenerate triangles
 * - split_mesh_bowties: Split invalid bowtie vertices in topology and attributes
 * - auto_repair_mesh_normals: Repair inconsistent triangle winding and normals
 * - recompute_mesh_normals: Rebuild normals on the full mesh or a stored selection
 * - auto_generate_mesh_uvs: Generate UVs for a dynamic mesh using UE Geometry Script
 * - get_dynamic_mesh_stats: Query dynamic mesh topology/material/UV stats
 * - select_mesh_faces_by_normal: Store a reusable face selection by normal direction
 * - select_mesh_faces_by_material: Store a reusable face selection by material ID
 * - select_mesh_connected_region: Expand a stored selection to its connected region
 * - extrude_mesh_selection: Extrude a stored face selection
 * - inset_outset_mesh_selection: Inset or outset a stored face selection
 * - bevel_mesh_selection: Bevel a stored face selection
 * - duplicate_mesh_selection: Duplicate a stored face selection
 * - disconnect_mesh_selection: Disconnect a stored face selection
 * - apply_mesh_shell: Thicken a mesh into a shell
 * - apply_mesh_smooth: Smooth an entire mesh or stored selection
 * - delete_mesh_selection: Delete triangles referenced by a stored selection
 * - generate_mesh_collision: Generate simple collision for a dynamic mesh actor
 * - set_static_mesh_nanite: Enable or disable Nanite on a static mesh asset
 * - set_mesh_material: Assign material to a dynamic mesh
 * - convert_static_to_dynamic: Convert a static mesh to a dynamic mesh for editing
 * - export_dynamic_to_static: Bake a dynamic mesh back to a static mesh asset
 */
class RIFTBORNAI_API FGeometryScriptToolsModule : public TToolModuleBase<FGeometryScriptToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("GeometryScriptTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_CreateDynamicMeshActor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GenerateBoxMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GenerateSphereMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GenerateCylinderMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GenerateCapsuleMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GenerateTorusMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GeneratePlaneMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GenerateLinearStairsMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GenerateCurvedStairsMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GenerateRevolvePolygonMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GenerateSpiralRevolvePolygonMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GenerateSweptPolygonMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GenerateSplineSweptPolygonMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GenerateExtrudedPolygonMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GenerateTaperedExtrudedPolygonMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateGrassCardMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshBoolean(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshSelfUnion(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshPlaneCut(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshPlaneSlice(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshMirror(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshNoise(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshUniformRemesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshUniformTessellation(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshPNTessellation(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshSelectiveTessellation(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_WeldMeshEdges(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_FillMeshHoles(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RepairMeshDegenerateGeometry(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SplitMeshBowties(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AutoRepairMeshNormals(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RecomputeMeshNormals(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AutoGenerateMeshUVs(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetDynamicMeshStats(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SelectMeshFacesByNormal(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SelectMeshFacesByMaterial(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SelectMeshConnectedRegion(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ExtrudeMeshSelection(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_InsetOutsetMeshSelection(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_BevelMeshSelection(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DuplicateMeshSelection(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DisconnectMeshSelection(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshShell(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshSmooth(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DeleteMeshSelection(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GenerateMeshCollision(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetStaticMeshNanite(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetMeshMaterial(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ConvertStaticToDynamic(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ExportDynamicToStatic(const FClaudeToolCall& Call);

    // Modeling additions — verified against UE 5.7 GeometryScript / Modeling APIs
    static FClaudeToolResult Tool_ApplyMeshTransform(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshBend(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshTwist(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshFlare(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshSimplify(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetVertexPosition(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyConvexDecomposition(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AlignActorToGround(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyMeshSubdivision(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_InsertEdgeLoop(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ExportStaticMeshToFBX(const FClaudeToolCall& Call);

    // Architectural assemblies — high-instance-count constructs via HISM
    static FClaudeToolResult Tool_BuildBlockPyramid(const FClaudeToolCall& Call);

private:
    static FString MakeSelectionStoreKey(const FString& ActorName, const FString& SelectionName);
    static void StoreMeshSelection(const FString& ActorName, const FString& SelectionName, const struct FGeometryScriptMeshSelection& Selection);
    static bool LoadMeshSelection(const FString& ActorName, const FString& SelectionName, struct FGeometryScriptMeshSelection& OutSelection);
    static void RemoveMeshSelection(const FString& ActorName, const FString& SelectionName);
    static void RemoveAllMeshSelectionsForActor(const FString& ActorName);
    static int32 GetUniqueSelectionCount(class UDynamicMesh* DynMesh, const struct FGeometryScriptMeshSelection& Selection);
};
