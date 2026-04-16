// Copyright RiftbornAI. All Rights Reserved.
// PlanPreflight.h - Pre-execution validation for PROOF eligibility
//
// Priority 7 (2026-01-31):
// Before user can approve a plan, preflight answers:
// - Is this plan PROOF-approvable?
// - Which steps fail certification and why?
// - What must be changed to make it approvable?
//
// RULE: Approval is BLOCKED if any step is not EToolCertificationStatus::ProofReady in PROOF mode.
// RULE: Preflight uses the SAME certification logic as execution enforcement.

#pragma once

#include "CoreMinimal.h"
#include "ToolCertification.h"
#include "UI/CopilotStateMachine.h"

/**
 * Fix suggestion type - what kind of action resolves the gap
 */
enum class EFixSuggestionType : uint8
{
	InstallToolPack,        // Need to install/enable a tool pack
	AddContract,            // Tool needs a contract definition
	AddWitness,             // Contract needs RequiredWitness
	SwitchTool,             // Use a different tool that's already compliant
	SwitchLane,             // Tool needs C++ implementation (PythonDev → CppOnly)
	ImplementUndo,          // Tool needs undo support
	DowngradeRiskTier,      // Accept irreversibility in contract
	CannotFix               // No known fix available
};

/**
 * Single fix suggestion for a failing step
 */
struct RIFTBORNAI_API FPreflightFixSuggestion
{
	EFixSuggestionType Type = EFixSuggestionType::CannotFix;
	FString Description;            // Human-readable explanation
	FString ActionLabel;            // Button text (e.g., "Switch to spawn_actor_cpp")
	
	// For tool switching
	FString AlternativeToolName;    // If Type == SwitchTool
	
	// For witness fixes
	TArray<FString> MissingWitnesses;  // What needs to be emitted
	
	FPreflightFixSuggestion() = default;
	FPreflightFixSuggestion(EFixSuggestionType InType, const FString& InDesc)
		: Type(InType), Description(InDesc) {}
};

/**
 * Preflight result for a single step
 */
struct RIFTBORNAI_API FStepPreflightResult
{
	// Step identity
	int32 StepIndex = 0;
	FString ToolName;
	
	// Certification status (from FToolCertificationScanner)
	EToolCertificationStatus Status = EToolCertificationStatus::Blocked;
	bool bProofEligible = false;
	
	// Blocking reasons (from certification)
	TArray<FString> BlockingReasons;
	
	// Argument validation
	bool bArgsValid = true;
	TArray<FString> ArgValidationErrors;
	
	// Witness binding validation
	bool bWitnessBindingsValid = true;
	TArray<FString> UnresolvedBindings;  // Bindings needed from earlier steps
	
	// Overall
	bool bPasses() const { return bProofEligible && bArgsValid && bWitnessBindingsValid; }
	
	// Fix suggestions
	TArray<FPreflightFixSuggestion> Suggestions;
	
	// Full certification result (for UI detail view)
	TOptional<FToolCertificationResult> CertificationDetail;
};

/**
 * Full plan preflight result
 */
struct RIFTBORNAI_API FPlanPreflightResult
{
	// Plan identity
	FGuid DraftId;
	FDateTime CheckedAt;
	
	// Overall result
	bool bProofApprovable = false;      // True if ALL steps pass in PROOF mode
	bool bDevApprovable = true;         // True in non-PROOF mode (less strict)
	int32 PassingStepCount = 0;
	int32 FailingStepCount = 0;
	
	// Per-step results
	TArray<FStepPreflightResult> StepResults;
	
	// Aggregate blocking reasons
	TArray<FString> BlockingReasons;    // High-level summary for header
	
	// Risk summary
	EToolRisk HighestRisk = EToolRisk::Safe;
	int32 MutatingStepCount = 0;
	int32 DestructiveStepCount = 0;
	bool bFullyReversible = true;
	
	// Quick accessors
	bool HasFailingSteps() const { return FailingStepCount > 0; }
	
	TArray<int32> GetFailingStepIndices() const
	{
		TArray<int32> Indices;
		for (const FStepPreflightResult& SR : StepResults)
		{
			if (!SR.bPasses())
			{
				Indices.Add(SR.StepIndex);
			}
		}
		return Indices;
	}
	
	FString GetBlockingSummary() const
	{
		if (bProofApprovable)
		{
			return TEXT("Plan is PROOF-approvable");
		}
		if (BlockingReasons.Num() > 0)
		{
			return BlockingReasons[0];
		}
		return FString::Printf(TEXT("%d step(s) fail PROOF eligibility"), FailingStepCount);
	}
	
	// Export for debugging/CI
	TSharedPtr<FJsonObject> ToJson() const;
	FString ToMarkdown() const;
};

/**
 * Alternative tool mapping for fix suggestions
 * Maps non-compliant tools to compliant equivalents
 */
struct RIFTBORNAI_API FToolAlternative
{
	FString FromTool;               // Non-compliant tool
	FString ToTool;                 // Compliant alternative
	FString Reason;                 // Why the switch helps
	bool bArgCompatible = true;     // Can args be copied directly?
	TMap<FString, FString> ArgMapping;  // If not, map arg names
};

/**
 * FPlanPreflight - Validates a plan draft before approval
 * 
 * This is the gatekeeper. Uses the SAME certification logic as execution,
 * ensuring what preflight approves will actually execute.
 */
class RIFTBORNAI_API FPlanPreflight
{
public:
	static FPlanPreflight& Get();
	
	/**
	 * Run preflight on a plan draft
	 * 
	 * @param Draft         The plan to validate
	 * @param bProofMode    True if in PROOF mode (strict), false for dev mode
	 * @return              Full preflight result with per-step details
	 */
	FPlanPreflightResult CheckPlan(const FPlanDraft& Draft, bool bProofMode = true);
	
	/**
	 * Quick check: is this plan approvable?
	 * Faster than full CheckPlan if you just need the answer.
	 */
	bool IsApprovable(const FPlanDraft& Draft, bool bProofMode = true);
	
	/**
	 * Check single step (for incremental validation during edits)
	 */
	FStepPreflightResult CheckStep(const FDraftToolCall& Step, int32 StepIndex, const FPlanDraft& Context);
	
	/**
	 * Generate fix suggestions for a failing step
	 */
	TArray<FPreflightFixSuggestion> GenerateSuggestions(
		const FStepPreflightResult& FailingStep,
		EToolCertificationStatus Status);
	
	/**
	 * Get alternative tool if one exists
	 */
	TOptional<FToolAlternative> GetToolAlternative(const FString& ToolName) const;
	
	/**
	 * Register a tool alternative (for plugins/tool packs)
	 */
	void RegisterAlternative(const FToolAlternative& Alt);

private:
	FPlanPreflight() { InitializeAlternatives(); }
	
	/** Initialize known tool alternatives */
	void InitializeAlternatives();
	
	/** Validate step arguments against schema */
	bool ValidateStepArgs(const FDraftToolCall& Step, TArray<FString>& OutErrors);
	
	/** Validate witness bindings (does step N have what step N+1 needs?) */
	bool ValidateWitnessBindings(const FPlanDraft& Draft, int32 StepIndex, TArray<FString>& OutUnresolved);
	
	/** Tool alternatives map */
	TMap<FString, FToolAlternative> Alternatives;
};

/**
 * Console command: RiftbornAI.PreflightPlan <PlanJson>
 * Runs preflight and prints result
 */
void RegisterPreflightCommands();
