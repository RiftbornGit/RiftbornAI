// Copyright RiftbornAI. All Rights Reserved.
// Tool Contract - Data-driven policy from contracts.json
// 
// CRITICAL: This is the SINGLE SOURCE OF TRUTH for tool policy.
// Both C++ and Python read from the same contracts.json.
// Do NOT duplicate policy logic in code.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "ToolContract.generated.h"

// Test hook gate: compile out in non-editor / non-dev builds
#ifndef RIFTBORN_TEST_HOOKS_ENABLED
#if WITH_EDITOR && (UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG) && defined(RIFTBORN_TEST_HOOKS)
#define RIFTBORN_TEST_HOOKS_ENABLED 1
#else
#define RIFTBORN_TEST_HOOKS_ENABLED 0
#endif
#endif

/**
 * Risk tier from contract - maps to Python RISK_TIERS
 */
UENUM(BlueprintType)
enum class EContractRiskTier : uint8
{
	Unknown,             // Not yet classified
	Safe,                // Read-only, no state change
	Verification,        // Read-only verification/checking
	MutatingReversible,  // State change, can undo
	MutatingIrreversible,// State change, cannot undo
	Dangerous,           // Could break project
	Destructive          // ALWAYS_CONFIRM
};

/**
 * What to do when a precondition fails
 */
UENUM(BlueprintType)
enum class EPreconditionAction : uint8
{
	Block,    // Stop execution
	Repair,   // Auto-run repair tool
	Ask,      // Escalate to user
	Warn      // Log and continue
};

/**
 * Execution lane - where the tool is allowed to run
 * PROOF mode enforces this strictly
 */
UENUM(BlueprintType)
enum class EExecutionLane : uint8
{
	CppOnly,      // Only C++ typed tools allowed
	PythonDev,    // Python allowed in DEV mode only
	PythonAlways  // Python allowed always (NEVER in PROOF mode contracts)
};

/**
 * Determinism class - how predictable the tool is
 */
UENUM(BlueprintType)
enum class EDeterminismClass : uint8
{
	Deterministic,       // Same inputs → same outputs
	BoundedNondeterminism, // Outputs vary but within bounds (e.g., spawn location)
	Nondeterministic     // Outputs may vary arbitrarily (e.g., network calls)
};

/**
 * Evidence binding - what a tool produces or consumes
 * This is the CORE of verifiable chaining
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FEvidenceBinding
{
	GENERATED_BODY()
	
	/** Binding key (e.g., "actor_guid", "asset_path", "blueprint_path") */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Key;
	
	/** Expected type (guid, path, name, int, bool, json) */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Type;
	
	/** Human-readable description */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Description;
	
	/** Is this binding required (for produces) or optional */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bRequired = true;
};

