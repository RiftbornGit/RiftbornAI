// Copyright RiftbornAI. All Rights Reserved.
// BlueprintHeuristics - Instant suggestions without LLM (< 80ms)

#pragma once

#include "CoreMinimal.h"
#include "BlueprintPatch.h"
#include "EditorContextService.h"

/**
 * Heuristic Blueprint Suggestions
 * 
 * Provides instant suggestions based on pin type, node context, and common patterns.
 * These appear immediately (< 80ms) while the LLM is processing.
 * 
 * The LLM suggestion replaces/augments these when it arrives.
 */
class RIFTBORNAI_API FBlueprintHeuristics
{
public:
	static FBlueprintHeuristics& Get();
	
	/**
	 * Generate instant suggestions based on context
	 * @param Context The current Blueprint editor context
	 * @return Array of patches, ordered by relevance
	 */
	TArray<FBlueprintPatch> GenerateInstantSuggestions(const FBlueprintGraphContext& Context);
	
	/**
	 * Get suggestions for a specific pin type
	 * @param PinType The pin category (exec, bool, float, object, etc.)
	 * @param PinSubType For objects/structs, the specific type
	 * @param bIsOutput Whether the pin is an output (we suggest what to wire TO it)
	 * @param Position Suggested position for new nodes
	 * @return Array of patches
	 */
	TArray<FBlueprintPatch> GetPinTypeSuggestions(
		const FString& PinType,
		const FString& PinSubType,
		bool bIsOutput,
		const FVector2D& Position);
	
	/**
	 * Get suggestions for exec flow continuation
	 * @param CurrentNodeType The type of node we're extending from
	 * @param Position Suggested position for new nodes
	 * @return Array of patches
	 */
	TArray<FBlueprintPatch> GetExecFlowSuggestions(
		const FString& CurrentNodeType,
		const FVector2D& Position);
	
	/**
	 * Get common patterns for the given parent class
	 * @param ParentClass The Blueprint's parent class (Character, Actor, etc.)
	 * @return Array of common pattern patches
	 */
	TArray<FBlueprintPatch> GetClassPatternSuggestions(const FString& ParentClass);

private:
	FBlueprintHeuristics() = default;
	
	// === Pin Type -> Suggestions ===
	TArray<FBlueprintPatch> SuggestForExecPin(bool bIsOutput, const FVector2D& Position);
	TArray<FBlueprintPatch> SuggestForBoolPin(bool bIsOutput, const FVector2D& Position);
	TArray<FBlueprintPatch> SuggestForFloatPin(bool bIsOutput, const FVector2D& Position);
	TArray<FBlueprintPatch> SuggestForIntPin(bool bIsOutput, const FVector2D& Position);
	TArray<FBlueprintPatch> SuggestForStringPin(bool bIsOutput, const FVector2D& Position);
	TArray<FBlueprintPatch> SuggestForVectorPin(bool bIsOutput, const FVector2D& Position);
	TArray<FBlueprintPatch> SuggestForObjectPin(const FString& ObjectType, bool bIsOutput, const FVector2D& Position);
	
	// Helper to create a simple patch
	FBlueprintPatch CreateNodePatch(
		const FString& Description,
		const FString& NodeClass,
		const FString& FunctionRef,
		const FVector2D& Position,
		float Confidence = 0.8f);
};
