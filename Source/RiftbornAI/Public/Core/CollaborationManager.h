// Copyright RiftbornAI. All Rights Reserved.
// Collaborative Features - ENH-009: Multi-user support with locking and conflict resolution

#pragma once

#include "CoreMinimal.h"

/**
 * Lock information for an asset
 */
struct RIFTBORNAI_API FAssetLockInfo
{
	/** Path to the locked asset */
	FString AssetPath;
	
	/** User who holds the lock */
	FString LockedBy;
	
	/** Machine/hostname where lock was acquired */
	FString MachineName;
	
	/** When the lock was acquired */
	FDateTime LockedAt;
	
	/** Optional lock reason */
	FString Reason;
	
	/** Is this a hard lock (blocks) or soft lock (warns) */
	bool bIsHardLock = false;
	
	/** Lock expiration (0 = never expires) */
	FDateTime ExpiresAt;
	
	/** Is the lock still valid */
	bool IsValid() const
	{
		if (ExpiresAt.GetTicks() > 0 && FDateTime::Now() > ExpiresAt)
		{
			return false;
		}
		return !LockedBy.IsEmpty();
	}
};

/**
 * Conflict information between two versions
 */
struct RIFTBORNAI_API FAssetConflict
{
	/** Asset path */
	FString AssetPath;
	
	/** Local version info */
	FString LocalVersion;
	FDateTime LocalModified;
	FString LocalUser;
	
	/** Remote version info */
	FString RemoteVersion;
	FDateTime RemoteModified;
	FString RemoteUser;
	
	/** Conflict type */
	enum class EConflictType
	{
		BothModified,      // Both sides modified
		DeletedLocally,    // Deleted locally, modified remotely
		DeletedRemotely,   // Modified locally, deleted remotely
		TypeMismatch       // Asset type changed
	};
	EConflictType Type = EConflictType::BothModified;
	
	/** Human-readable description */
	FString GetDescription() const;
};

/**
 * Collaborative Features Manager
 * 
 * Enables team collaboration by providing:
 * - Asset locking to prevent edit conflicts
 * - Lock status tracking and notification
 * - Conflict detection and resolution tools
 * 
 * ENH-009: Collaborative Features
 */
class RIFTBORNAI_API FCollaborationManager
{
public:
	static FCollaborationManager& Get();
	
	// =========================================================================
	// Asset Locking
	// =========================================================================
	
	/**
	 * Acquire a lock on an asset
	 * @param AssetPath - Asset to lock
	 * @param Reason - Optional reason for lock
	 * @param bHardLock - Hard lock blocks, soft lock warns
	 * @param ExpirationMinutes - Lock expiration (0 = no expiration)
	 * @return True if lock acquired
	 */
	bool LockAsset(const FString& AssetPath, const FString& Reason = TEXT(""), bool bHardLock = false, int32 ExpirationMinutes = 0);
	
	/**
	 * Release a lock on an asset
	 * @param AssetPath - Asset to unlock
	 * @param bForce - Force unlock even if locked by another user
	 * @return True if unlocked
	 */
	bool UnlockAsset(const FString& AssetPath, bool bForce = false);
	
	/**
	 * Check if an asset is locked
	 * @param AssetPath - Asset to check
	 * @return Lock info (empty LockedBy if not locked)
	 */
	FAssetLockInfo GetLockStatus(const FString& AssetPath) const;
	
	/**
	 * Get all current locks
	 */
	TArray<FAssetLockInfo> GetAllLocks() const;
	
	/**
	 * Get locks held by current user
	 */
	TArray<FAssetLockInfo> GetMyLocks() const;
	
	/**
	 * Check if current user can edit an asset
	 * @param AssetPath - Asset to check
	 * @param OutBlocker - If blocked, who is blocking
	 * @return True if can edit
	 */
	bool CanEditAsset(const FString& AssetPath, FString& OutBlocker) const;
	
	/**
	 * Extend a lock's expiration
	 * @param AssetPath - Asset to extend lock for
	 * @param AdditionalMinutes - Minutes to add
	 * @return True if extended
	 */
	bool ExtendLock(const FString& AssetPath, int32 AdditionalMinutes);
	
	// =========================================================================
	// Conflict Detection
	// =========================================================================
	
	/**
	 * Check for conflicts with remote version
	 * @param AssetPath - Asset to check
	 * @return Conflict info (empty if no conflict)
	 */
	FAssetConflict CheckForConflict(const FString& AssetPath) const;
	
	/**
	 * Get all current conflicts
	 */
	TArray<FAssetConflict> GetAllConflicts() const;
	
	/**
	 * Mark a conflict as resolved
	 * @param AssetPath - Asset with conflict
	 * @param Resolution - How it was resolved (local, remote, merged)
	 */
	void MarkConflictResolved(const FString& AssetPath, const FString& Resolution);
	
	// =========================================================================
	// User Management
	// =========================================================================
	
	/**
	 * Get current user name
	 */
	FString GetCurrentUser() const;
	
	/**
	 * Get current machine name
	 */
	FString GetMachineName() const;
	
	/**
	 * Get list of active collaborators (users with locks)
	 */
	TArray<FString> GetActiveCollaborators() const;
	
	// =========================================================================
	// Notifications
	// =========================================================================
	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAssetLocked, const FAssetLockInfo&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAssetUnlocked, const FString&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnConflictDetected, const FAssetConflict&);
	
	FOnAssetLocked OnAssetLocked;
	FOnAssetUnlocked OnAssetUnlocked;
	FOnConflictDetected OnConflictDetected;
	
	// =========================================================================
	// Persistence
	// =========================================================================
	
	bool SaveLockState();
	bool LoadLockState();
	
	/** Clean up expired locks */
	void CleanupExpiredLocks();
	
private:
	FCollaborationManager();
	
	/** Active locks */
	TMap<FString, FAssetLockInfo> Locks;
	
	/** Known conflicts */
	TMap<FString, FAssetConflict> Conflicts;
	
	/** Path to lock file */
	FString GetLockFilePath() const;
	
	/** Cached user/machine info */
	mutable FString CachedUserName;
	mutable FString CachedMachineName;
};