/**
 * Single precondition from contract
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FToolPrecondition
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ProbeName;       // e.g., "pie_state", "asset_exists"
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Predicate;       // Predicate expression (for compatibility)
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TMap<FString, FString> ProbeArgs;  // Arguments to pass to probe
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ExpectedJson;    // Expected result (JSON string)
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	EPreconditionAction OnFail = EPreconditionAction::Block;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString RepairTool;      // Tool to run if OnFail == Repair
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString SkipCondition;   // Expression to skip this precondition
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Reason;          // Human-readable failure reason
};

/**
 * Single postcondition probe from contract.
 *
 * Evaluated AFTER the tool dispatches successfully (see
 * FAgenticLoopRunner::FinalizeSingleToolExecution). Each postcondition
 * names a probe (e.g. `actor_exists`, `actor_at_location`,
 * `viewport_mode_equals`) that lives in PredicateEvaluator's whitelisted
 * registry, plus the args to pass and the expected boolean outcome.
 *
 * A failing postcondition doesn't fail the tool call itself — it's
 * surfaced into FClaudeToolResult.Metadata as structured `postcondition_*`
 * entries so the agent + brain layer can detect "succeeded but wrong"
 * outcomes. This is the key signal the audit flagged as missing.
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FToolPostcondition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ProbeName;       // e.g. "actor_exists", "viewport_mode_equals"

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TMap<FString, FString> ProbeArgs;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bExpected = true;   // Default: probe must PASS for the tool to have done its job

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Note;            // Contract-author reason / context
};

/**
 * Failure mode and recovery from contract
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FToolFailureMode
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ErrorCode;       // e.g., "actor_not_found"
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Description;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString RecoveryTool;    // Tool to run for recovery (empty if none)
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bEscalate = false;  // If true, ask user
};

/**
 * Recovery chain step from contract
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRecoveryChainStep
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString OnError;         // Error code that triggers this
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> TryTools;// Tools to try in order
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	int32 MaxDepth = 2;      // Max chain depth
};

/**
 * Full tool contract loaded from contracts.json
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FToolContract
{
	GENERATED_BODY()
	
	// Identity
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ToolName;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString DisplayName;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Description;
	
	// Classification
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	EContractRiskTier RiskTier = EContractRiskTier::Safe;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString Category;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	EToolCost Cost = EToolCost::Cheap;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	EToolVisibility Visibility = EToolVisibility::Public;
	
	// Schema
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	FString ArgsSchemaJson;  // Full JSON schema for validation
	
	// =========================================================================
	// EVIDENCE CONTRACTS (PROOF mode enforcement)
	// =========================================================================
	// These fields define what the tool MUST produce and CAN consume.
	// In PROOF mode, success without required evidence = FAILURE.
	
	/** What this tool produces (output bindings available for chaining) */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI|Evidence")
	TArray<FEvidenceBinding> Produces;
	
	/** What this tool consumes (input bindings it expects from prior steps) */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI|Evidence")
	TArray<FEvidenceBinding> Consumes;
	
	/** Required witness keys that MUST appear in result for success */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI|Evidence")
	TArray<FString> RequiredWitness;
	
	/** Execution lane (C++ only, Python dev, etc.) */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI|Execution")
	EExecutionLane ExecutionLane = EExecutionLane::CppOnly;
	
	/** Determinism class */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI|Execution")
	EDeterminismClass Determinism = EDeterminismClass::Deterministic;
	
	/** Timeout in milliseconds (0 = default 30000) */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI|Execution")
	int32 TimeoutMs = 0;
	
	/** Declared side effects (files, assets, level, etc.) */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI|Execution")
	TArray<FString> SideEffects;
	
	// =========================================================================
	// Governance
	// =========================================================================
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FToolPrecondition> Preconditions;

	/** Postcondition probes evaluated after the tool dispatches. Drives
	 *  the agent's "succeeded-but-wrong" detection via surprise scoring.
	 *  Parsed from contracts.json `postconditions` array. */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FToolPostcondition> Postconditions;

	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> WitnessProbes;  // Probes to capture evidence
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TMap<FString, FToolFailureMode> FailureModes;
	
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FRecoveryChainStep> RecoveryChain;
	
	// =========================================================================
	// Legacy/Compatibility fields (used by some code paths)
	// =========================================================================
	
	/** Whether this tool supports undo operations */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bUndoSupported = false;
	
	/** Whether this tool is mutating (modifies state) */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bIsMutating = false;
	
	/** Whether this tool requires proof mode */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bRequiresProofMode = false;
	
	/** Acceptance criteria for success (probe names) */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> AcceptanceCriteria;
	
	// Helpers
	bool IsReadOnlyByRiskTier() const
	{
		return RiskTier == EContractRiskTier::Safe || RiskTier == EContractRiskTier::Verification;
	}

	bool IsMutatingByRiskTier() const
	{
		switch (RiskTier)
		{
			case EContractRiskTier::MutatingReversible:
			case EContractRiskTier::MutatingIrreversible:
			case EContractRiskTier::Dangerous:
			case EContractRiskTier::Destructive:
				return true;
			default:
				return false;
		}
	}

	bool RequiresConfirmation() const 
	{ 
		return RiskTier == EContractRiskTier::Dangerous || 
		       RiskTier == EContractRiskTier::Destructive; 
	}
	
	bool CanAutoRepair() const
	{
		for (const FToolPrecondition& Pre : Preconditions)
		{
			if (Pre.OnFail == EPreconditionAction::Repair && !Pre.RepairTool.IsEmpty())
			{
				return true;
			}
		}
		return false;
	}
	
	/** Check if a result witness contains all required evidence keys */
	bool ValidateWitness(const TMap<FString, FString>& Witness, TArray<FString>& OutMissing) const
	{
		OutMissing.Empty();
		for (const FString& RequiredKey : RequiredWitness)
		{
			if (!Witness.Contains(RequiredKey))
			{
				OutMissing.Add(RequiredKey);
			}
		}
		return OutMissing.Num() == 0;
	}
	
	/** Check if all required produces bindings are present in witness */
	bool ValidateProducedEvidence(const TMap<FString, FString>& Witness, TArray<FString>& OutMissing) const
	{
		OutMissing.Empty();
		for (const FEvidenceBinding& Binding : Produces)
		{
			if (Binding.bRequired && !Witness.Contains(Binding.Key))
			{
				OutMissing.Add(Binding.Key);
			}
		}
		return OutMissing.Num() == 0;
	}
	
	/** Get effective timeout (default 30000ms if not specified) */
	int32 GetEffectiveTimeoutMs() const
	{
		return TimeoutMs > 0 ? TimeoutMs : 30000;
	}
	
	// Convert contract risk tier to tool registry risk
	EToolRisk ToToolRisk() const
	{
		switch (RiskTier)
		{
			case EContractRiskTier::Safe: return EToolRisk::Safe;
			case EContractRiskTier::Verification: return EToolRisk::Safe;
			case EContractRiskTier::MutatingReversible: return EToolRisk::Mutation;
			case EContractRiskTier::MutatingIrreversible: return EToolRisk::Mutation;
			case EContractRiskTier::Dangerous: return EToolRisk::Dangerous;
			case EContractRiskTier::Destructive: return EToolRisk::Destructive;
			default: return EToolRisk::Unknown;
		}
	}
	
	/** PROOF mode requires undo capability declaration */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bCanUndo = false;
	
	/** PROOF mode requires postcondition probes */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> PostconditionProbes;
	
	/** Allowed predicate validators for this tool */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> AllowedPredicates;
	
	/** Evidence bindings for chaining (outputs) */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	TArray<FString> EvidenceBindings;
	
	/** Execution lane: C++ only in PROOF mode */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bRequiresCppExecution = false;
	
	// =========================================================================
	// Parse-time error tracking (2026-02-01)
	// Unknown enum values are recorded here during parsing, then caught in validation
	// This ensures rejection produces proper proof artifacts at execution time
	// =========================================================================
	
	/** Parse errors encountered during contract loading */
	TArray<FString> ParseErrors;
	
	/** Check if contract had parse errors (unknown enums, etc.) */
	bool HasParseErrors() const { return ParseErrors.Num() > 0; }
	
	// =========================================================================
	// Validation state (2026-02-01)
	// Contracts MUST be validated before execution can proceed
	// In PROOF mode, unvalidated contracts are execution-blocked
	// =========================================================================
	
	/** Whether ValidateContract() has been called on this contract */
	bool bValidated = false;
	
	/** Validation errors (populated by ValidateContract()) */
	TArray<FString> ValidationErrors;
	
	/** Whether this contract is certified (no parse errors, no validation errors) */
	bool IsCertified() const { return bValidated && ParseErrors.Num() == 0 && ValidationErrors.Num() == 0; }
	
	/** DEV mode: contract has errors but can execute in UNCERTIFIED lane */
	bool bUncertified = false;
	
	// =========================================================================
	// PROOF MODE BLOCKING (2026-02-02)
	// Tools marked blocked_in_proof_mode MUST be denied at the central choke point
	// This is NOT optional - it's enforced in GetExecutionDecision()
	// =========================================================================
	
	/** Tool is BLOCKED in PROOF mode - cannot execute regardless of other checks */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bBlockedInProofMode = false;
	
	/** Tool is lab-only - allowed ONLY when RIFTBORN_LAB_MODE=1 */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bLabOnly = false;
	
	/** Tool requires explicit confirmation even in DEV mode */
	UPROPERTY(BlueprintReadOnly, Category = "RiftbornAI")
	bool bRequiresConfirmation = false;
};

