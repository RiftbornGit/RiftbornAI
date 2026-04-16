// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// Predicate Evaluator - Structured predicate system for PROOF mode
//
// This bridges the Python predicate_protocol.py concepts to C++.
// Predicates are DATA, not CODE. Each predicate has a name and args.

#pragma once

#include "CoreMinimal.h"
#include "Agent/AgentEvent.h"
#include "PredicateEvaluator.generated.h"

/**
 * Predicate types supported by the evaluator
 */
UENUM(BlueprintType)
enum class EPredicateType : uint8
{
	/** Named validator - calls specific C++ validation function */
	NamedValidator,
	
	/** Tool query - executes a read-only tool and checks result */
	ToolQuery,
	
	/** Snapshot assertion - checks cached state */
	SnapshotAssert,
};

/**
 * A structured predicate request - DATA not CODE
 * 
 * Example:
 *   FPredicateRequest Request;
 *   Request.Type = EPredicateType::NamedValidator;
 *   Request.Name = TEXT("is_pie_running");
 *   Request.ExpectedBool = false;
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FPredicateRequest
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predicate")
	EPredicateType Type = EPredicateType::NamedValidator;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predicate")
	FString Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predicate")
	TMap<FString, FString> Args;
	
	/** For assertions - expected bool value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predicate")
	bool ExpectedBool = true;
	
	/** For assertions - expected int value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predicate")
	int32 ExpectedInt = 0;
	
	/** For assertions - expected string value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predicate")
	FString ExpectedString;
	
	/** Tolerance for numeric comparisons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predicate")
	float Tolerance = 0.0f;
	
	/** Generate deterministic hash */
	FString ComputeHash() const;
	
	/** Parse from acceptance predicate string (e.g., "is_pie_running" or "actors_with_tag:enemy_spawn:min=3") */
	static FPredicateRequest Parse(const FString& PredicateString);
};

/**
 * Whitelisted validators - the ONLY validators allowed in PROOF mode
 * Each corresponds to a deterministic C++ function.
 */
namespace PredicateValidators
{
	// Validator function signature
	typedef FPredicateResult (*FValidatorFunc)(const TMap<FString, FString>& Args);
	
	/** Get validator function by name, or nullptr if not whitelisted */
	FValidatorFunc GetValidator(const FString& Name);
	
	/** Get all whitelisted validator names */
	TArray<FString> GetWhitelistedNames();
	
	/** Check if a validator name is whitelisted */
	bool IsWhitelisted(const FString& Name);
	
	/** Get metadata for a predicate, or nullptr if not configured */
	const FPredicateMeta* GetPredicateMeta(const FString& Name);
	
	/** Check if predicate has explicit metadata (required for PROOF mode) */
	bool HasExplicitMeta(const FString& Name);
	
	// === Whitelisted Validators ===
	
	/** PIE is running */
	FPredicateResult IsPIERunning(const TMap<FString, FString>& Args);
	
	/** PIE not running (inverse) */
	FPredicateResult IsPIENotRunning(const TMap<FString, FString>& Args);
	
	/** Player pawn is valid (in PIE) */
	FPredicateResult PIEPlayerValid(const TMap<FString, FString>& Args);
	
	/** All blueprints compile */
	FPredicateResult AllBlueprintsCompile(const TMap<FString, FString>& Args);
	
	/** Specific blueprint compiles */
	FPredicateResult BlueprintCompilesClean(const TMap<FString, FString>& Args);
	
	/** Navmesh is built */
	FPredicateResult NavmeshBuilt(const TMap<FString, FString>& Args);
	
	/** Point is reachable on navmesh */
	FPredicateResult PointReachableOnNavmesh(const TMap<FString, FString>& Args);
	
	/** Count actors with tag */
	FPredicateResult ActorsWithTagCount(const TMap<FString, FString>& Args);
	
	/** Actor exists with tag */
	FPredicateResult ActorExistsWithTag(const TMap<FString, FString>& Args);
	
	/** Asset exists at path */
	FPredicateResult AssetExists(const TMap<FString, FString>& Args);
	
	/** UE error count check */
	FPredicateResult GetUEErrorCount(const TMap<FString, FString>& Args);
	
	/** No new UE errors since checkpoint */
	FPredicateResult HasNoNewUEErrors(const TMap<FString, FString>& Args);
	
	/** Governance session is clean (no bypass attempts) */
	FPredicateResult SessionIsClean(const TMap<FString, FString>& Args);
	
	/** Get bypass attempt count */
	FPredicateResult BypassAttemptCount(const TMap<FString, FString>& Args);
}

/**
 * Predicate Evaluator - Evaluates structured predicates
 * 
 * Usage:
 *   FPredicateEvaluator Evaluator;
 *   FPredicateResult Result = Evaluator.Evaluate(Request);
 *   
 *   // Or evaluate by name
 *   FPredicateResult Result = Evaluator.EvaluateByName("is_pie_running");
 */
class RIFTBORNAI_API FPredicateEvaluator
{
public:
	/**
	 * Evaluate a structured predicate request
	 */
	FPredicateResult Evaluate(const FPredicateRequest& Request);
	
	/**
	 * Evaluate by predicate name string (parses string into request)
	 * Format: "validator_name" or "validator_name:arg1=val1:arg2=val2"
	 */
	FPredicateResult EvaluateByName(const FString& PredicateString);
	
	/**
	 * Evaluate multiple predicates, return all results
	 */
	TArray<FPredicateResult> EvaluateAll(const TArray<FString>& PredicateNames);
	
	/**
	 * Check if all predicates pass
	 */
	bool CheckAll(const TArray<FString>& PredicateNames, TArray<FPredicateResult>& OutResults);
	
	/**
	 * Check if a predicate has explicit metadata registered
	 */
	bool HasExplicitMeta(const FString& PredicateName) const;
	
	/**
	 * Get metadata for a predicate (returns default if not registered)
	 */
	FPredicateMeta GetPredicateMeta(const FString& PredicateName) const;
	
	/** Get singleton instance */
	static FPredicateEvaluator& Get();
	
private:
	FPredicateEvaluator() = default;
	
	/** Evaluate a named validator */
	FPredicateResult EvaluateValidator(const FPredicateRequest& Request);
	
	/** Registry of predicate metadata */
	TMap<FString, FPredicateMeta> PredicateMetaRegistry;
};
