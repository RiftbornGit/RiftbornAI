// Copyright RiftbornAI. All Rights Reserved.
// Lightweight async git-info provider for the copilot panel footer.
// Shells out to `git` via FPlatformProcess::ExecProcess on a worker thread,
// marshals result back to game thread. 30s TTL cache.

#pragma once

#include "CoreMinimal.h"

struct FCopilotGitInfo
{
	bool    bIsRepo      = false;
	bool    bGhAvailable = false;
	FString Branch;          // e.g. "main"
	FString Upstream;        // e.g. "origin/main" (may be empty)
	FString RemoteUrl;       // e.g. git@github.com:owner/repo.git
	int32   AddedLines   = 0;
	int32   RemovedLines = 0;
	FDateTime FetchedAt;
};

class FCopilotGitInfoProvider
{
public:
	using FResultCallback = TFunction<void(const FCopilotGitInfo&)>;

	/** Kick off an async refresh. OnDone fires on the game thread. */
	static void RefreshAsync(FResultCallback OnDone);

	/** Return the last-fetched info (may be empty if never refreshed). */
	static FCopilotGitInfo GetCached();

	/** Open a Create PR flow — prefers `gh pr create --web`, falls back to compare URL. */
	static void OpenCreatePR();

	static constexpr double CacheTTLSeconds = 30.0;
};
