// Copyright RiftbornAI. All Rights Reserved.
// Tool Certification Report - PROOF eligibility scanner
//
// Priority 6 (2026-01-31):
// This answers: "Which tools are PROOF-ready and which are gaps?"
// Without this visibility, enforcement creates chaos instead of progress.

#pragma once

#include "CoreMinimal.h"
#include "ToolContract.h"
#include "ClaudeToolUse.h"

/**
 * Certification status for a single tool
 */
enum class EToolCertificationStatus : uint8
{
	ProofReady,         // Fully certified for PROOF mode
	NeedsContract,      // Missing contract entirely
	NeedsWitness,       // Has contract but missing RequiredWitness spec
	NeedsProduces,      // Has contract but Produces don't match RequiredWitness
	NeedsDeterminism,   // Nondeterministic in ways that block PROOF
	NeedsUndo,          // Claims reversible but has no undo support
	PythonBlocked,      // Python-only tool blocked in PROOF
	Blocked             // Other blocking issues
};

FString CertificationStatusToString(EToolCertificationStatus Status);

/**
 * Runtime certification status for the live canary slice.
 * This is orthogonal to static PROOF eligibility.
 */
enum class EToolRuntimeCertificationStatus : uint8
{
	NotRun,
	Passed,
	Failed,
	Skipped
};

FString RuntimeCertificationStatusToString(EToolRuntimeCertificationStatus Status);

/**
 * Per-tool certification result
 */
struct RIFTBORNAI_API FToolCertificationResult
{
	// Identity
	FString ToolName;
	FString Category;
	
	// Contract status
	bool bHasContract = false;
	bool bContractValid = false;
	FString ContractHash;
	
	// PROOF eligibility
	bool bProofEligible = false;
	EToolCertificationStatus Status = EToolCertificationStatus::Blocked;
	TArray<FString> BlockingReasons;
	
	// Evidence contract
	TArray<FString> RequiredWitness;
	TArray<FString> ProducesKeys;
	TArray<FString> ConsumesKeys;
	
	// Execution profile
	EExecutionLane Lane = EExecutionLane::CppOnly;
	EDeterminismClass Determinism = EDeterminismClass::Deterministic;
	EContractRiskTier RiskTier = EContractRiskTier::Safe;
	
	// Undo support
	bool bClaimsReversible = false;
	bool bHasUndoSupport = false;
	bool bUndoMismatch = false;  // Claims reversible but no undo
	
	// Runtime stats (if available)
	int32 ExecutionCount = 0;
	int32 WitnessPassCount = 0;
	int32 WitnessFailCount = 0;
	float WitnessPassRate = 0.0f;

	// Runtime certification slice (if executed)
	EToolRuntimeCertificationStatus RuntimeStatus = EToolRuntimeCertificationStatus::NotRun;
	bool bRuntimeCanaryCovered = false;
	FDateTime RuntimeLastExecutedAt;
	double RuntimeExecutionTimeMs = 0.0f;
	TArray<FString> RuntimeEvidence;
	
	// Convert to JSON
	TSharedPtr<FJsonObject> ToJson() const;
};

/**
 * Aggregate metrics for the certification report
 */
struct RIFTBORNAI_API FToolCertificationAggregates
{
	// Totals
	int32 TotalTools = 0;
	int32 ProofReadyCount = 0;
	int32 NeedsContractCount = 0;
	int32 NeedsWitnessCount = 0;
	int32 NeedsProducesCount = 0;
	int32 BlockedCount = 0;

	// Runtime canary slice
	int32 RuntimeCoveredCount = 0;
	int32 RuntimePassedCount = 0;
	int32 RuntimeFailedCount = 0;
	int32 RuntimeSkippedCount = 0;
	
	// By lane
	int32 CppOnlyCount = 0;
	int32 PythonDevCount = 0;
	int32 PythonAlwaysCount = 0;  // Should be 0 in healthy system
	
	// By risk tier
	TMap<EContractRiskTier, int32> ByRiskTier;
	
