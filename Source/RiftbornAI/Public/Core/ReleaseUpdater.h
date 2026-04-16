// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Widgets/Notifications/SNotificationList.h"

struct RIFTBORNAI_API FRiftbornReleaseAsset
{
	FString Name;
	FString DownloadUrl;
	int64 SizeBytes = 0;
};

struct RIFTBORNAI_API FRiftbornReleaseInfo
{
	FString Version;
	FString TagName;
	FString Title;
	FString HtmlUrl;
	bool bPrerelease = false;
	TArray<FRiftbornReleaseAsset> Assets;
	FRiftbornReleaseAsset PreferredAsset;

	bool IsValid() const
	{
		return !Version.IsEmpty() && !PreferredAsset.DownloadUrl.IsEmpty();
	}
};

class RIFTBORNAI_API FRiftbornReleaseUpdater
{
public:
	static FRiftbornReleaseUpdater& Get();

	void Initialize();
	void Shutdown();
	void CheckForUpdates(bool bForce = false);

	static FString NormalizeVersionString(const FString& RawVersion);
	static int32 CompareVersions(const FString& LeftVersion, const FString& RightVersion);
	static bool ShouldCheckForUpdates(const FString& LastCheckUtcIso8601, int32 IntervalHours, const FDateTime& NowUtc);
	static bool HasPendingUpdateReady(const FString& CurrentVersion, const FString& PendingVersion, const FString& PendingZipPath);
	static bool TrySelectReleaseAsset(const TArray<FRiftbornReleaseAsset>& Assets, const FString& EngineVersionToken, FRiftbornReleaseAsset& OutAsset);
	static bool TryParseGitHubReleasesResponse(
		const FString& ResponseBody,
		const FString& CurrentVersion,
		const FString& EngineVersionToken,
		bool bIncludePrereleases,
		const FString& SkippedVersion,
		FRiftbornReleaseInfo& OutRelease);

private:
	bool RunDeferredCheck(float DeltaTime);
	void StartDownload(const FRiftbornReleaseInfo& Release);
	void QueueInstallerForNextRestart();
	void LaunchInstallerAndRestart();
	void SkipRelease(const FString& Version);
	void OpenReleaseNotes(const FString& Url) const;
	void PersistPendingUpdateState(const FRiftbornReleaseInfo& Release, const FString& ZipPath) const;
	void ClearPendingUpdateState() const;
	void ShowUpdateAvailableNotification(const FRiftbornReleaseInfo& Release);
	void ShowDownloadReadyNotification(const FRiftbornReleaseInfo& Release);
	void ShowPendingNotification(const FString& Message);
	void ShowTransientNotification(const FString& Message, SNotificationItem::ECompletionState CompletionState, float ExpireDuration = 6.0f) const;
	void DismissActiveNotification(SNotificationItem::ECompletionState CompletionState);
	FString GetCurrentVersion() const;
	FString GetEngineVersionToken() const;
	FString GetReleaseRepository() const;

	bool bInitialized = false;
	bool bCheckingForUpdates = false;
	bool bDownloadingUpdate = false;
	bool bInstallerQueued = false;
	FTSTicker::FDelegateHandle DeferredCheckHandle;
	TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> ActiveRequest;
	TSharedPtr<class SNotificationItem> ActiveNotification;
	FRiftbornReleaseInfo PendingRelease;
	FString PendingDownloadedZipPath;
};