/**
 * Structured parse error with field location
 * Helps debugging when contracts.json grows large
 */
struct RIFTBORNAI_API FContractParseError
{
	FString ToolName;       // Which tool's contract
	FString Field;          // Which field had the error (e.g., "classification.risk_tier")
	FString Value;          // The invalid value found
	FString Message;        // Human-readable error message
	
	FString ToString() const
	{
		return FString::Printf(TEXT("[%s] %s: '%s' - %s"), *ToolName, *Field, *Value, *Message);
	}
};

/**
 * Execution decision - centralized authority result
 * CRITICAL: This is the SINGLE source of truth for whether a tool can execute
 * All denials in PROOF mode MUST be emitted as rejection proofs
 */
UENUM(BlueprintType)
enum class EContractDecision : uint8
{
	Allowed,               // Execution permitted
	Denied_NoContract,     // PROOF mode: no contract exists
	Denied_NotValidated,   // Contract exists but validation never ran
	Denied_ValidationFailed, // Contract failed validation
	Denied_ParseErrors,    // Contract had parse errors (unknown enums, etc.)
	Denied_SessionTainted, // Session is tainted - all execution blocked
	Denied_Uncertified,    // DEV mode can override, PROOF mode blocks
	Denied_BlockedInProofMode, // Tool is explicitly blocked in PROOF mode (2026-02-02)
	Denied_LabOnlyOutsideLab,  // Tool requires LAB mode but LAB is not enabled (2026-02-02)
};

