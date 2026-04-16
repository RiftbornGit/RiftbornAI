// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// Tool Execution Proof (TEP) v1.0.0 - FROZEN SPEC
// The atomic unit of governed mutation evidence

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "ExecCtxValidator.h"
#include "ClaimTypes.h"
#include "ToolExecutionProof.generated.h"

/**
 * TEP outcome status - only 4 states
 */
UENUM(BlueprintType)
enum class ETEPOutcomeStatus : uint8
{
    Passed,     // Tool executed successfully
    Failed,     // Tool execution failed
    Refused,    // Policy blocked execution
    Error       // System error (not tool failure)
};

/**
 * TEP rollback status
 */
UENUM(BlueprintType)
enum class ETEPRollbackStatus : uint8
{
    NotRequired,    // Success - no rollback needed
    Succeeded,      // Rollback completed successfully
    Failed,         // Rollback attempted but failed
    Skipped         // Rollback not attempted (by design or error)
};

/**
 * Policy decision - why execution was allowed or refused
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FTEPPolicyDecision
{
    GENERATED_BODY()

    /** Whether execution was permitted */
    UPROPERTY(BlueprintReadOnly, Category = "Policy")
    bool bAllowed = false;

    /** Structured reason codes (not prose) */
    UPROPERTY(BlueprintReadOnly, Category = "Policy")
    TArray<FString> ReasonCodes;

    /** Safe mode was active */
    UPROPERTY(BlueprintReadOnly, Category = "Policy")
    bool bSafeMode = false;

    /** Role that authorized execution */
    UPROPERTY(BlueprintReadOnly, Category = "Policy")
    FString RbacRole;

    /** Individual gate results */
    TMap<FString, bool> GateResults;

    TSharedPtr<FJsonObject> ToJson() const;
    static FTEPPolicyDecision FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * Observations - evidence for delta computation
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FTEPObservations
{
    GENERATED_BODY()

    /** Fingerprint of state before execution */
    UPROPERTY(BlueprintReadOnly, Category = "Observations")
    FString ObsBefore;

    /** Fingerprint of state after execution */
    UPROPERTY(BlueprintReadOnly, Category = "Observations")
    FString ObsAfter;

    /** Computed delta summary (JSON) */
    TSharedPtr<FJsonObject> Delta;

    /** Assets/packages modified */
    UPROPERTY(BlueprintReadOnly, Category = "Observations")
    TArray<FString> ArtifactsTouched;

    /** Visual evidence paths */
    UPROPERTY(BlueprintReadOnly, Category = "Observations")
    TArray<FString> Screenshots;

    TSharedPtr<FJsonObject> ToJson() const;
    static FTEPObservations FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * Outcome - execution result with claims
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FTEPOutcome
{
    GENERATED_BODY()

    /** Final verdict */
    UPROPERTY(BlueprintReadOnly, Category = "Outcome")
    ETEPOutcomeStatus Status = ETEPOutcomeStatus::Error;

    /** Stable error enum (if failed/error) */
    UPROPERTY(BlueprintReadOnly, Category = "Outcome")
    FString ErrorCode;

    /** Bounded error details */
    UPROPERTY(BlueprintReadOnly, Category = "Outcome")
    FString ErrorDetails;

    /** Execution duration in ms */
    UPROPERTY(BlueprintReadOnly, Category = "Outcome")
    float DurationMs = 0.0f;

    /** Claims made by the tool */
    UPROPERTY(BlueprintReadOnly, Category = "Outcome")
    TArray<FToolClaim> Claims;

    TSharedPtr<FJsonObject> ToJson() const;
    static FTEPOutcome FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * Rollback evidence - embedded in TEP (Option A)
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FTEPRollback
{
    GENERATED_BODY()

    /** Rollback outcome */
    UPROPERTY(BlueprintReadOnly, Category = "Rollback")
    ETEPRollbackStatus Status = ETEPRollbackStatus::NotRequired;

    /** Was rollback attempted */
    UPROPERTY(BlueprintReadOnly, Category = "Rollback")
    bool bAttempted = false;

    /** Snapshot used for rollback */
    UPROPERTY(BlueprintReadOnly, Category = "Rollback")
    FString SnapshotId;

    /** State fingerprint after rollback */
    UPROPERTY(BlueprintReadOnly, Category = "Rollback")
    FString ObsAfterRollback;

    /** Delta after rollback (should be empty on success) */
    TSharedPtr<FJsonObject> RollbackDelta;

    /** Rollback duration in ms */
    UPROPERTY(BlueprintReadOnly, Category = "Rollback")
    float DurationMs = 0.0f;

    TSharedPtr<FJsonObject> ToJson() const;
    static FTEPRollback FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * Timestamps
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FTEPTimestamps
{
    GENERATED_BODY()

    /** When proof generation started */
    UPROPERTY(BlueprintReadOnly, Category = "Timestamps")
    FDateTime CreatedAt;

    /** When proof was finalized */
    UPROPERTY(BlueprintReadOnly, Category = "Timestamps")
    FDateTime FinalizedAt;

    TSharedPtr<FJsonObject> ToJson() const;
    static FTEPTimestamps FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * Environment context
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FTEPEnvironment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    FString UEVersion;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    FString ProjectName;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    FString MapName;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    FString GitCommit;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    FString KernelVersion;

    /** Hash of verifier bundle - prevents version mismatch */
    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    FString VerifierHash;

    TSharedPtr<FJsonObject> ToJson() const;
    static FTEPEnvironment FromJson(const TSharedPtr<FJsonObject>& Json);
};

/**
 * Tool Execution Proof (TEP) v1.0.0
 * 
 * The canonical, atomic proof bundle for a single governed mutation attempt.
 * This is the fundamental audit and replay unit for governed execution.
 * 
 * FROZEN SPEC - Breaking changes require version bump.
 * 
 * Key invariants:
 * - One TEP = One tool invocation = One ExecCtx
 * - ExecCtx is embedded (not just referenced)
 * - Rollback evidence is embedded (Option A)
 * - UE is authoritative (Python mirrors/verifies)
 * - Signed TEPs required for training/production
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FToolExecutionProof
{
    GENERATED_BODY()

    // =========================================================================
    // SCHEMA VERSION
    // =========================================================================
    
    /** Schema version - const "1.0.0" */
    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FString SchemaVersion = TEXT("1.0.0");

    // =========================================================================
    // IDENTITY & ORDERING
    // =========================================================================

    /** Unique proof ID: tep_ + 32 hex chars */
    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FString ProofId;

    /** PIE/session instance ID */
    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FString SessionId;

    /** SHA256 of frozen plan JSON */
    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FString PlanId;

    /** Monotonic step within plan (replay protection) */
    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    int32 StepId = 0;

    /** Unique tool invocation ID */
    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FString ToolCallId;

    /** Content hash of previous TEP (hash chain). Empty for first step. */
    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FString PrevProofHash;

    // =========================================================================
    // ACTION
    // =========================================================================

    /** Registered tool name */
    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FString ToolName;

    /** Full tool arguments (for training). Stored as JSON. */
    TSharedPtr<FJsonObject> ToolArgs;

    /** SHA256 of canonical args JSON */
    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FString ToolArgsHash;

    /** Risk tier */
    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    ERiftbornRiskTier RiskTier = ERiftbornRiskTier::Safe;

    // =========================================================================
    // POLICY DECISION
    // =========================================================================

    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FTEPPolicyDecision PolicyDecision;

    // =========================================================================
    // EXEC CTX (EMBEDDED - THE NERVOUS IMPULSE)
    // =========================================================================

    /** Full embedded ExecCtx that authorized this execution */
    FExecCtx ExecCtx;

    // =========================================================================
    // OBSERVATIONS
    // =========================================================================

    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FTEPObservations Observations;

    // =========================================================================
    // OUTCOME
    // =========================================================================

    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FTEPOutcome Outcome;

    // =========================================================================
    // ROLLBACK (EMBEDDED - OPTION A)
    // =========================================================================

    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FTEPRollback Rollback;

    // =========================================================================
    // TIMESTAMPS
    // =========================================================================

    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FTEPTimestamps Timestamps;

    // =========================================================================
    // ENVIRONMENT
    // =========================================================================

    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FTEPEnvironment Environment;

    // =========================================================================
    // INTEGRITY
    // =========================================================================

    /** SHA256 of canonical JSON (excluding signature block) */
    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FString ContentHash;

    /** HMAC-SHA256 signature of ContentHash. Empty if unsigned. */
    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FString Signature;

    /** Was this signed with a real key? */
    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    bool bSigned = false;

    /** First 8 chars of signing key hash (for rotation tracking) */
    UPROPERTY(BlueprintReadOnly, Category = "TEP")
    FString KeyId;

    // =========================================================================
    // METHODS
    // =========================================================================

    /** Generate a new proof ID */
    static FString GenerateProofId();

    /** Compute content hash from all fields (excluding signature block) */
    FString ComputeContentHash() const;

    /** Finalize the proof: compute hash and optionally sign */
    void Finalize(const FString& SigningSecret = TEXT(""));

    /** Verify content hash integrity */
    bool VerifyIntegrity() const;

    /** Verify signature (requires signing secret) */
    bool VerifySignature(const FString& SigningSecret) const;

    /** Validate ExecCtx binding (plan_id, step_id, tool_name, args_hash must match) */
    bool ValidateExecCtxBinding() const;

    /** Full validation: integrity + signature + ExecCtx binding */
    bool Validate(const FString& SigningSecret) const;

    /** Serialize to canonical JSON */
    TSharedPtr<FJsonObject> ToJson() const;

    /** Serialize to canonical JSON string */
    FString ToJsonString() const;

    /** Deserialize from JSON */
    static FToolExecutionProof FromJson(const TSharedPtr<FJsonObject>& Json);

    /** Save to file */
    bool SaveToFile(const FString& Path) const;

    /** Load from file */
    static TOptional<FToolExecutionProof> LoadFromFile(const FString& Path);
};

/**
 * TEP Builder - fluent API for constructing proofs
 */
class RIFTBORNAI_API FTEPBuilder
{
public:
    FTEPBuilder();

    /** Set identity fields */
    FTEPBuilder& SetSessionId(const FString& SessionId);
    FTEPBuilder& SetPlanId(const FString& PlanId);
    FTEPBuilder& SetStepId(int32 StepId);
    FTEPBuilder& SetToolCallId(const FString& ToolCallId);
    FTEPBuilder& SetPrevProofHash(const FString& Hash);

    /** Set action fields */
    FTEPBuilder& SetToolName(const FString& ToolName);
    FTEPBuilder& SetToolArgs(const TSharedPtr<FJsonObject>& Args);
    FTEPBuilder& SetRiskTier(ERiftbornRiskTier Tier);

    /** Set policy decision */
    FTEPBuilder& SetPolicyDecision(const FTEPPolicyDecision& Decision);

    /** Set ExecCtx (the nervous impulse) */
    FTEPBuilder& SetExecCtx(const FExecCtx& Ctx);

    /** Set observations */
    FTEPBuilder& SetObservations(const FTEPObservations& Obs);

    /** Set outcome */
    FTEPBuilder& SetOutcome(const FTEPOutcome& Outcome);

    /** Set rollback */
    FTEPBuilder& SetRollback(const FTEPRollback& Rollback);

    /** Set environment */
    FTEPBuilder& SetEnvironment(const FTEPEnvironment& Env);

    /** Build and finalize (optionally sign) */
    FToolExecutionProof Build(const FString& SigningSecret = TEXT(""));

private:
    FToolExecutionProof Proof;
};
