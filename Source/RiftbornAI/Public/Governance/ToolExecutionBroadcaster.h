// Copyright RiftbornAI. All Rights Reserved.
// ToolExecutionBroadcaster.h - Singleton for broadcasting tool execution events
//
// PURPOSE:
// The C++ choke point (FClaudeToolRegistry::ExecuteTool) needs to emit events
// to the agent timeline UI. However, the choke point has no direct access to
// FAgentEventStream or FAgentTaskRunner.
//
// SOLUTION:
// This singleton provides a global broadcast point that:
// 1. The choke point calls BroadcastExecution() after every tool execution
// 2. UI components (SAgentTimeline, FAgentTaskRunner) subscribe via delegates
// 3. Events are thread-safe and can be consumed on game thread
//
// This is the canonical source of truth for tool execution events.
// All execution events originate here, whether from HTTP, TCP, or internal calls.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"  // For EToolRisk, FClaudeToolResult
#include "Agent/AgentEvent.h"

/**
 * Tool execution event data - broadcast from choke point
 */
struct RIFTBORNAI_API FToolExecutionEvent
{
	/** Tool name (resolved, not aliased) */
	FString ToolName;
	
	/** Unique ID for this tool use (from LLM request) */
	FString ToolUseId;
	
	/** Whether execution succeeded */
	bool bSuccess = false;
	
	/** Error message if failed */
	FString ErrorMessage;
	
	/** Result summary (truncated for UI) */
	FString ResultSummary;
	
	/** Execution time in milliseconds */
	double ExecutionTimeMs = 0.0;
	
	/** Tool risk level */
	EToolRisk Risk = EToolRisk::Unknown;
	
	/** Was this a PROOF mode execution? */
	bool bProofMode = false;
	
	/** Was this explicitly safe (not pattern-based)? */
	bool bExplicitlySafe = false;
	
	/** Proof hash (if PROOF mode) */
	FString ProofHash;
	
	/** Timestamp */
	FDateTime Timestamp = FDateTime::UtcNow();
	
	/** Create from FClaudeToolResult */
	static FToolExecutionEvent FromResult(
		const FString& InToolName,
		const FString& InToolUseId,
		const FClaudeToolResult& Result,
		bool bInProofMode,
		bool bInExplicitlySafe);
};

/**
 * Delegate for tool execution events
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnToolExecutionEvent, const FToolExecutionEvent&);

/**
 * Plan step event data - broadcast from PlanExecutor
 */
struct RIFTBORNAI_API FPlanStepEvent
{
	enum class EType : uint8
	{
		StepStarted,
		StepCompleted,
		StepFailed,
		PlanStarted,
		PlanCompleted,
		PlanFailed
	};
	
	EType Type = EType::StepStarted;
	
	/** Step index (0-based) */
	int32 StepIndex = -1;
	
	/** Total steps in plan */
	int32 TotalSteps = 0;
	
	/** Step description */
	FString Description;
	
	/** Success/failure */
	bool bSuccess = true;
	
	/** Error message if failed */
	FString ErrorMessage;
	
	/** Iteration number (for repair loops) */
	int32 Iteration = 0;
	
	/** Timestamp */
	FDateTime Timestamp = FDateTime::UtcNow();
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlanStepEvent, const FPlanStepEvent&);

/**
 * Proof event data - broadcast when proofs are emitted/rejected
 */
struct RIFTBORNAI_API FProofEvent
{
	enum class EType : uint8
	{
		ProofEmitted,
		RejectionProofEmitted,
		VerificationPassed,
		VerificationFailed
	};
	
	EType Type = EType::ProofEmitted;
	
	/** Proof bundle ID */
	FString ProofBundleId;
	
	/** Tool/step that generated the proof */
	FString SourceTool;
	
	/** Verification result if applicable */
	bool bVerified = false;
	
	/** Details */
	FString Details;
	
	/** Timestamp */
	FDateTime Timestamp = FDateTime::UtcNow();
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnProofEvent, const FProofEvent&);

/**
 * Confirmation event - when user confirmation is required/provided
 */
struct RIFTBORNAI_API FConfirmationEvent
{
	enum class EType : uint8
	{
		Required,       // Tool needs confirmation
		Approved,       // User approved
		Rejected,       // User rejected
		Expired         // Confirmation window expired
	};
	
	EType Type = EType::Required;
	
	/** Tool requiring confirmation */
	FString ToolName;
	
	/** Confirmation token */
	FString ConfirmationToken;
	
	/** Why confirmation is needed */
	FString Reason;
	
	/** Timestamp */
	FDateTime Timestamp = FDateTime::UtcNow();
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnConfirmationEvent, const FConfirmationEvent&);

/**
 * Agent event - canonical unified event stream
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAgentEventBroadcast, const FAgentEvent&);

/**
 * FToolExecutionBroadcaster - Singleton for broadcasting execution events
 *
 * Usage from choke point:
 *   FToolExecutionBroadcaster::Get().BroadcastExecution(Event);
 *
 * Usage from UI:
 *   FToolExecutionBroadcaster::Get().OnToolExecution.AddSP(this, &MyClass::OnToolExecuted);
 */
class RIFTBORNAI_API FToolExecutionBroadcaster
{
public:
	/** Get singleton instance */
	static FToolExecutionBroadcaster& Get();
	
	// === Tool Execution Events ===
	
	/** Broadcast a tool execution event (called from choke point) */
	void BroadcastExecution(const FToolExecutionEvent& Event);
	
	/** Delegate for subscribers */
	FOnToolExecutionEvent OnToolExecution;
	
	// === Plan Step Events ===
	
	/** Broadcast a plan step event */
	void BroadcastPlanStep(const FPlanStepEvent& Event);
	
	/** Delegate for subscribers */
	FOnPlanStepEvent OnPlanStep;
	
	// === Proof Events ===
	
	/** Broadcast a proof event */
	void BroadcastProof(const FProofEvent& Event);
	
	/** Delegate for subscribers */
	FOnProofEvent OnProof;
	
	// === Confirmation Events ===
	
	/** Broadcast a confirmation event */
	void BroadcastConfirmation(const FConfirmationEvent& Event);
	
	/** Delegate for subscribers */
	FOnConfirmationEvent OnConfirmation;
	
	// === Agent Events (Canonical) ===
	
	/** Broadcast a unified agent event */
	void BroadcastAgentEvent(const FAgentEvent& Event);
	
	/** Delegate for subscribers */
	FOnAgentEventBroadcast OnAgentEvent;
	
	// === Statistics ===
	
	/** Get total events broadcast this session */
	int64 GetTotalEventCount() const { return TotalEventCount.load(); }
	
	/** Get events per type */
	int64 GetToolExecutionCount() const { return ToolExecutionCount.load(); }
	int64 GetPlanStepCount() const { return PlanStepCount.load(); }
	int64 GetProofCount() const { return ProofCount.load(); }
	int64 GetConfirmationCount() const { return ConfirmationCount.load(); }
	int64 GetAgentEventCount() const { return AgentEventCount.load(); }
	
private:
	FToolExecutionBroadcaster() = default;
	
	// Counters
	std::atomic<int64> TotalEventCount{0};
	std::atomic<int64> ToolExecutionCount{0};
	std::atomic<int64> PlanStepCount{0};
	std::atomic<int64> ProofCount{0};
	std::atomic<int64> ConfirmationCount{0};
	std::atomic<int64> AgentEventCount{0};
	
	// Thread safety for delegate invocation
	FCriticalSection BroadcastLock;
};
