// Copyright RiftbornAI. All Rights Reserved.
// Photorealism Tools — ecosystem growth simulation, per-instance material variation,
// advanced lighting realism (SSS, light functions, volumetric, caustics).
// Verified against UE 5.7: FLandscapeEditDataInterface, SetCustomDataValue,
// USubsurfaceProfile, ULightComponent::LightFunctionMaterial, SetVolumetricScatteringIntensity.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FPhotorealismToolsModule
 * 
 * Photorealism Tools — ecosystem growth simulation, per-instance material variation,
 */
class RIFTBORNAI_API FPhotorealismToolsModule : public TToolModuleBase<FPhotorealismToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("PhotorealismTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// Multi-century forest growth simulation: terrain analysis + seed dispersal + competition + mortality
	static FClaudeToolResult Tool_SimulateEcosystemGrowth(const FClaudeToolCall& Call);

	// Per-instance material uniqueness via custom data floats + procedural noise wiring
	static FClaudeToolResult Tool_SetupInstanceVariation(const FClaudeToolCall& Call);

	// Subsurface profiles + light functions + per-light volumetric + water caustic decals
	static FClaudeToolResult Tool_SetupAdvancedLightingRealism(const FClaudeToolCall& Call);

	// Compute soil depth/fertility/pH from heightmap, paint landscape layers
	static FClaudeToolResult Tool_ComputeSoilMap(const FClaudeToolCall& Call);
};
