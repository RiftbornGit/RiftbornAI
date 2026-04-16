// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// RegretPersistence.h - JSONL persistence, hash chain verification, and rotation for regret memory
//
// FRegretMemory stores regret events across sessions as JSONL with hash chain integrity.
// Includes rotation policy to prevent unbounded memory growth.
//
// Extracted from RegretScope.h for modularity.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

#include "RegretEnums.h"
#include "RegretRecord.h"

/**
 * Persistent regret memory.
 * Stores regret events across sessions as JSONL.
 */
class FRegretMemory
{
public:
    FRegretMemory()
    {
        // Default path - can be overridden
        StoragePath = FPaths::ProjectSavedDir() / TEXT("RiftbornAI") / TEXT("regret_memory.jsonl");
    }

    void SetStoragePath(const FString& Path)
    {
        StoragePath = Path;
    }

    /** Record a new regret event */
    void RecordRegret(const FRegretEvent& Event)
    {
        // Calculate effective penalty
        FRegretEvent MutableEvent = Event;
        MutableEvent.EffectivePenalty = MutableEvent.CalculateEffectivePenalty();

        // Add to in-memory cache
        Events.Add(MutableEvent);
        EventById.Add(MutableEvent.EventId, Events.Num() - 1);

        // Append to persistent storage
        AppendToDisk(MutableEvent);

        // Update aggregates
        UpdateAggregates(MutableEvent);
    }

    /** Record an unblock proof */
    void RecordProof(const FUnblockProof& Proof)
    {
        Proofs.Add(Proof);
        AppendProofToDisk(Proof);

        // Mark resolved regrets
        for (const FGuid& RegretId : Proof.ResolvedRegrets)
        {
            if (int32* Idx = EventById.Find(RegretId))
            {
                Events[*Idx].bResolved = true;
                Events[*Idx].ResolutionMethod = Proof.ProofType;
                Events[*Idx].ResolutionTime = Proof.Timestamp;
            }
        }
    }

    /** Get unresolved regret events for a tool */
    TArray<FRegretEvent> GetUnresolvedForTool(const FString& ToolName) const
    {
        TArray<FRegretEvent> Result;
        for (const FRegretEvent& Event : Events)
        {
            if (Event.ToolName == ToolName && !Event.bResolved)
            {
                Result.Add(Event);
            }
        }
        return Result;
    }

    /** Get total unresolved penalty for a tool */
    float GetUnresolvedPenalty(const FString& ToolName) const
    {
        float Total = 0.0f;
        for (const FRegretEvent& Event : Events)
        {
            if (Event.ToolName == ToolName && !Event.bResolved)
            {
                Total += Event.EffectivePenalty;
            }
        }
        return Total;
    }

    /** Get penalty breakdown by scope */
    TMap<ERegretScope, float> GetPenaltyByScope(const FString& ToolName) const
    {
        TMap<ERegretScope, float> Result;
        Result.Add(ERegretScope::Tactical, 0.0f);
        Result.Add(ERegretScope::Strategic, 0.0f);
        Result.Add(ERegretScope::Optionality, 0.0f);
        Result.Add(ERegretScope::Deferred, 0.0f);
        Result.Add(ERegretScope::Unknown, 0.0f);

        for (const FRegretEvent& Event : Events)
        {
            if (Event.ToolName == ToolName && !Event.bResolved)
            {
                Result[Event.Scope] += Event.EffectivePenalty;
            }
        }
        return Result;
    }

    /** Check if tool should be blocked based on scoped regret */
    bool ShouldBlock(const FString& ToolName, float Threshold = 3.0f) const
    {
        return GetUnresolvedPenalty(ToolName) >= Threshold;
    }

    // =========================================================================
    // DEFERRED EXPIRATION PROCESSING
    // =========================================================================

    /** Process all deferred events, escalating expired ones. Returns count escalated. */
    int32 ProcessDeferredExpirations()
    {
        int32 EscalatedCount = 0;

        for (FRegretEvent& Event : Events)
        {
            if (Event.Scope == ERegretScope::Deferred && !Event.bResolved)
            {
                if (Event.ShouldEscalate())
                {
                    Event.EscalateToStrategic();
                    EscalatedCount++;

                    UE_LOG(LogTemp, Warning,
                        TEXT("DEFERRED EXPIRED: Tool '%s' regret escalated to Strategic (penalty now %.2f)"),
                        *Event.ToolName, Event.EffectivePenalty);
                }
            }
        }

        return EscalatedCount;
    }

