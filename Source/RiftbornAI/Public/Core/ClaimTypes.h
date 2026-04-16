// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// Claim → Proof pipeline: the spine of epistemic authority

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "ClaimTypes.generated.h"

/**
 * Claim status - only 4 states, no room for ambiguity
 */
UENUM(BlueprintType)
enum class EClaimStatus : uint8
{
    Pending,      // Not yet verified
    Verified,     // Predicate satisfied, evidence collected
    Refuted,      // Predicate failed, evidence collected
    Unverified    // Verifier unavailable or not applicable - REDUCES AUTHORITY
};

/**
 * Claim kind - determines which verification path to use
 */
UENUM(BlueprintType)
enum class EClaimKind : uint8
{
    Fact,         // Can be checked via scene/asset query (actor.exists, bp.compiles)
    Behavior,     // Can be checked via runtime harness (gameplay.works)
    Constraint    // Budget/safety/determinism gate (perf.under_budget, no_crashes)
};

/**
 * A single claim made by a tool about the world
 * 
 * Key design: Predicate + Args is your semantic diff unit.
 * That's how time-travel works later.
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FToolClaim
{
    GENERATED_BODY()

    /** Stable hash of predicate+args for deduplication */
    UPROPERTY(BlueprintReadOnly, Category = "Claim")
    FString ClaimId;

    /** Tool that asserted this claim */
    UPROPERTY(BlueprintReadOnly, Category = "Claim")
    FString ToolName;

    /** When this claim was made */
    UPROPERTY(BlueprintReadOnly, Category = "Claim")
    FDateTime Timestamp;

    /** What kind of claim (determines verification path) */
    UPROPERTY(BlueprintReadOnly, Category = "Claim")
    EClaimKind Kind = EClaimKind::Fact;

    /** The predicate being asserted (from registry, e.g. "actor.exists", "bp.compiles") */
    UPROPERTY(BlueprintReadOnly, Category = "Claim")
    FString Predicate;

    /** Typed arguments for the predicate (e.g. {"actor_name": "Mybox", "loc": [0,0,0]}) */
    TSharedPtr<FJsonObject> Args;

    /** Which verifier to use (e.g. "scene_query", "compile_check", "harness:gameplay") */
    UPROPERTY(BlueprintReadOnly, Category = "Claim")
    FString VerifyMethod;

    /** Current status */
    UPROPERTY(BlueprintReadOnly, Category = "Claim")
    EClaimStatus Status = EClaimStatus::Pending;

    /** Why this status (especially for Unverified/Refuted) */
    UPROPERTY(BlueprintReadOnly, Category = "Claim")
    FString StatusReason;

    /** Compute stable claim ID from predicate + args */
    void ComputeClaimId();

    /** JSON serialization */
    TSharedPtr<FJsonObject> ToJson() const;
    static FToolClaim FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * A proof bundle - evidence for a set of claims from a single tool execution
 * 
 * CHAIN BINDING (2026-01-31): Proofs now include governance configuration hashes
 * to prove they were generated under specific contract rules.
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FClaimProofBundle
{
    GENERATED_BODY()

    /** Unique proof ID (timestamp + toolcall hash) */
    UPROPERTY(BlueprintReadOnly, Category = "Proof")
    FString ProofId;

    /** Session ID for grouping */
    UPROPERTY(BlueprintReadOnly, Category = "Proof")
    FString SessionId;

    /** The tool call that generated these claims */
    UPROPERTY(BlueprintReadOnly, Category = "Proof")
    FString ToolCallId;

    /** Tool that was executed */
    UPROPERTY(BlueprintReadOnly, Category = "Proof")
    FString ToolName;

    /** Original tool arguments (for replay) */
    TSharedPtr<FJsonObject> ToolArgs;

    /** All claims made by this tool execution */
    UPROPERTY(BlueprintReadOnly, Category = "Proof")
    TArray<FToolClaim> Claims;

    /** Collected evidence (query results, snapshots, timings) */
    TSharedPtr<FJsonObject> Evidence;

    /** When proof was finalized */
    UPROPERTY(BlueprintReadOnly, Category = "Proof")
    FDateTime FinalizedAt;

    /** SHA256 of canonical JSON (tamper detection) */
    UPROPERTY(BlueprintReadOnly, Category = "Proof")
    FString ProofHash;
    
    /** CHAIN LINKING: Hash of previous proof in chain (empty for first proof) */
    UPROPERTY(BlueprintReadOnly, Category = "Proof")
    FString PreviousProofHash;

    /** Environment signature (UE version, project, etc.) */
    UPROPERTY(BlueprintReadOnly, Category = "Proof")
    FString EnvironmentSignature;
    
    // =========================================================================
    // CHAIN BINDING FIELDS (2026-01-31) - Prove governance configuration
    // =========================================================================
    
    /** Hash of contracts.json at execution time */
    UPROPERTY(BlueprintReadOnly, Category = "ChainBinding")
    FString ContractsRegistryHash;
    
    /** Hash of the approved plan that authorized this execution */
    UPROPERTY(BlueprintReadOnly, Category = "ChainBinding")
    FString PlanHash;
    
    /** Unreal Engine version */
    UPROPERTY(BlueprintReadOnly, Category = "ChainBinding")
    FString EngineVersion;
    
    /** RiftbornAI plugin version */
    UPROPERTY(BlueprintReadOnly, Category = "ChainBinding")
    FString PluginVersion;
    
    /** Project GUID for multi-project verification */
    UPROPERTY(BlueprintReadOnly, Category = "ChainBinding")
    FString ProjectGuid;
    
    /** Was PROOF mode enabled for this execution */
    UPROPERTY(BlueprintReadOnly, Category = "ChainBinding")
    bool bProofModeEnabled = false;
    
    /** Was LAB mode enabled for this execution (2026-02-02) */
    UPROPERTY(BlueprintReadOnly, Category = "ChainBinding")
    bool bLabModeEnabled = false;
    
    /** Was session tainted at execution time (2026-02-02) */
    UPROPERTY(BlueprintReadOnly, Category = "ChainBinding")
    bool bSessionTainted = false;
    
    /** Step index within the plan (0-based) */
    UPROPERTY(BlueprintReadOnly, Category = "ChainBinding")
    int32 PlanStepIndex = -1;

    /** Compute counts */
    int32 GetVerifiedCount() const;
    int32 GetRefutedCount() const;
    int32 GetUnverifiedCount() const;

    /** Compute overall success (all claims verified) */
    bool IsFullyVerified() const;

    /** JSON serialization */
    TSharedPtr<FJsonObject> ToJson() const;
    static FClaimProofBundle FromJson(const TSharedPtr<FJsonObject>& Json);

    /** Compute and set ProofHash (alias for Finalize) */
    void ComputeHash() { Finalize(); }
    
    /** Compute and set ProofHash */
    void Finalize();
    
    /** 
     * Populate chain binding fields from current environment
     * Call this BEFORE Finalize() to ensure binding fields are included in hash
     */
    void PopulateChainBinding(const FString& InPlanHash = TEXT(""), int32 InStepIndex = -1);
};

/**
 * Authority score for a tool - computed from proof history
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FToolAuthority
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Authority")
    FString ToolName;

    UPROPERTY(BlueprintReadOnly, Category = "Authority")
    int32 VerifiedCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Authority")
    int32 RefutedCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Authority")
    int32 UnverifiedCount = 0;

    /** Authority score: verified / (verified + refuted + 0.25*unverified) */
    float GetAuthorityScore() const;

    /** True if authority is too low for autonomous execution */
    bool RequiresEscalation(float Threshold = 0.7f) const;
};
