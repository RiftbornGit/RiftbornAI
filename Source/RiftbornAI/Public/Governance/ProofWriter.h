// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// Proof Writer - writes tamper-evident proof bundles to disk
//
// 2026-01-31 HARDENING:
// - Atomic writes via temp file → fsync → rename
// - Chain hashing: each proof links to previous via prev_proof_hash
// - PROOF mode enforcement

#pragma once

#include "CoreMinimal.h"
#include "ClaimTypes.h"

/**
 * Writes proof bundles as canonical JSON with hash verification
 * 
 * Output format: Saved/RiftbornAI/Proofs/<timestamp>_<toolcall>_<proofhash>.json
 * 
 * INTEGRITY GUARANTEES:
 * - Atomic writes: temp file → fsync → rename (no partial writes)
 * - Chain hashing: prev_proof_hash links to prior proof
 * - Tamper detection: SHA256 hash of canonical JSON
 */
class RIFTBORNAI_API FProofWriter
{
public:
    static FProofWriter& Get();

    /**
     * Initialize with session ID (should be unique per editor session)
     */
    void Initialize(const FString& InSessionId);
    
    /** Initialize with GUID session ID */
    void Initialize(const FGuid& InSessionId);
    
    /** Get the current session ID */
    FGuid GetSessionId() const { return SessionGuid; }
    
    /** Get the hash of the last written proof (for chain linking) */
    FString GetLastProofHash() const { return LastProofHash; }
    
    /** Get the path of the last written proof */
    FString GetLastProofPath() const { return LastProofPath; }

    /**
     * Write a finalized proof bundle to disk with ATOMIC guarantees
     * 
     * ATOMIC WRITE PROTOCOL:
     * 1. Write to temp file (*.tmp)
     * 2. Flush and sync to disk
     * 3. Rename to final path
     * 4. Update chain link (LastProofHash)
     * 
     * @param Bundle - The proof bundle to write (must be finalized with hash)
     * @param OutPath - On success, filled with the path to written file
     * @return true if write succeeded atomically
     */
    bool WriteProof(const FClaimProofBundle& Bundle, FString& OutPath);
    
    /**
     * Write proof with chain linking (includes prev_proof_hash)
     * This is the PREFERRED method in PROOF mode.
     * 
     * @param Bundle - The proof bundle (will be modified to add chain link)
     * @param OutPath - On success, filled with the path to written file
     * @return true if write succeeded atomically
     */
    bool WriteProofChained(FClaimProofBundle& Bundle, FString& OutPath);

    /**
     * Read a proof bundle from disk and verify its hash
     * @param Path - Path to proof JSON file
     * @param OutBundle - The loaded bundle
     * @return true if loaded and hash verified, false if tampered or invalid
     */
    bool ReadAndVerifyProof(const FString& Path, FClaimProofBundle& OutBundle) const;
    
    /**
     * Verify the entire proof chain from a starting proof
     * @param StartPath - Path to proof to start verification from
     * @param OutBrokenLink - On failure, filled with the path where chain broke
     * @return true if entire chain verifies, false if any link is broken
     */
    bool VerifyProofChain(const FString& StartPath, FString& OutBrokenLink) const;

    /**
     * Get the proof directory path
     */
    FString GetProofDirectory() const;

    /**
     * Get all proof files in the directory
     */
    TArray<FString> GetAllProofFiles() const;

    /**
     * Load all proofs for a specific tool (for authority computation)
     * @param ToolName - Tool to filter by
     * @return Array of verified proof bundles
     */
    TArray<FClaimProofBundle> LoadProofsForTool(const FString& ToolName) const;

    /**
     * Compute authority for a tool from proof history
     */
    FToolAuthority ComputeAuthority(const FString& ToolName) const;

    /**
     * Get environment signature for current session
     */
    FString GetEnvironmentSignature() const;

private:
    FProofWriter() = default;
    