    /** Decrement step counter for all active deferred events of a tool */
    void DecrementDeferredSteps(const FString& ToolName)
    {
        for (FRegretEvent& Event : Events)
        {
            if (Event.ToolName == ToolName &&
                Event.Scope == ERegretScope::Deferred &&
                !Event.bResolved)
            {
                Event.DecrementDeferredStep();
            }
        }

        // Check for expirations
        ProcessDeferredExpirations();
    }

    /** Resolve deferred regrets for a tool with evidence */
    int32 ResolveDeferredWithEvidence(const FString& ToolName, const FString& Evidence)
    {
        int32 ResolvedCount = 0;

        for (FRegretEvent& Event : Events)
        {
            if (Event.ToolName == ToolName &&
                Event.Scope == ERegretScope::Deferred &&
                !Event.bResolved)
            {
                Event.ResolveWithEvidence(Evidence);
                ResolvedCount++;
            }
        }

        return ResolvedCount;
    }

    // =========================================================================
    // PROOF-BASED UNBLOCKING WITH ANTI-FARMING
    // =========================================================================

    /** Maximum penalty reductions allowed per epoch (anti-farming) */
    static constexpr int32 MAX_REDUCTIONS_PER_EPOCH = 5;
    static constexpr int32 EPOCH_DURATION_SECONDS = 300;  // 5 minutes