	// By category
	TMap<FString, int32> ByCategory;
	
	// Coverage
	float ContractCoverage = 0.0f;      // % of tools with contracts
	float ProofReadyCoverage = 0.0f;    // % of tools PROOF-ready
	float WitnessCoverage = 0.0f;       // % of contracted tools with witnesses
	float RuntimeCoverage = 0.0f;       // % of tools covered by runtime canaries
	
	// Critical gaps (most used tools that aren't PROOF-ready)
	TArray<FString> CriticalGaps;
	
	// Top offenders (tools failing witness enforcement most)
	TArray<FString> TopWitnessOffenders;
	
	// Convert to JSON
	TSharedPtr<FJsonObject> ToJson() const;
};

/**
 * Full certification report
 */
struct RIFTBORNAI_API FToolCertificationReport
{
	// Metadata
	FDateTime Timestamp;
	FString RegistryHash;
	FString EngineVersion;
	bool bProofModeEnabled = false;
	bool bRuntimeSliceExecuted = false;
	FString RuntimeSliceName;
	TArray<FString> RuntimeWarnings;
	
	// Per-tool results
	TMap<FString, FToolCertificationResult> ToolResults;
	
	// Aggregates
	FToolCertificationAggregates Aggregates;
	
	// Convert to JSON
	TSharedPtr<FJsonObject> ToJson() const;
	
	// Generate human-readable summary
	FString ToMarkdownSummary() const;
	
	// Save to file
	bool SaveToFile(const FString& JsonPath, const FString& MarkdownPath) const;
};

/**
 * Tool Certification Scanner
 * 
 * Scans all registered tools against contracts and generates certification report.
 * This is the single source of truth for PROOF eligibility.
 */
class RIFTBORNAI_API FToolCertificationScanner
{
public:
	static FToolCertificationScanner& Get();
	
	/**
	 * Generate full certification report
	 * Scans all tools in registry against all contracts
	 */
	FToolCertificationReport GenerateReport(bool bIncludeRuntimeSlice = false);
	
	/**
	 * Check single tool certification status
	 */
	FToolCertificationResult CertifyTool(const FString& ToolName);
	
	/**
	 * Get list of PROOF-ready tools
	 */
	TArray<FString> GetProofReadyTools();
	
	/**
	 * Get list of critical gaps (high-use tools not PROOF-ready)
	 */
	TArray<FString> GetCriticalGaps();
	
	/**
	 * Check if a specific tool is PROOF-eligible
	 */
	bool IsProofEligible(const FString& ToolName);
	
	/**
	 * Get blocking reasons for a tool
	 */
	TArray<FString> GetBlockingReasons(const FString& ToolName);

private:
	FToolCertificationScanner() = default;
	
	/** Evaluate a tool against its contract */
	FToolCertificationResult EvaluateTool(const FClaudeTool* Tool, const FToolContract* Contract);
	
	/** Check if witness spec is complete */
	bool ValidateWitnessSpec(const FToolContract& Contract, TArray<FString>& OutIssues);
	
	/** Check if produces/consumes are consistent */
	bool ValidateEvidenceBindings(const FToolContract& Contract, TArray<FString>& OutIssues);
	
	/** Check undo support matches risk tier */
	bool ValidateUndoSupport(const FClaudeTool* Tool, const FToolContract* Contract, TArray<FString>& OutIssues);

	/** Run the live runtime-certification slice and merge results into the report */
	void ApplyRuntimeCertificationSlice(FToolCertificationReport& Report);
	
	/** Cached report (regenerated on demand) */
	TOptional<FToolCertificationReport> CachedReport;
	FDateTime CacheTimestamp;
	
	/** Cache validity (5 minutes) */
	static constexpr double CacheValiditySeconds = 300.0;
};

/**
 * Console command for generating certification report
 * Usage: RiftbornAI.GenerateCertificationReport [OutputPath]
 */
void RegisterCertificationCommands();
