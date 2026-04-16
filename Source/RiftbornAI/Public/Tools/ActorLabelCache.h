// Copyright RiftbornAI. All Rights Reserved.
// O(1) actor-by-label lookup. Replaces TActorIterator linear scan.
// Auto-invalidates on actor add/delete/world change via engine delegates.

#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"

class AActor;
class UWorld;

class RIFTBORNAI_API FActorLabelCache
{
public:
	static FActorLabelCache& Get();

	/** O(1) lookup by exact label. Returns nullptr on miss. */
	AActor* FindByLabel(UWorld* World, const FString& Label);

	/** O(1) case-insensitive lookup. Falls back to prefix match via TActorIterator. */
	AActor* FindByLabelRobust(UWorld* World, const FString& Label);

	/** Force full rebuild on next lookup. */
	void Invalidate();

	/** Hook into engine delegates. Called once from module startup. */
	void BindDelegates();

	/** Unhook delegates. Called from module shutdown. */
	void UnbindDelegates();

private:
	void EnsureBuilt(UWorld* World);
	void OnActorAdded(AActor* Actor);
	void OnActorDeleted(AActor* Actor);
	void OnLevelChanged();
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	TMap<FString, TWeakObjectPtr<AActor>> ExactMap;   // label → actor
	TMap<FString, TWeakObjectPtr<AActor>> LowerMap;   // lowercase label → actor
	TWeakObjectPtr<UWorld> CachedWorld;
	bool bDirty = true;
};