/**
 * Step-level execution context for rejection proofs (2026-02-01)
 * This ensures proofs can be correlated to exact plan steps
 */
struct RIFTBORNAI_API FStepExecutionContext
{
	int32 StepIndex = -1;              // Which step in the plan (0-based)
	FString ToolUseId;                  // Unique ID for this tool invocation
	FString StepArgsHash;               // SHA256 of canonicalized args JSON
	int32 PlanSequenceId = 0;           // Monotonic sequence for the plan run
	FString EditorSessionId;            // Session identifier
	bool bProofModeEnabled = false;     // Was PROOF mode on?
	bool bSessionTainted = false;       // Was session tainted?
	
	/** Build from plan step data */
	static FStepExecutionContext FromStep(int32 InStepIndex, const FString& InToolUseId, 
		const TMap<FString, FString>& Args, int32 InSequenceId);
	
	/** Compute canonical args hash */
	static FString ComputeArgsHash(const TMap<FString, FString>& Args);
};

struct RIFTBORNAI_API FContractExecutionDecision
{
	EContractDecision Decision = EContractDecision::Denied_NoContract;
	FString ToolName;
	TArray<FString> Errors;   // Why execution was denied
	bool bProofRequired = false; // If true, caller MUST emit rejection proof
	
	// Step context for detailed proofs (populated by caller before EmitRejectionProof)
	FStepExecutionContext StepContext;
	
	// Proof tracking (set after EmitRejectionProof is called)
	FString ProofBundleId;     // Non-empty if proof was emitted
	
	bool IsAllowed() const { return Decision == EContractDecision::Allowed; }
	bool IsDenied() const { return Decision != EContractDecision::Allowed; }
	bool HasProof() const { return !ProofBundleId.IsEmpty(); }
	
	/** Get reason code string for proof bundles */
	FString GetReasonCode() const
	{
		switch (Decision)
		{
			case EContractDecision::Allowed: return TEXT("ALLOWED");
			case EContractDecision::Denied_NoContract: return TEXT("NO_CONTRACT");
			case EContractDecision::Denied_NotValidated: return TEXT("NOT_VALIDATED");
			case EContractDecision::Denied_ValidationFailed: return TEXT("VALIDATION_FAILED");
			case EContractDecision::Denied_ParseErrors: return TEXT("PARSE_ERRORS");
			case EContractDecision::Denied_SessionTainted: return TEXT("SESSION_TAINTED");
			case EContractDecision::Denied_Uncertified: return TEXT("UNCERTIFIED");
			case EContractDecision::Denied_BlockedInProofMode: return TEXT("BLOCKED_IN_PROOF_MODE");
			case EContractDecision::Denied_LabOnlyOutsideLab: return TEXT("LAB_ONLY_OUTSIDE_LAB");
			default: return TEXT("UNKNOWN");
		}
	}
	
	/** Build human-readable summary */
	FString GetSummary() const
	{
		if (IsAllowed()) return FString::Printf(TEXT("Tool '%s' execution ALLOWED"), *ToolName);
		return FString::Printf(TEXT("Tool '%s' execution DENIED [%s]: %s"), 
			*ToolName, *GetReasonCode(), 
			Errors.Num() > 0 ? *Errors[0] : TEXT("no details"));
	}
};

/**
 * Validation result for a single contract
 */
struct RIFTBORNAI_API FContractValidationResult
{
	FString ToolName;
	bool bValid = false;
	TArray<FString> Errors;      // Fatal: blocks execution
	TArray<FString> Warnings;    // Non-fatal in DEV mode
	
