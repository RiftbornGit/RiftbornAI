// Copyright RiftbornAI. All Rights Reserved.
// Creative build tools based on seeded grammar expansion.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"
#include "Math/RandomStream.h"

/** Creative build tools for assembled structures and walls. */
class RIFTBORNAI_API FCreativeBuildToolsModule : public TToolModuleBase<FCreativeBuildToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("CreativeBuildTools"); }
	
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

private:
	// Main entry points
	static FClaudeToolResult Tool_CreativeBuild(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreativeWall(const FClaudeToolCall& Call);
	
	// =======================================================================
	// STRUCTURAL TYPES
	// =======================================================================
	
	/** Structural role of a building part */
	enum class EPartRole : uint8
	{
		Floor,
		Wall,
		Roof,
		Door,
		Window,
		Column,
		Stair,
		Fence,
		Chimney,
		Foundation,
		Porch,
		Balcony,
		Buttress,
		Awning,
	};

	/** Roof geometry type — expanded from old fixed gable */
	enum class ERoofType : uint8
	{
		Gable,      // Two angled planes meeting at ridge (the old default)
		Hip,        // Four angled planes, no vertical gable ends
		Flat,       // Flat slab with slight parapet
		Gambrel,    // Barn-style, two slopes per side
		Dome,       // Hemispherical approximation (for towers, mosques)
		Shed,       // Single angled plane
		Conical,    // Cone (for towers)
	};
	
	/** Structure archetype — what grammar ruleset to use */
	enum class EArchetype : uint8
	{
		House,
		Tower,
		Bridge,
		Wall,
		Castle,
		Chapel,
		Warehouse,
		Generic,
	};
	
	// =======================================================================
	// STYLE SYSTEM — coherent material/proportion palette per build
	// =======================================================================
	
	/** PBR material preset */
	struct FMaterialPreset
	{
		FString Color;   // "R,G,B" (0-1 range)
		float Roughness;
		float Metallic;
	};
	
	/**
	 * A complete style palette for one build.
	 * Instead of per-part keyword matching, the style is chosen ONCE and
	 * propagated through all parts with role-based variation.
	 */
	struct FBuildStyle
	{
		FString StyleName;           // e.g. "Medieval Stone", "Modern Concrete"
		FMaterialPreset WallPrimary;
		FMaterialPreset WallAccent;  // For trim, corners, quoins
		FMaterialPreset RoofMat;
		FMaterialPreset FloorMat;
		FMaterialPreset DetailMat;   // Shutters, railings, porch columns
		FMaterialPreset FoundationMat;
		
		// Proportional tendencies (0-1 range, grammar uses these as bias)
		float WallHeightBias;        // 0=squat, 1=tall
		float RoofSteepnessBias;     // 0=flat, 1=steep
		float OrnamentationBias;     // 0=plain, 1=ornate (more details)
		float SymmetryBias;          // 0=organic/asymmetric, 1=rigid symmetric
		
		/** Get material for a given part role with optional accent flag */
		FMaterialPreset GetMaterialForRole(EPartRole Role, bool bAccent = false) const;
	};
	
	// =======================================================================
	// BUILD PARTS (same as before, but with style reference)
	// =======================================================================
	
	/** A single structural part to create */
	struct FBuildPart
	{
		FString Name;
		EPartRole Role;
		FVector Location;
		FRotator Rotation;
		FVector Scale;       // For static meshes (UE scale factor)
		FVector Dimensions;  // For dynamic meshes (actual size in cm)
		FString MaterialColor; // "R,G,B" (0-1 range)
		float Roughness = 0.7f;
		float Metallic = 0.0f;
		bool bUseDynamicMesh = false;
		
		struct FOpening
		{
			FString Name;
			FVector RelativeLocation;
			FVector Size;
		};
		TArray<FOpening> Openings;
	};
	
	/** A complete building concept decomposition */
	struct FBuildConcept
	{
		FString ConceptName;
		TArray<FBuildPart> Parts;
		FVector Origin;
		FString FolderName;
		FBuildStyle Style;
		int32 Seed;
		float BrainScore;   // Legacy compatibility field. -1 when external scoring is disabled.
	};
	
	// =======================================================================
	// GRAMMAR EXPANSION — seeded production rules
	// =======================================================================
	
	/** Parse description into archetype + size + style hints */
	struct FBuildRequest
	{
		EArchetype Archetype;
		FString Description;
		FVector Origin;
		int32 Seed;
		float SizeMultiplier;   // Parsed from "small"/"large"/etc.
		TArray<FString> StyleHints;  // Extracted keywords: "medieval", "stone", "rustic", etc.
	};
	
	static FBuildRequest ParseRequest(const FString& Description, const FVector& Origin, int32 Seed);
	
	/** Build a style palette from request hints + seed */
	static FBuildStyle BuildStyleFromRequest(const FBuildRequest& Request, FRandomStream& Rng);
	
	/** Grammar expansion — each archetype has its own production rules */
	static FBuildConcept ExpandHouseGrammar(const FBuildRequest& Request, const FBuildStyle& Style, FRandomStream& Rng);
	static FBuildConcept ExpandTowerGrammar(const FBuildRequest& Request, const FBuildStyle& Style, FRandomStream& Rng);
	static FBuildConcept ExpandBridgeGrammar(const FBuildRequest& Request, const FBuildStyle& Style, FRandomStream& Rng);
	static FBuildConcept ExpandWallGrammar(const FBuildRequest& Request, const FBuildStyle& Style, FRandomStream& Rng);
	static FBuildConcept ExpandCastleGrammar(const FBuildRequest& Request, const FBuildStyle& Style, FRandomStream& Rng);
	static FBuildConcept ExpandChapelGrammar(const FBuildRequest& Request, const FBuildStyle& Style, FRandomStream& Rng);
	static FBuildConcept ExpandWarehouseGrammar(const FBuildRequest& Request, const FBuildStyle& Style, FRandomStream& Rng);
	static FBuildConcept ExpandGenericGrammar(const FBuildRequest& Request, const FBuildStyle& Style, FRandomStream& Rng);
	
	// =======================================================================
	// GRAMMAR PRIMITIVES — reusable sub-rules
	// =======================================================================
	
	/** Choose roof type based on style bias + rng */
	static ERoofType ChooseRoofType(EArchetype Archetype, const FBuildStyle& Style, FRandomStream& Rng);
	
	/** Generate a polygonal wall ring (N sides, not always 4) */
	static void EmitWallRing(
		FBuildConcept& Concept, const FBuildStyle& Style, FRandomStream& Rng,
		float Width, float Depth, int32 NumSides, float WallHeight, float WallThickness,
		const FVector& BaseOrigin, int32 DoorWallIndex, int32 NumWindows);
	
	/** Generate roof geometry for the given type */
	static void EmitRoof(
		FBuildConcept& Concept, const FBuildStyle& Style, FRandomStream& Rng,
		ERoofType RoofType, float Width, float Depth, float WallHeight,
		float RoofSteepness, const FVector& BaseOrigin);
	
	/** Generate optional details (chimney, porch, buttresses, etc.) */
	static void EmitDetails(
		FBuildConcept& Concept, const FBuildStyle& Style, FRandomStream& Rng,
		float Width, float Depth, float WallHeight, const FVector& BaseOrigin);
	
	/** Add a floor/foundation slab */
	static void EmitFloor(
		FBuildConcept& Concept, const FBuildStyle& Style,
		float Width, float Depth, const FVector& BaseOrigin, bool bIsFoundation);
	
	// =======================================================================
	// LEGACY CANDIDATE SELECTION
	// =======================================================================
	
	/**
	 * Legacy compatibility hook for candidate evaluation.
	 * External brain scoring is no longer part of the shipped product surface,
	 * so this returns -1.0 and callers fall back to deterministic selection.
	 */
	static float ScoreBuildPlan(const FBuildConcept& Concept);
	
	/**
	 * Generate N candidate decompositions with different sub-seeds,
	 * return the selected concept.
	 * External scoring is disabled, so selection is deterministic.
	 */
	static FBuildConcept SelectBestCandidate(
		const FBuildRequest& Request, int32 NumCandidates = 3);
	
	// =======================================================================
	// MATERIAL CATALOG
	// =======================================================================
	
	static FMaterialPreset GetMaterial(const FString& Intent);
	
	/** Jitter a color slightly with rng for visual variety */
	static FMaterialPreset JitterMaterial(const FMaterialPreset& Base, FRandomStream& Rng, float Amount = 0.05f);
	
	// =======================================================================
	// EXECUTION ENGINE (unchanged from v1)
	// =======================================================================
	
	static FClaudeToolResult ExecuteBuildPlan(const FBuildConcept& Concept);
	static bool CreateStaticMeshPart(const FBuildPart& Part, const FString& Folder, TArray<FString>& CreatedActors, FString& Error);
	static bool CreateDynamicMeshPart(const FBuildPart& Part, const FString& Folder, TArray<FString>& CreatedActors, FString& Error);
	static bool CutOpening(const FString& WallActorName, const FBuildPart::FOpening& Opening, const FVector& WallLocation, TArray<FString>& CreatedActors, FString& Error);
	static bool SetActorColor(const FString& ActorLabel, const FString& Color, float Roughness, float Metallic, FString& Error);
	
	static UWorld* GetEditorWorld();
	
	// =======================================================================
	// HELPERS
	// =======================================================================
	
	/** Lerp between two FMaterialPresets */
	static FMaterialPreset LerpMaterial(const FMaterialPreset& A, const FMaterialPreset& B, float Alpha);
	
	/** Random float in range from FRandomStream */
	static float RngRange(FRandomStream& Rng, float Min, float Max);
	
	/** Random int in range from FRandomStream */
	static int32 RngRangeInt(FRandomStream& Rng, int32 Min, int32 Max);
};