    /** Atomic write helper: temp file → fsync → rename */
    bool AtomicWriteFile(const FString& FinalPath, const FString& Content);
    
    FString SessionId;
    FGuid SessionGuid;
    FString ProofDir;
    
    // Chain linking state
    FString LastProofHash;    // Hash of last written proof
    FString LastProofPath;    // Path to last written proof
};

// =============================================================================
// P0.5: FProofChainVerifier - Comprehensive chain verification (2026-02-03)
// 
// Verifies:
// 1. Hash chain integrity (no tampered or missing proofs)
// 2. Policy hash consistency (contracts.json didn't change mid-session)
// 3. Plan context presence (mutations have plan_hash + step_id)
// 4. Session consistency (all proofs from same session)
// 5. Completeness (expected proof count for plan matches actual)
// =============================================================================

/** Result of a single proof verification */
struct RIFTBORNAI_API FProofVerificationResult
{
    bool bValid = false;
    FString ProofPath;
    FString ProofHash;
    FString ErrorMessage;
    
    // Specific issues
    bool bHashMismatch = false;
    bool bMissingPlanContext = false;
    bool bPolicyHashMismatch = false;
    bool bSessionMismatch = false;
    bool bChainLinkBroken = false;
};

/** Result of full chain verification */
struct RIFTBORNAI_API FChainVerificationResult
{
    bool bValid = false;
    FString SessionId;
    FString PolicyHash;
    
    // Statistics
    int32 TotalProofs = 0;
    int32 ValidProofs = 0;
    int32 MutatingProofsWithContext = 0;
    int32 MutatingProofsWithoutContext = 0;
    
    // First error encountered
    FString FirstError;
    FString BrokenLinkHash;
    
    // Individual proof issues
    TArray<FProofVerificationResult> ProofResults;
    
    /** Get a summary string */
    FString GetSummary() const
    {
        if (bValid)
        {
            return FString::Printf(TEXT("VALID: %d proofs verified, %d mutations with plan context"),
                TotalProofs, MutatingProofsWithContext);
        }
        return FString::Printf(TEXT("INVALID: %s (valid=%d/%d)"), *FirstError, ValidProofs, TotalProofs);
    }
};

/**
 * P0.5: Comprehensive proof chain verifier
 * 
 * Ensures PROOF mode audits are complete and tamper-evident.
 */
class RIFTBORNAI_API FProofChainVerifier
{
public:
    /**
     * Verify the entire proof chain for a session
     * @param ProofDirectory - Directory containing proof files
     * @param ExpectedSessionId - Expected session ID (empty = any)
     * @return Comprehensive verification result
     */
    static FChainVerificationResult VerifySession(
        const FString& ProofDirectory,
        const FString& ExpectedSessionId = TEXT(""));
    
    /**
     * Verify a single proof file
     * @param ProofPath - Path to proof JSON
     * @param ExpectedPolicyHash - Expected contracts hash (empty = any)
     * @param ExpectedSessionId - Expected session ID (empty = any)
     * @return Verification result for this proof
     */
    static FProofVerificationResult VerifyProof(
        const FString& ProofPath,
        const FString& ExpectedPolicyHash = TEXT(""),
        const FString& ExpectedSessionId = TEXT(""));
    
    /**
     * P0.6: Verify completeness - all expected proofs are present
     * @param ProofDirectory - Directory containing proof files
     * @param ExpectedPlanHash - Plan hash to check
     * @param ExpectedStepCount - Expected number of steps in plan
     * @return True if all expected proofs are present
     */
    static bool VerifyPlanCompleteness(
        const FString& ProofDirectory,
        const FString& ExpectedPlanHash,
        int32 ExpectedStepCount,
        TArray<int32>& OutMissingSteps);
    
    /**
     * Helper: Check if a tool is a mutating tool
     * Public for use by session stats endpoint
     */
    static bool IsMutatingTool(const FString& ToolName);
};