	bool HasErrors() const { return Errors.Num() > 0; }
	bool HasWarnings() const { return Warnings.Num() > 0; }
	bool IsValid() const { return bValid && !HasErrors(); }
	FString GetSummary() const;
};

/**
 * Validation result for entire contracts.json
 */
struct RIFTBORNAI_API FContractsValidationResult
{
	bool bValid = false;
	int32 SchemaVersion = 0;
	int32 ContractVersion = 0;
	TArray<FContractValidationResult> ToolResults;
	TArray<FString> GlobalErrors;
	TArray<FString> GlobalWarnings;
	
	int32 GetValidCount() const
	{
		int32 Count = 0;
		for (const auto& R : ToolResults) if (R.bValid) Count++;
		return Count;
	}
	
	int32 GetInvalidCount() const
	{
		return ToolResults.Num() - GetValidCount();
	}
};

// Expected schema version - must match contracts.json
static constexpr int32 EXPECTED_CONTRACT_SCHEMA_VERSION = 1;

// Required fields for PROOF mode contracts
static const TArray<FString> PROOF_MODE_REQUIRED_FIELDS = {
	TEXT("tool_name"),
	TEXT("classification"),
	TEXT("args_schema"),
};

/**
 * Contract Registry - Singleton that loads and caches contracts.json
 * 
 * USAGE:
 *   FToolContractRegistry& Registry = FToolContractRegistry::Get();
 *   const FToolContract* Contract = Registry.GetContract("spawn_actor");
 *   if (Contract) { ... use preconditions, failure modes, etc ... }
 * 
 * The registry auto-reloads on file change if bWatchForChanges is true.
 */
class RIFTBORNAI_API FToolContractRegistry
{
public:
	~FToolContractRegistry() { StopWatching(); }

	static FToolContractRegistry& Get();
	
	/**
	 * Load contracts from JSON file
	 * @param FilePath - Path to contracts.json (uses default if empty)
	 * @return true if loaded successfully
	 */
	bool LoadContracts(const FString& FilePath = TEXT(""));
	
	/**
	 * Reload contracts from disk
	 */
	bool ReloadContracts();
	
	/**
	 * Get contract for a tool
	 * @return nullptr if tool not in contracts
	 */
	const FToolContract* GetContract(const FString& ToolName) const;
	
	/**
	 * Get all tool names with contracts
	 */
	TArray<FString> GetContractedTools() const;
	
	/**
	 * Check if a tool has a contract
	 */
	bool HasContract(const FString& ToolName) const;
	
	/**
	 * Check if a tool is read-only (non-mutating)
	 * A tool is read-only if:
	 * - It has a contract with a read-only risk tier (Safe or Verification)
	 * Tools without contracts are NOT considered read-only (default deny)
	 */
	bool IsReadOnly(const FString& ToolName) const
	{
		const FToolContract* Contract = GetContract(ToolName);
		if (!Contract) return false;  // No contract = not read-only (default deny)
		return Contract->IsReadOnlyByRiskTier();
	}
	
	/**
	 * Get the contract file path
	 */
	FString GetContractFilePath() const { return ContractFilePath; }
	
	/**
	 * Get contract version
	 */
	int32 GetContractVersion() const { return ContractVersion; }
	
	/**
	 * Get schema version string
	 */
	FString GetSchemaVersion() const { return SchemaVersion; }
	
	/**
	 * Check if contracts are loaded
	 */
	bool IsLoaded() const { return bLoaded; }
	
	/**
	 * Enable/disable file watching
	 */
	void SetWatchForChanges(bool bWatch);
	
	/**
	 * Check if PROOF mode is enabled (contracts mandatory)
	 */
	static bool IsProofModeEnabled();

#if RIFTBORN_TEST_HOOKS_ENABLED
	/**
	 * TEST ONLY: Force PROOF mode on/off (overrides env/CLI).
	 * Returns the previous mode value.
	 */
	static bool SetProofModeEnabledForTests(bool bEnabled);
#endif
	
	/**
	 * Check if a tool can be executed given current mode
	 * In PROOF mode: requires contract
	 * In DEV mode: always true (but logs warning if no contract)
	 * @param ToolName - Tool to check
	 * @param OutReason - Reason if execution blocked
	 * @return true if tool can be executed
	 * @deprecated Use GetExecutionDecision() for structured results with proof support
	 */
	bool CanExecuteTool(const FString& ToolName, FString& OutReason) const;
	
