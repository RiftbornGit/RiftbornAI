// Copyright RiftbornAI. All Rights Reserved.
// AgentEvent.h - Auditable event stream for agent task execution

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"  // For EToolRisk
#include "ProjectAnalyzer.h"  // For FRiftbornFileChange

/**
 * Event types - small, fixed vocabulary of agent actions
 * 
 * Design principle: Every event is an OBSERVABLE ACTION, not model narration.
 * Events reference evidence (diffs, logs, proofs) rather than containing it.
 */
UENUM()
enum class EAgentEventType : uint8
{
	// === Task Lifecycle ===
	TaskStart,          // Task accepted, workspace prepared
	TaskComplete,       // Task finished (success or terminal failure)
	
	// === Information Gathering ===
	Read,               // File/symbol/log opened for inspection
	Search,             // Codebase/asset search performed
	
	// === Planning ===
	PlanGenerated,      // New plan produced (step list + rationale)
	PlanApproved,       // User approved plan execution
	PlanRejected,       // User rejected plan
	
	// === Execution ===
	StepStart,          // Beginning a plan step
	StepComplete,       // Step finished (success/fail)
	ToolComplete,       // Individual tool execution complete (LLM-direct path)
	Edit,               // File modification (patch produced)
	
	// === Build/Run ===
	BuildStart,         // UBT/Live Coding invoked
	BuildComplete,      // Build finished (success/fail + logs)
	PIEStart,           // Play-In-Editor started
	PIEStop,            // Play-In-Editor stopped
	
	// === Verification ===
	ProbeStart,         // Predicate check beginning
	ProbeComplete,      // Predicate result (pass/fail + witness)
	GateResult,         // Aggregate gate result (all predicates)
	
	// === Repair/Recovery ===
	RepairStart,        // Repair iteration beginning
	RepairComplete,     // Repair attempt finished
	Rollback,           // Undo operation performed
	
	// === Control Flow ===
	NeedsInput,         // Blocked, waiting for user decision
	Stop,               // Execution stopped (reason provided)
	Error,              // Error occurred (tool failed, exception, etc.)
	
	// === Evidence ===
	ProofGenerated,     // Proof bundle created (hash reference)
	ArtifactCaptured    // Screenshot/log/evidence captured
};

/**
 * Stop reasons - why execution halted
 */
UENUM()
enum class EAgentStopReason : uint8
{
	None,               // Not stopped
	Success,            // All acceptance predicates passed
	MaxIterations,      // Hit repair iteration limit
	MaxTime,            // Hit time budget
	MaxBuildFails,      // Too many build failures
	RepeatedFailure,    // Same failure signature repeated
	RiskEscalation,     // Would need higher risk tier
	UserCancelled,      // User requested stop
	EnvironmentUnhealthy,// PIE/tick/bridge issue
	PredicateFailed,    // Critical predicate cannot be satisfied
	Blocked             // Cannot proceed (missing dependency, etc.)
};

// ============================================================================
// PREDICATE ENFORCEMENT (Priority 3 - 2026-01-31)
// ============================================================================

/**
 * Predicate determinism classification
 * Affects how the predicate can be used in PROOF mode
 */
enum class EPredicateDeterminism : uint8
{
	/** Same inputs always produce same outputs */
	Deterministic,
	
	/** Outputs may vary within known bounds (e.g., timing-dependent) */
	BoundedNondeterministic,
	
	/** Outputs can vary arbitrarily - DISCOURAGED in PROOF mode */
	Nondeterministic
};

/**
 * Repairability classification for predicate failures
 * Determines whether the repair gate should attempt repairs
 */
UENUM(BlueprintType)
enum class ERepairability : uint8
{
	/** Unknown - legacy fallback, treat as potentially repairable */
	Unknown,
	
	/** Repairable - a repair action may fix this (e.g., compile errors) */
	Repairable,
	
	/** NotRepairable - no repair possible (e.g., asset not found) */
	NotRepairable,
	
	/** Infrastructure - environment issue, not a repair target (e.g., bridge down) */
	Infrastructure,
};

/**
 * Predicate metadata - single source of truth for predicate enforcement
 * 
 * Each whitelisted predicate MUST have explicit metadata.
 * No defaults in PROOF mode - forces intentional configuration.
 */
struct RIFTBORNAI_API FPredicateMeta
{
	/** Predicate name (must match validator function name) */
	FString Name;
	
	/** Maximum execution time in milliseconds */
	int32 TimeoutMs = 250;
	
	/** Predicate must not modify any state */
	bool bPure = true;
	
	/** Determinism classification */
	EPredicateDeterminism Determinism = EPredicateDeterminism::Deterministic;
	
	/** Must run on game thread (most UE checks do) */
	bool bRequiresGameThread = true;
	
	/** Human-readable description */
	FString Description;
	
	/** Is this predicate explicitly configured (not default)? */
	bool bExplicitlyConfigured = false;
};

