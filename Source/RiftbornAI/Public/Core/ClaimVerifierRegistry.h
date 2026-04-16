// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// Claim Verifier Registry - enforces "no claim without verification method"

#pragma once

#include "CoreMinimal.h"
#include "ClaimTypes.h"

/**
 * Function signature for claim verifiers
 * 
 * @param Claim - The claim to verify (read predicate + args)
 * @param OutReason - Set to explanation if verification fails or is unavailable
 * @param OutEvidence - Set to evidence JSON if verification succeeds
 * @return true if claim is verified, false if refuted
 * @note If verifier cannot determine truth, it should return false and set OutReason to explain why
 */
using FClaimVerifierFn = TFunction<bool(const FToolClaim& Claim, FString& OutReason, TSharedPtr<FJsonObject>& OutEvidence)>;

/**
 * Registry of claim predicates and their verifiers
 * 
 * The One Rule: No claim without a verification method. No action without a claim.
 * - If you can't test it, you can't claim it
 * - If you claim it, you must say how to test it
 * - If the test fails, the claim is refuted and recorded
 */
class RIFTBORNAI_API FClaimVerifierRegistry
{
public:
    static FClaimVerifierRegistry& Get();

    /**
     * Register a verifier for a predicate
     * @param Predicate - The predicate name (e.g., "actor.exists", "bp.compiles")
     * @param Verifier - Function that verifies claims with this predicate
     */
    void RegisterVerifier(const FString& Predicate, FClaimVerifierFn Verifier);

    /**
     * Check if a predicate has a registered verifier
     */
    bool HasVerifier(const FString& Predicate) const;

    /**
     * Verify a claim
     * 
     * @param Claim - The claim to verify (will be modified with Status and StatusReason)
     * @param EvidenceOut - Collected evidence if verification succeeds
     * @return true if claim is verified, false if refuted or unverifiable
     * 
     * Sets Claim.Status to:
     * - Verified: predicate satisfied, evidence collected
     * - Refuted: predicate failed, reason provided
     * - Unverified: no verifier registered (reduces authority)
     */
    bool Verify(FToolClaim& Claim, TSharedPtr<FJsonObject>& EvidenceOut);

    /**
     * Verify all claims in a proof bundle
     * @return Number of verified claims
     */
    int32 VerifyAll(FClaimProofBundle& Bundle);

    /**
     * Get all registered predicates
     */
    TArray<FString> GetRegisteredPredicates() const;

    /**
     * Register built-in verifiers for core predicates
     * Called at module startup
     */
    void RegisterBuiltinVerifiers();

private:
    FClaimVerifierRegistry() = default;
    
    TMap<FString, FClaimVerifierFn> Verifiers;
    mutable FCriticalSection VerifiersLock;
};

/**
 * Helper to create claims with proper predicate registration check
 */
namespace ClaimBuilder
{
    /**
     * Create a Fact claim (verifiable via query)
     */
    RIFTBORNAI_API FToolClaim Fact(
        const FString& ToolName,
        const FString& Predicate,
        const TSharedPtr<FJsonObject>& Args,
        const FString& VerifyMethod = TEXT("scene_query")
    );

    /**
     * Create a Behavior claim (verifiable via harness)
     */
    RIFTBORNAI_API FToolClaim Behavior(
        const FString& ToolName,
        const FString& Predicate,
        const TSharedPtr<FJsonObject>& Args,
        const FString& VerifyMethod = TEXT("harness")
    );

    /**
     * Create a Constraint claim (budget/safety gate)
     */
    RIFTBORNAI_API FToolClaim Constraint(
        const FString& ToolName,
        const FString& Predicate,
        const TSharedPtr<FJsonObject>& Args,
        const FString& VerifyMethod = TEXT("metric_check")
    );
    
    // Convenience helpers for common spawn_actor claims
    
    /** Create actor.exists claim */
    RIFTBORNAI_API FToolClaim ActorExists(const FString& ActorLabel);
    
    /** Create actor.transform claim (location with tolerance) */
    RIFTBORNAI_API FToolClaim ActorAtLocation(const FString& ActorLabel, const FVector& Location, float Tolerance = 1.0f);
    
    /** Create actor.class_is claim */
    RIFTBORNAI_API FToolClaim ActorClassIs(const FString& ActorLabel, const FString& ExpectedClass);
    
    /** Create world.gamemode_override_is claim */
    RIFTBORNAI_API FToolClaim WorldGameModeIs(const FString& ExpectedGameModeClass);
}