	/**
	 * Get structured execution decision for a tool (2026-02-01)
	 * This is the SINGLE AUTHORITY for execution permission.
	 * 
	 * CRITICAL: In PROOF mode, if decision.bProofRequired is true,
	 * the caller MUST emit a rejection proof via EmitRejectionProof().
	 * 
	 * @param ToolName - Tool to check
	 * @return Decision with reason code, errors, and proof requirement flag
	 */
	FContractExecutionDecision GetExecutionDecision(const FString& ToolName) const;
	
	/**
	 * Emit a rejection proof bundle for a denied execution decision
	 * MUST be called for every denial in PROOF mode where decision.bProofRequired is true
	 * 
	 * @param Decision - The denial decision (must be IsDenied())
	 * @param PlanHash - Hash of the plan that tried to execute (if available)
	 * @param PlanHashKind - "canonical", "raw_fallback", or "canonicalization_failed"
	 * @param CanonicalizationError - Error string if canonicalization failed
	 * @return Path to the emitted proof bundle, or empty string on failure
	 */
	FString EmitRejectionProof(
		const FContractExecutionDecision& Decision,
		const FString& PlanHash = TEXT(""),
		const FString& PlanHashKind = TEXT(""),
		const FString& CanonicalizationError = TEXT("")
	) const;
	
	/**
	 * Validate all loaded contracts against schema requirements
	 * In PROOF mode: hard fails on any validation error
	 * @return Validation result with per-tool and global errors
	 */
	FContractsValidationResult ValidateContracts() const;
	
	/**
	 * Validate a single contract and update its validation state
	 * After calling this, Contract.bValidated will be true (even if validation failed)
	 * and Contract.ValidationErrors will contain any errors found.
	 * 
	 * @param Contract - Contract to validate (will be modified)
	 * @param bStrictMode - If true (PROOF mode), enforce all requirements
	 * @return Validation result for this contract
	 */
	FContractValidationResult ValidateContract(FToolContract& Contract, bool bStrictMode) const;
	
	/**
	 * Validate a single contract (const version for compatibility)
	 * NOTE: This version cannot update the contract's validation state.
	 * Prefer the non-const version when possible.
	 */
	FContractValidationResult ValidateContract(const FToolContract& Contract, bool bStrictMode) const;
	
	/**
	 * Get validation result (cached from last load)
	 */
	const FContractsValidationResult& GetValidationResult() const { return ValidationResult; }
	
	/**
	 * Check if contracts passed validation
	 */
	bool IsValidated() const { return ValidationResult.bValid; }
	
	/**
	 * Compute SHA256 hash of the entire loaded contract registry
	 * This is included in proof bundles to bind proofs to specific governance configuration
	 * @return Hex-encoded SHA256 hash
	 */
	FString ComputeRegistryHash() const;
	
	/**
	 * Get the cached registry hash (computed at load time)
	 */
	FString GetRegistryHash() const { return RegistryHash; }
	
	/**
	 * Validate all contracts (alias for ValidateContracts)
	 */
	FContractValidationResult ValidateAllContracts() const;
	
	/**
	 * Validate a single contract (no strict mode parameter)
	 */
	FContractValidationResult ValidateContract(const FToolContract& Contract) const;

private:
	FToolContractRegistry();
	
	bool ParseContractsJson(const FString& JsonContent);
	bool ParseToolContract(const TSharedPtr<FJsonObject>& ToolObj, FToolContract& OutContract);
	EContractRiskTier ParseRiskTier(const FString& TierStr);
	EPreconditionAction ParsePreconditionAction(const FString& ActionStr);
	
	/** Validate contracts at load time - called automatically by LoadContracts */
	bool ValidateContractsOnLoad();
	
	TMap<FString, FToolContract> Contracts;
	FString ContractFilePath;
	FString SchemaVersion;
	int32 ContractVersion = 0;
	bool bLoaded = false;
	bool bWatchForChanges = false;
	FDateTime LastLoadTime;
	FContractsValidationResult ValidationResult;

	/** Cached registry hash computed at load time */
	FString RegistryHash;

	/** Directory watcher handle for auto-reload */
	FDelegateHandle DirectoryWatcherHandle;

	/** Tear down an active file watcher */
	void StopWatching();
};