/**
 * Evidence of impurity detected during predicate evaluation
 */
struct RIFTBORNAI_API FImpurityEvidence
{
	/** Objects that were modified */
	TArray<FString> ModifiedObjects;
	
	/** Packages that became dirty */
	TArray<FString> DirtiedPackages;
	
	/** Actors that were spawned/despawned */
	TArray<FString> ActorChanges;
	
	/** Transactions that occurred */
	int32 TransactionsDelta = 0;
	
	/** Any other detected side effects */
	TArray<FString> OtherEffects;
	
	bool HasEvidence() const
	{
		return ModifiedObjects.Num() > 0 || DirtiedPackages.Num() > 0 ||
		       ActorChanges.Num() > 0 || TransactionsDelta != 0 ||
		       OtherEffects.Num() > 0;
	}
	
	FString GetSummary() const;
};

/**
 * Gate result for a single predicate
 * 
 * ENHANCED (2026-01-31): Now includes timeout/impurity evidence
 * for PROOF mode enforcement
 */
struct RIFTBORNAI_API FPredicateResult
{
	// === Identity ===
	FString PredicateName;
	
	// === Outcome ===
	bool bPassed = false;
	FString Message;        // Why it failed (if failed)
	
	// === Failure Classification (NEW 2026-02-01) ===
	FString ReasonCode;     // Machine-readable code (e.g., "BP_NOT_FOUND", "BP_COMPILE_ERRORS")
	ERepairability Repairability = ERepairability::Unknown;  // Can this be repaired?
	FString RepairAction;   // Suggested repair action (e.g., "repair_missing_parent")
	
	// === Evidence ===
	FString WitnessHash;    // Hash of evidence
	TMap<FString, FString> Inputs;   // Canonicalized inputs
	TMap<FString, FString> Outputs;  // Structured outputs
	
	// === Timing (PROOF enforcement) ===
	float EvaluationTimeMs = 0.0f;
	int32 TimeoutMs = 0;           // Configured timeout
	bool bTimedOut = false;        // Exceeded timeout
	
	// === Purity (PROOF enforcement) ===
	bool bPureExpected = true;     // Was purity required?
	bool bImpureDetected = false;  // Did we detect side effects?
	FImpurityEvidence ImpurityEvidence;
	
	// === Determinism ===
	EPredicateDeterminism DeterminismClass = EPredicateDeterminism::Deterministic;
	
	// === Governance ===
	bool bTaintedSession = false;  // Did this evaluation taint the session?
	FString TaintReason;           // Why the session was tainted
	
	/** Check if this result is usable (passed without tainting) */
	bool IsClean() const
	{
		return bPassed && !bTimedOut && !bImpureDetected && !bTaintedSession;
	}
	
	/** Check if this result taints the session */
	bool TaintsSession() const
	{
		return bTimedOut || bImpureDetected;
	}
};

/**
 * File change summary (part of Edit events)
 */
struct RIFTBORNAI_API FFileChange
{
	FString FilePath;
	int32 LinesAdded = 0;
	int32 LinesRemoved = 0;
	FString PatchRef;       // Path to unified diff or patch ID
};

/**
 * FAgentEvent - Single auditable action in the agent's execution
 * 
 * Design principles:
 * - Events are IMMUTABLE after creation
 * - Events REFERENCE artifacts (don't embed them)
 * - Every event is human-auditable
 * - Events enable REPLAY of task execution
 */
struct RIFTBORNAI_API FAgentEvent
{
	// === Identity ===
	FGuid EventId;              // Unique event ID
	FDateTime Timestamp;        // When this happened (UTC)
	
	// === Context ===
	FGuid TaskId;               // Which task this belongs to
	FGuid StepId;               // Which step (if applicable)
	int32 Iteration = 0;        // Repair iteration (0 = first attempt)
	
	// === Event Data ===
	EAgentEventType Type = EAgentEventType::TaskStart;
	FString Summary;            // One-sentence human description
	
	// === Inputs (what triggered this) ===
	TArray<FString> InputPaths; // Files/assets involved
	FString ToolName;           // Tool being used (if applicable)
	TMap<FString, FString> ToolArgs; // Tool arguments
	
	// === Outputs (what this produced) ===
	bool bSuccess = true;
	FString ResultSummary;      // One-line result
	TArray<FRiftbornFileChange> FilesChanged;
	FString DiffRef;            // Path to unified diff (for Edit events)
	FString LogRef;             // Path to log file (for Build/PIE events)
	
	// === Artifact Deltas (human-legible evidence) ===
	TArray<FString> AssetsCreated;
	TArray<FString> AssetsModified;
	TArray<FString> AssetsDeleted;
	TArray<FString> ActorsSpawned;
	TArray<FString> ActorsModified;
	TArray<FString> ActorsDeleted;
	TArray<FString> BlueprintsCompiled;
	TArray<FString> PredicatesRun;
	