    /**
     * Attempt to apply a proof for unblocking.
     * Returns actual reduction applied (may be 0 if rejected).
     */
    float ApplyProofWithValidation(const FUnblockProof& Proof)
    {
        // Anti-farming check 1: Is proof valid?
        if (!Proof.IsValid())
        {
            UE_LOG(LogTemp, Warning,
                TEXT("PROOF REJECTED: Invalid proof for '%s' (world_delta=%.2f, claim=%s)"),
                *Proof.ToolName, Proof.WorldDelta, *Proof.ClaimFamily);
            return 0.0f;
        }

        // Anti-farming check 2: Velocity limit
        if (!CheckReductionVelocity(Proof.ToolName))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("PROOF REJECTED: Rate limit exceeded for '%s' (max %d per %ds)"),
                *Proof.ToolName, MAX_REDUCTIONS_PER_EPOCH, EPOCH_DURATION_SECONDS);
            return 0.0f;
        }

        // Anti-farming check 3: Claim family must match
        float MatchingReduction = 0.0f;
        TArray<FGuid> ActuallyResolved;

        for (const FGuid& RegretId : Proof.ResolvedRegrets)
        {
            if (int32* Idx = EventById.Find(RegretId))
            {
                FRegretEvent& Event = Events[*Idx];

                // Skip already resolved
                if (Event.bResolved) continue;

                // Check claim family match
                if (!Event.ViolatedClaimFamily.IsEmpty() &&
                    Event.ViolatedClaimFamily != Proof.ClaimFamily)
                {
                    UE_LOG(LogTemp, Log,
                        TEXT("CLAIM MISMATCH: Event claims '%s', proof claims '%s' - skipping"),
                        *Event.ViolatedClaimFamily, *Proof.ClaimFamily);
                    continue;
                }

                // Check tool match by name
                if (Event.ToolName != Proof.ToolName)
                {
                    UE_LOG(LogTemp, Log,
                        TEXT("TOOL MISMATCH: Event tool '%s', proof tool '%s' - skipping"),
                        *Event.ToolName, *Proof.ToolName);
                    continue;
                }

                // This regret can be resolved
                Event.bResolved = true;
                Event.ResolutionMethod = Proof.ProofType;
                Event.ResolutionTime = Proof.Timestamp;
                MatchingReduction += Event.EffectivePenalty;
                ActuallyResolved.Add(RegretId);
            }
        }

        if (MatchingReduction > 0.0f)
        {
            // Record successful proof
            FUnblockProof ValidatedProof = Proof;
            ValidatedProof.ResolvedRegrets = ActuallyResolved;
            ValidatedProof.PenaltyReduction = MatchingReduction;
            Proofs.Add(ValidatedProof);
            AppendProofToDisk(ValidatedProof);

            // Record for velocity tracking
            RecordReductionEvent(Proof.ToolName);

            UE_LOG(LogTemp, Warning,
                TEXT("PROOF ACCEPTED: Tool '%s' penalty reduced by %.2f (resolved %d regrets)"),
                *Proof.ToolName, MatchingReduction, ActuallyResolved.Num());
        }

        return MatchingReduction;
    }

    /** Load from persistent storage */
    bool LoadFromDisk()
    {
        if (!FPaths::FileExists(StoragePath))
        {
            return true; // No file yet is not an error
        }

        TArray<FString> Lines;
        if (!FFileHelper::LoadFileToStringArray(Lines, *StoragePath))
        {
            return false;
        }

        Events.Empty();
        EventById.Empty();

        for (const FString& Line : Lines)
        {
            if (Line.IsEmpty()) continue;

            TSharedPtr<FJsonObject> Obj;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Line);
            if (FJsonSerializer::Deserialize(Reader, Obj) && Obj.IsValid())
            {
                FRegretEvent Event = FRegretEvent::FromJson(Obj);
                Events.Add(Event);
                EventById.Add(Event.EventId, Events.Num() - 1);
            }
        }

        return true;
    }

    /** Get all events (for debugging/analysis) */
    const TArray<FRegretEvent>& GetAllEvents() const { return Events; }

    /** Get event count */
    int32 GetEventCount() const { return Events.Num(); }

    /** Get unresolved count */
    int32 GetUnresolvedCount() const
    {
        int32 Count = 0;
        for (const FRegretEvent& Event : Events)
        {
            if (!Event.bResolved) Count++;
        }
        return Count;
    }

    // =========================================================================
    // HASH CHAIN INTEGRITY VERIFICATION
    // =========================================================================

    /**
     * Result of hash chain verification.
     */
    struct FHashChainVerificationResult
    {
        bool bValid = true;
        int32 TotalEntries = 0;
        int32 VerifiedEntries = 0;
        int32 CorruptedEntries = 0;
        TArray<int32> CorruptedIndices;
        FString FirstCorruptionReason;
    };

    /**
     * Verify the integrity of the hash chain from the storage file.
     * This re-reads the file and verifies each entry's hash against its predecessor.
     *
     * Returns verification result with details about any corruption found.
     */
    FHashChainVerificationResult VerifyHashChainIntegrity() const
    {
        FHashChainVerificationResult Result;

        if (!FPaths::FileExists(StoragePath))
        {
            // No file = valid (empty)
            return Result;
        }

        TArray<FString> Lines;
        if (!FFileHelper::LoadFileToStringArray(Lines, *StoragePath))
        {
            Result.bValid = false;
            Result.FirstCorruptionReason = TEXT("Failed to read storage file");
            return Result;
        }

        FString ExpectedPrevHash = TEXT("");
        int32 ExpectedSeq = 0;

        for (int32 i = 0; i < Lines.Num(); i++)
        {
            const FString& Line = Lines[i];
            if (Line.IsEmpty()) continue;

            Result.TotalEntries++;

            TSharedPtr<FJsonObject> Obj;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Line);
            if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid())
            {
                Result.bValid = false;
                Result.CorruptedEntries++;
                Result.CorruptedIndices.Add(i);
                if (Result.FirstCorruptionReason.IsEmpty())
                {
                    Result.FirstCorruptionReason = FString::Printf(TEXT("Entry %d: Invalid JSON"), i);
                }
                continue;
            }

            // Extract hash chain fields
            FString PrevHash = Obj->GetStringField(TEXT("_prev_hash"));
            int32 Seq = (int32)Obj->GetNumberField(TEXT("_seq"));

            // Verify sequence
            if (Seq != ExpectedSeq)
            {
                Result.bValid = false;
                Result.CorruptedEntries++;
                Result.CorruptedIndices.Add(i);
                if (Result.FirstCorruptionReason.IsEmpty())
                {
                    Result.FirstCorruptionReason = FString::Printf(
                        TEXT("Entry %d: Sequence mismatch (expected %d, got %d)"),
                        i, ExpectedSeq, Seq);
                }
            }

            // Verify prev_hash
            if (PrevHash != ExpectedPrevHash)
            {
                Result.bValid = false;
                Result.CorruptedEntries++;
                if (!Result.CorruptedIndices.Contains(i))
                {
                    Result.CorruptedIndices.Add(i);
                }
                if (Result.FirstCorruptionReason.IsEmpty())
                {
                    Result.FirstCorruptionReason = FString::Printf(
                        TEXT("Entry %d: Hash chain broken (expected '%s', got '%s')"),
                        i, *ExpectedPrevHash, *PrevHash);
                }
            }
            else
            {
                Result.VerifiedEntries++;
            }

            // Compute this entry's hash for next iteration
            Obj->RemoveField(TEXT("_prev_hash"));
            Obj->RemoveField(TEXT("_seq"));
            Obj->SetStringField(TEXT("_prev_hash"), PrevHash);
            Obj->SetNumberField(TEXT("_seq"), Seq);

            FString JsonForHash;
            TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
                TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonForHash);
            FJsonSerializer::Serialize(Obj.ToSharedRef(), Writer);

            ExpectedPrevHash = FMD5::HashAnsiString(*JsonForHash).Left(16);
            ExpectedSeq++;
        }

        return Result;
    }

    /**
     * Emit a corruption event for monitoring/alerting.
     */
    void EmitCorruptionEvent(const FHashChainVerificationResult& Verification)
    {
        if (Verification.bValid) return;

        UE_LOG(LogTemp, Error,
            TEXT("REGRET MEMORY CORRUPTION DETECTED: %d/%d entries corrupted. Reason: %s"),
            Verification.CorruptedEntries,
            Verification.TotalEntries,
            *Verification.FirstCorruptionReason);

        // Write corruption report to separate file
        FString CorruptionReportPath = FPaths::GetPath(StoragePath) /
            FString::Printf(TEXT("corruption_report_%s.json"),
                *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));

        TSharedPtr<FJsonObject> Report = MakeShared<FJsonObject>();
        Report->SetStringField(TEXT("timestamp"), FDateTime::Now().ToIso8601());
        Report->SetBoolField(TEXT("valid"), Verification.bValid);
        Report->SetNumberField(TEXT("total_entries"), Verification.TotalEntries);
        Report->SetNumberField(TEXT("verified_entries"), Verification.VerifiedEntries);
        Report->SetNumberField(TEXT("corrupted_entries"), Verification.CorruptedEntries);
        Report->SetStringField(TEXT("first_corruption_reason"), Verification.FirstCorruptionReason);

        TArray<TSharedPtr<FJsonValue>> IndicesArray;
        for (int32 Idx : Verification.CorruptedIndices)
        {
            IndicesArray.Add(MakeShared<FJsonValueNumber>(Idx));
        }
        Report->SetArrayField(TEXT("corrupted_indices"), IndicesArray);

        FString ReportJson;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ReportJson);
        FJsonSerializer::Serialize(Report.ToSharedRef(), Writer);

        FFileHelper::SaveStringToFile(ReportJson, *CorruptionReportPath);
    }

    // =========================================================================
    // ROTATION POLICY - Prevent unbounded memory growth
    // =========================================================================

    /** Maximum events to keep in memory */
    static constexpr int32 MAX_EVENTS_IN_MEMORY = 10000;

    /** Maximum age of events in days before they're rotated out */
    static constexpr int32 MAX_EVENT_AGE_DAYS = 90;

    /** Minimum events to archive when rotation is triggered */
    static constexpr int32 MIN_ARCHIVE_BATCH_SIZE = 1000;

    /**
     * Apply rotation policy.
     * Returns number of events rotated to archive.
     *
     * Policy:
     * 1. Keep all unresolved events (never delete active penalties)
     * 2. Rotate resolved events older than MAX_EVENT_AGE_DAYS
     * 3. If still over MAX_EVENTS_IN_MEMORY, rotate oldest resolved events
     */
    int32 ApplyRotationPolicy()
    {
        if (Events.Num() <= MAX_EVENTS_IN_MEMORY)
        {
            return 0;  // No rotation needed
        }

        TArray<FRegretEvent> ToArchive;
        TArray<FRegretEvent> ToKeep;
        const FDateTime CutoffDate = FDateTime::Now() - FTimespan::FromDays(MAX_EVENT_AGE_DAYS);

        // First pass: separate by age and resolution status
        for (const FRegretEvent& Event : Events)
        {
            // NEVER rotate unresolved events - they're active penalties
            if (!Event.bResolved)
            {
                ToKeep.Add(Event);
                continue;
            }

            // Rotate old resolved events
            if (Event.Timestamp < CutoffDate)
            {
                ToArchive.Add(Event);
            }
            else
            {
                ToKeep.Add(Event);
            }
        }

        // If still over limit, archive oldest resolved events
        if (ToKeep.Num() > MAX_EVENTS_IN_MEMORY)
        {
            // Sort ToKeep by timestamp (oldest first among resolved)
            ToKeep.Sort([](const FRegretEvent& A, const FRegretEvent& B)
            {
                // Unresolved always sorts to the end (keep them)
                if (!A.bResolved && B.bResolved) return false;
                if (A.bResolved && !B.bResolved) return true;
                // Among resolved, oldest first (for archiving)
                return A.Timestamp < B.Timestamp;
            });

            // Move oldest resolved events to archive
            while (ToKeep.Num() > MAX_EVENTS_IN_MEMORY && ToKeep.Num() > 0)
            {
                const FRegretEvent& Event = ToKeep[0];
                if (Event.bResolved)
                {
                    ToArchive.Add(Event);
                    ToKeep.RemoveAt(0);
                }
                else
                {
                    break;  // Hit an unresolved event, stop
                }
            }
        }

        // Archive the events
        if (ToArchive.Num() >= MIN_ARCHIVE_BATCH_SIZE ||
            (ToArchive.Num() > 0 && Events.Num() > MAX_EVENTS_IN_MEMORY * 1.5))
        {
            ArchiveEvents(ToArchive);

            // Rebuild in-memory state
            Events = ToKeep;
            EventById.Empty();
            for (int32 i = 0; i < Events.Num(); i++)
            {
                EventById.Add(Events[i].EventId, i);
            }

            // Rebuild aggregates
            TotalPenaltyByTool.Empty();
            UnresolvedCountByTool.Empty();
            for (const FRegretEvent& Event : Events)
            {
                UpdateAggregates(Event);
            }

            // Rewrite the main file (compact it)
            RewriteStorageFile();

            UE_LOG(LogTemp, Warning,
                TEXT("REGRET ROTATION: Archived %d events, keeping %d (unresolved=%d)"),
                ToArchive.Num(), Events.Num(), GetUnresolvedCount());

            return ToArchive.Num();
        }

        return 0;
    }

    /**
     * Get rotation status for monitoring.
     */
    struct FRotationStatus
    {
        int32 TotalEvents;
        int32 UnresolvedEvents;
        int32 EventsOverAgeLimit;
        bool RotationNeeded;
        float CapacityUsed;  // 0.0-1.0
    };

    FRotationStatus GetRotationStatus() const
    {
        FRotationStatus Status;
        Status.TotalEvents = Events.Num();
        Status.UnresolvedEvents = GetUnresolvedCount();
        Status.EventsOverAgeLimit = 0;

        const FDateTime CutoffDate = FDateTime::Now() - FTimespan::FromDays(MAX_EVENT_AGE_DAYS);
        for (const FRegretEvent& Event : Events)
        {
            if (Event.bResolved && Event.Timestamp < CutoffDate)
            {
                Status.EventsOverAgeLimit++;
            }
        }

        Status.RotationNeeded = Events.Num() > MAX_EVENTS_IN_MEMORY;
        Status.CapacityUsed = static_cast<float>(Events.Num()) / static_cast<float>(MAX_EVENTS_IN_MEMORY);

        return Status;
    }

