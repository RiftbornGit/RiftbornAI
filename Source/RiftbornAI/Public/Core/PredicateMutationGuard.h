// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
//
// Predicate Mutation Guard - RAII scope for detecting impure predicates
//
// PROOF MODE REQUIREMENT (2026-01-31):
// Predicates are court witnesses. If they mutate state, they're perjuring.
// This guard detects mutations during predicate evaluation.

#pragma once

#include "CoreMinimal.h"
#include "Agent/AgentEvent.h"  // For FImpurityEvidence

/**
 * FPredicateMutationGuard - RAII scope that detects mutations during predicate evaluation
 * 
 * On construction:
 * - Subscribes to object modification hooks
 * - Captures baseline state (dirty packages, transaction count, actor count)
 * 
 * On destruction:
 * - Unsubscribes from hooks
 * - Compares against baseline
 * - Populates evidence if mutations detected
 * 
 * Usage:
 *   FImpurityEvidence Evidence;
 *   {
 *       FPredicateMutationGuard Guard(Evidence);
 *       // ... evaluate predicate ...
 *   }
 *   if (Evidence.HasEvidence()) { // predicate was impure }
 */
class RIFTBORNAI_API FPredicateMutationGuard
{
public:
	/**
	 * Constructor - captures baseline and subscribes to hooks
	 * @param OutEvidence Reference to evidence struct that will be populated
	 */
	explicit FPredicateMutationGuard(FImpurityEvidence& OutEvidence);
	
	/** Destructor - unsubscribes and computes delta */
	~FPredicateMutationGuard();
	
	/** Non-copyable */
	FPredicateMutationGuard(const FPredicateMutationGuard&) = delete;
	FPredicateMutationGuard& operator=(const FPredicateMutationGuard&) = delete;
	
	/** Check if any mutations were detected so far */
	bool DidMutate() const;
	
	/** Get current evidence (can be called before destruction) */
	const FImpurityEvidence& GetEvidence() const { return Evidence; }

private:
	/** Reference to output evidence */
	FImpurityEvidence& Evidence;
	
	// === Baseline State ===
	
	/** Dirty package count at guard creation */
	int32 BaselineDirtyPackageCount = 0;
	
	/** Package names that were dirty at baseline */
	TSet<FString> BaselineDirtyPackages;
	
	/** Transaction buffer count at baseline (if available) */
	int32 BaselineTransactionCount = 0;
	
	/** Active transaction state at baseline */
	bool bBaselineInTransaction = false;
	
	/** Actor count in editor world at baseline */
	int32 BaselineActorCount = 0;
	
	// === Captured Events ===
	
	/** Objects modified during guard scope */
	TArray<FString> ModifiedObjectPaths;
	
	/** Delegate handle for OnObjectModified */
	FDelegateHandle ObjectModifiedHandle;

	/** Delegate handle for explicit package dirty attempts */
	FDelegateHandle PackageMarkedDirtyHandle;
	
	/** Max objects to track (prevent unbounded memory) */
	static constexpr int32 MaxTrackedObjects = 100;
	
	// === Internal Methods ===
	
	/** Capture baseline state */
	void CaptureBaseline();
	
	/** Subscribe to modification hooks */
	void SubscribeToHooks();
	
	/** Unsubscribe from hooks */
	void UnsubscribeFromHooks();
	
	/** Compute delta and populate evidence */
	void ComputeDelta();
	
	/** Callback for object modification */
	void OnObjectModified(UObject* Object);

	/** Callback for package dirty attempts */
	void OnPackageMarkedDirty(UPackage* Package, bool bWasDirty);
	
	/** Count dirty packages */
	static int32 CountDirtyPackages(TSet<FString>& OutDirtyNames);
	
	/** Get editor world actor count */
	static int32 GetEditorActorCount();
	
	/** Check if in transaction */
	static bool IsInTransaction(int32& OutTransactionCount);
};

/**
 * Predicate evaluation wrapper with timeout + purity enforcement
 * 
 * Wraps a predicate evaluation with:
 * - Precise timing
 * - Mutation detection
 * - Timeout enforcement (post-hoc - we can't safely kill mid-evaluation)
 * - Session tainting on violation
 * 
 * Usage:
 *   FPredicateResult Result = FPredicateEnforcer::EvaluateWithGuards(
 *       Meta,
 *       [&Args]() { return IsPIERunning(Args); }
 *   );
 */
class RIFTBORNAI_API FPredicateEnforcer
{
public:
	/**
	 * Evaluate a predicate with full enforcement
	 * @param Meta Predicate metadata (timeout, purity, etc.)
	 * @param EvalFunc The actual evaluation function
	 * @param Args Arguments being passed (for evidence)
	 * @return Result with timing, purity, and taint information
	 */
	static FPredicateResult EvaluateWithGuards(
		const FPredicateMeta& Meta,
		TFunction<FPredicateResult()> EvalFunc,
		const TMap<FString, FString>& Args
	);
	
	/**
	 * Execute a simple bool predicate with guards (convenience overload)
	 * @param Meta Predicate metadata (timeout, purity, etc.)
	 * @param Predicate Simple bool predicate to evaluate
	 * @return Result with timing, purity, and taint information
	 */
	static FPredicateResult ExecuteWithGuards(
		const FPredicateMeta& Meta,
		TFunction<bool()> Predicate
	);
	
	/**
	 * Check if session is currently tainted
	 */
	static bool IsSessionTainted();
	
	/**
	 * Get taint reason (empty if not tainted)
	 */
	static FString GetTaintReason();
	
	/**
	 * Mark session as tainted (called on timeout/impurity)
	 */
	static void TaintSession(const FString& Reason);
	
	/**
	 * Clear taint (only for new sessions)
	 */
	static void ClearTaint();
	
	/**
	 * Get count of taint events this session
	 */
	static int32 GetTaintEventCount();

private:
	/** Session taint state */
	static bool bSessionTainted;
	static FString SessionTaintReason;
	static int32 TaintEventCount;
	static FCriticalSection TaintLock;
};

/**
 * Purity Allowlist - filters benign modifications that aren't real impurity
 * 
 * PROOF MODE HARDENING (2026-01-31):
 * OnObjectModified fires for all sorts of benign activity - caches, lazy init,
 * editor bookkeeping. These shouldn't trigger impurity detection.
 * 
 * True impurity is modification of /Game content or opening new transactions.
 */
namespace PurityAllowlist
{
	/** Check if a path represents benign editor/engine activity */
	RIFTBORNAI_API bool IsBenignPath(const FString& Path);
	
	/** Check if an object represents benign modification */
	RIFTBORNAI_API bool IsBenignObject(const UObject* Object);
	
	/** Check if a package path represents real game content (vs engine/script) */
	RIFTBORNAI_API bool IsGameContentPackage(const FString& PackageName);
}

