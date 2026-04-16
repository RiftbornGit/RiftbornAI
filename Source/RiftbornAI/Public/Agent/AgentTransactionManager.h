// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"

class AActor;

/**
 * Transaction System for RiftbornAI
 * Enables undo/rollback of multi-step operations
 */

RIFTBORNAI_API DECLARE_LOG_CATEGORY_EXTERN(LogAgentTransaction, Log, All);

/**
 * Types of undoable operations
 */
UENUM(BlueprintType)
enum class ETransactionOperationType : uint8
{
    None,
    CreateAsset,      // Created a new asset
    ModifyAsset,      // Modified an existing asset
    DeleteAsset,      // Deleted an asset
    SpawnActor,       // Spawned an actor
    DestroyActor,     // Destroyed an actor
    ModifyActor,      // Modified actor properties
    CreateFile,       // Created a file
    ModifyFile,       // Modified a file
    DeleteFile,       // Deleted a file
    Compile,          // Compiled code
    Custom            // Custom operation with manual undo
};

/**
 * Record of a single operation that can be undone
 */
struct RIFTBORNAI_API FTransactionOperation
{
    ETransactionOperationType Type = ETransactionOperationType::None;
    FString Description;
    
    // Asset info
    FString AssetPath;
    FString AssetClass;
    
    // Actor info
    FGuid ActorGuid;
    FString ActorClass;
    FTransform ActorTransform;
    
    // File info
    FString FilePath;
    FString OriginalContent;  // For restore
    
    // Serialized state for complex undo
    FString SerializedState;
    
    // Timestamp
    FDateTime Timestamp;
    
    FTransactionOperation()
        : Timestamp(FDateTime::Now())
    {}
};

/**
 * A transaction is a group of operations that can be undone together
 */
struct RIFTBORNAI_API FAgentTransaction
{
    FGuid TransactionId;
    FString Name;
    FString Description;
    
    TArray<FTransactionOperation> Operations;
    
    FDateTime StartTime;
    FDateTime EndTime;
    
    bool bCommitted = false;
    bool bRolledBack = false;
    
    FAgentTransaction()
        : TransactionId(FGuid::NewGuid())
        , StartTime(FDateTime::Now())
    {}
    
    int32 OperationCount() const { return Operations.Num(); }
};

/**
 * Transaction Manager
 */
class RIFTBORNAI_API FAgentTransactionManager
{
public:
    static FAgentTransactionManager& Get();
    
    /** Begin a new transaction */
    FGuid BeginTransaction(const FString& Name, const FString& Description = TEXT(""));
    
    /** Record an operation in the current transaction */
    void RecordOperation(const FTransactionOperation& Op);
    
    /** Commit the current transaction (finalize, can still be undone) */
    void CommitTransaction();
    
    /** Rollback the current transaction (undo all operations) */
    bool RollbackTransaction();
    
    /** Undo the last committed transaction */
    bool UndoLastTransaction();
    
    /** Redo the last undone transaction */
    bool RedoTransaction();
    
    /** Get transaction history */
    TArray<FAgentTransaction> GetTransactionHistory(int32 MaxCount = 20) const;
    
    /** Get current transaction (if any) */
    FAgentTransaction* GetCurrentTransaction();
    
    /** Is there an active transaction? */
    bool HasActiveTransaction() const { return CurrentTransaction.IsSet(); }
    
    // =========================================================================
    // PERSISTENCE: Save/load transaction state to disk for crash recovery
    // =========================================================================
    
    /** Save all committed transactions to disk (called on commit and periodically) */
    void SaveTransactionsToDisk();
    
    /** Load transactions from disk on startup (crash recovery) */
    void LoadTransactionsFromDisk();
    
    /** Get the path where transaction state is persisted */
    static FString GetPersistencePath();
    
    // Convenience methods for recording operations
    void RecordAssetCreated(const FString& AssetPath, const FString& AssetClass);
    void RecordAssetModified(const FString& AssetPath, const FString& OriginalState);
    void RecordActorSpawned(const FGuid& ActorGuid, const FString& ActorClass, const FTransform& Transform);
    void RecordActorSpawnedWithLabel(const FString& ActorLabel, const FString& ActorClass, const FTransform& Transform);
    void RecordActorDestroyed(AActor* Actor);  // Serializes actor state before destruction
    void RecordFileCreated(const FString& FilePath);
    void RecordFileModified(const FString& FilePath, const FString& OriginalContent);
    
    // Tool implementations
    static FClaudeToolResult Tool_BeginTransaction(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CommitTransaction(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_RollbackTransaction(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_UndoLastOperation(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetTransactionHistory(const FClaudeToolCall& Call);
    
private:
    FAgentTransactionManager() = default;

    mutable FCriticalSection TransactionLock;
    TOptional<FAgentTransaction> CurrentTransaction;
    TArray<FAgentTransaction> CommittedTransactions;
    TArray<FAgentTransaction> UndoneTransactions; // For redo
    
    // Undo helpers
    bool UndoOperation(const FTransactionOperation& Op);
    bool UndoAssetCreation(const FString& AssetPath);
    bool UndoAssetModification(const FString& AssetPath, const FString& SerializedState);
    bool UndoActorSpawn(const FGuid& ActorGuid, const FString& Description, const FString& ActorLabel);
    bool UndoActorDestruction(const FTransactionOperation& Op);  // Respawns destroyed actor
    bool UndoActorModification(const FGuid& ActorGuid, const FString& SerializedState, const FTransform& OriginalTransform);
    bool UndoFileCreation(const FString& FilePath);
    bool RestoreFile(const FString& FilePath, const FString& Content);
};