private:
    /**
     * Archive events to a separate file.
     */
    void ArchiveEvents(const TArray<FRegretEvent>& EventsToArchive)
    {
        if (EventsToArchive.Num() == 0) return;

        // Create archive filename with timestamp
        FString ArchivePath = FPaths::GetPath(StoragePath) /
            FString::Printf(TEXT("regret_archive_%s.jsonl"),
                *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));

        FString ArchiveContent;
        for (const FRegretEvent& Event : EventsToArchive)
        {
            TSharedPtr<FJsonObject> Obj = Event.ToJson();
            Obj->SetBoolField(TEXT("_archived"), true);
            Obj->SetStringField(TEXT("_archive_time"), FDateTime::Now().ToIso8601());

            FString JsonLine;
            TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
                TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonLine);
            FJsonSerializer::Serialize(Obj.ToSharedRef(), Writer);
            ArchiveContent += JsonLine + TEXT("\n");
        }

        FFileHelper::SaveStringToFile(ArchiveContent, *ArchivePath, FFileHelper::EEncodingOptions::AutoDetect);

        UE_LOG(LogTemp, Log,
            TEXT("Archived %d events to: %s"),
            EventsToArchive.Num(), *ArchivePath);
    }

    /**
     * Rewrite the storage file with current events (compaction).
     */
    void RewriteStorageFile()
    {
        // Reset hash chain for new file
        LastEventHash = TEXT("");
        ChainSequence = 0;

        // Backup existing file
        FString BackupPath = StoragePath + TEXT(".bak");
        IFileManager::Get().Move(*BackupPath, *StoragePath, true);

        // Write all current events
        for (const FRegretEvent& Event : Events)
        {
            AppendToDisk(Event);
        }

        // Delete backup on success
        IFileManager::Get().Delete(*BackupPath);
    }

    FString StoragePath;
    TArray<FRegretEvent> Events;
    TMap<FGuid, int32> EventById;
    TArray<FUnblockProof> Proofs;

    // Hash chain for integrity verification
    FString LastEventHash;
    int32 ChainSequence = 0;

    // Aggregated stats for fast lookup
    TMap<FString, float> TotalPenaltyByTool;
    TMap<FString, int32> UnresolvedCountByTool;

    // =========================================================================
    // ANTI-FARMING: Velocity tracking for penalty reductions
    // =========================================================================

    /** Timestamps of recent reductions per tool */
    TMap<FString, TArray<FDateTime>> ReductionTimestamps;

    /** Check if reduction is allowed (velocity limit not exceeded) */
    bool CheckReductionVelocity(const FString& ToolName) const
    {
        const TArray<FDateTime>* Timestamps = ReductionTimestamps.Find(ToolName);
        if (!Timestamps) return true;  // No history = allowed

        // Count reductions in current epoch
        const FDateTime EpochStart = FDateTime::Now() - FTimespan::FromSeconds(EPOCH_DURATION_SECONDS);
        int32 RecentCount = 0;

        for (const FDateTime& Ts : *Timestamps)
        {
            if (Ts >= EpochStart)
            {
                RecentCount++;
            }
        }

        return RecentCount < MAX_REDUCTIONS_PER_EPOCH;
    }

    /** Record a reduction event for velocity tracking */
    void RecordReductionEvent(const FString& ToolName)
    {
        TArray<FDateTime>& Timestamps = ReductionTimestamps.FindOrAdd(ToolName);
        Timestamps.Add(FDateTime::Now());

        // Prune old timestamps
        const FDateTime CutoffTime = FDateTime::Now() - FTimespan::FromSeconds(EPOCH_DURATION_SECONDS * 2);
        Timestamps.RemoveAll([&CutoffTime](const FDateTime& Ts) { return Ts < CutoffTime; });
    }

    void AppendToDisk(const FRegretEvent& Event)
    {
        // Ensure directory exists
        FString Dir = FPaths::GetPath(StoragePath);
        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        PlatformFile.CreateDirectoryTree(*Dir);

        // Serialize to JSON
        TSharedPtr<FJsonObject> Obj = Event.ToJson();

        // Add hash chain fields for integrity
        Obj->SetStringField(TEXT("_prev_hash"), LastEventHash);
        Obj->SetNumberField(TEXT("_seq"), ChainSequence);

        FString JsonLine;
        TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
            TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonLine);
        FJsonSerializer::Serialize(Obj.ToSharedRef(), Writer);

        // Compute hash of this entry (for next entry's _prev_hash)
        LastEventHash = FMD5::HashAnsiString(*JsonLine).Left(16);
        ChainSequence++;

        JsonLine += TEXT("\n");

        // Append to file
        FFileHelper::SaveStringToFile(JsonLine, *StoragePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
    }

    void AppendProofToDisk(const FUnblockProof& Proof)
    {
        FString ProofPath = FPaths::GetPath(StoragePath) / TEXT("unblock_proofs.jsonl");

        TSharedPtr<FJsonObject> Obj = Proof.ToJson();
        FString JsonLine;
        TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
            TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonLine);
        FJsonSerializer::Serialize(Obj.ToSharedRef(), Writer);
        JsonLine += TEXT("\n");

        FFileHelper::SaveStringToFile(JsonLine, *ProofPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
    }

    void UpdateAggregates(const FRegretEvent& Event)
    {
        if (!Event.bResolved)
        {
            float& Total = TotalPenaltyByTool.FindOrAdd(Event.ToolName);
            Total += Event.EffectivePenalty;

            int32& Count = UnresolvedCountByTool.FindOrAdd(Event.ToolName);
            Count++;
        }
    }
};