	// === Verification ===
	TArray<FPredicateResult> PredicateResults;  // Gate results
	FString ProofBundleRef;     // Path to proof bundle (for ProofGenerated)
	FString EvidenceHash;       // Hash of all evidence for this event
	
	// === Risk ===
	EToolRisk RiskTier = EToolRisk::Safe;
	bool bNeedsApproval = false;
	
	// === Stop Info (only for Stop events) ===
	EAgentStopReason StopReason = EAgentStopReason::None;
	FString StopDiagnosis;      // Detailed stop explanation
	
	// === Replay Support ===
	FString ReproCommand;       // Command to reproduce this action
	FString EnvironmentHash;    // Hash of environment state
	uint32 RandomSeed = 0;      // Seed used (for determinism)
	
	// === Timing ===
	float DurationMs = 0.0f;    // How long this took
	float ExecutionTimeMs = 0.0f; // Alias for DurationMs (for compatibility)
	
	FAgentEvent()
	{
		EventId = FGuid::NewGuid();
		Timestamp = FDateTime::UtcNow();
	}
	
	FAgentEvent(EAgentEventType InType, const FString& InSummary)
		: Type(InType), Summary(InSummary)
	{
		EventId = FGuid::NewGuid();
		Timestamp = FDateTime::UtcNow();
	}
	
	// === Serialization ===
	FString ToJsonLine() const;
	static TOptional<FAgentEvent> FromJsonLine(const FString& JsonLine);
};

/**
 * Task specification - what the agent should accomplish
 * 
 * Input contract for Agent Task Runner.
 */
struct RIFTBORNAI_API FAgentTaskSpec
{
	// === Identity ===
	FGuid TaskId;
	FDateTime CreatedAt;
	
	// === Goal ===
	FString Goal;               // Plain-English objective
	FString IntentHash;         // Hash of parsed intent
	
	// === Acceptance Criteria ===
	TArray<FString> AcceptancePredicates;  // Must all pass for success
	TArray<FString> WarningPredicates;     // Should pass, warn if not
	
	// === Constraints ===
	EToolRisk MaxRiskTier = EToolRisk::Mutation;  // Ceiling without approval
	TArray<FString> ToolAllowlist;  // Empty = all allowed
	TArray<FString> ToolDenylist;   // Explicit blocks
	int32 MaxFilesTouched = 20;     // Scope limit
	TArray<FString> PathAllowlist;  // Paths agent can modify
	
	// === Budget ===
	int32 MaxIterations = 3;        // Repair attempts
	int32 MaxBuildAttempts = 5;     // Build failures before stop
	float MaxMinutes = 30.0f;       // Time budget
	
	// === Workspace ===
	FString WorkspacePath;          // Root of UE project
	FString PluginPath;             // RiftbornAI plugin path
	FString BranchBase;             // Git branch to base off (optional)
	bool bUseWorktree = false;      // Create isolated worktree
	
	// === Environment Requirements ===
	bool bRequiresPIE = false;      // Task needs runtime validation
	bool bRequiresBuild = true;     // Task needs compilation
	
	FAgentTaskSpec()
	{
		TaskId = FGuid::NewGuid();
		CreatedAt = FDateTime::UtcNow();
	}
	
	// Validation
	bool IsValid(FString& OutError) const;
	
	// Serialization
	FString ToJson() const;
	static TOptional<FAgentTaskSpec> FromJson(const FString& Json);
};

/**
 * Task result - outcome of agent task execution
 * 
 * Output contract from Agent Task Runner.
 */
struct RIFTBORNAI_API FAgentTaskResult
{
	// === Identity ===
	FGuid TaskId;
	FGuid ResultId;
	FDateTime CompletedAt;
	
	// === Outcome ===
	bool bSuccess = false;
	EAgentStopReason StopReason = EAgentStopReason::None;
	FString Diagnosis;          // Human-readable summary
	
	// === Metrics ===
	int32 IterationsUsed = 0;
	int32 StepsExecuted = 0;
	int32 BuildsAttempted = 0;
	float TotalTimeMinutes = 0.0f;
	
	// === Changes ===
	TArray<FRiftbornFileChange> AllFilesChanged;
	FString FinalDiffRef;       // Consolidated diff
	
	// === Verification ===
	TArray<FPredicateResult> FinalPredicateResults;
	FString ProofBundleRef;     // Final proof bundle path
	
	// === Event Log ===
	FString EventLogPath;       // Path to events.jsonl
	int32 EventCount = 0;
	
	// === Replay ===
	FString ReplayScriptPath;   // replay.ps1 or replay.sh
	
	FAgentTaskResult()
	{
		ResultId = FGuid::NewGuid();
		CompletedAt = FDateTime::UtcNow();
	}
	
	// Serialization
	FString ToJson() const;
	static TOptional<FAgentTaskResult> FromJson(const FString& Json);
};
