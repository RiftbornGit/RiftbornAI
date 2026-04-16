// AssetImportWatcher.h - Detects and auto-analyzes imported/dragged-in assets
// Hooks into UE's AssetRegistry to detect new assets and trigger visual analysis

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AssetRegistry/AssetRegistryModule.h"

// Forward declarations
struct FAssetData;

/**
 * Asset Import Watcher
 * 
 * Monitors the Asset Registry for:
 * 1. New assets being imported (drag & drop, import dialogs)
 * 2. Assets being added programmatically
 * 3. Assets being modified
 * 4. Assets being deleted
 * 
 * When enabled, automatically triggers visual analysis on new visual assets
 * (meshes, textures, materials) so agents always understand what's in the project.
 */
class RIFTBORNAI_API FAssetImportWatcher
{
public:
    static FAssetImportWatcher& Get();
    
    /** Start watching for asset changes */
    void StartWatching();
    
    /** Stop watching */
    void StopWatching();
    
    /** Check if currently watching */
    bool IsWatching() const { return bIsWatching; }
    
    /** Enable/disable auto-analysis of new assets */
    void SetAutoAnalyzeEnabled(bool bEnable) { bAutoAnalyzeEnabled = bEnable; }
    bool IsAutoAnalyzeEnabled() const { return bAutoAnalyzeEnabled; }
    
    /** Get statistics */
    struct FWatcherStats
    {
        int32 TotalAssetsDetected;
        int32 AssetsAnalyzed;
        int32 AssetsQueued;
        int32 AssetsFailed;
    };
    FWatcherStats GetStats() const { return Stats; }
    
    /** Manually trigger analysis of recently imported assets */
    void AnalyzePendingAssets();
    
    /** Delegate fired when a new asset is analyzed */
    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAssetAnalyzed, const FString& /*AssetPath*/, bool /*bSuccess*/);
    FOnAssetAnalyzed OnAssetAnalyzed;
    
private:
    FAssetImportWatcher();
    ~FAssetImportWatcher();
    
    // Asset Registry event handlers
    void OnAssetAdded(const FAssetData& AssetData);
    void OnAssetRemoved(const FAssetData& AssetData);
    void OnAssetUpdated(const FAssetData& AssetData);
    void OnAssetRenamed(const FAssetData& AssetData, const FString& OldObjectPath);
    void OnFilesLoaded();
    
    // Internal
    bool ShouldAnalyzeAsset(const FAssetData& AssetData) const;
    void QueueAssetForAnalysis(const FString& AssetPath);
    void ProcessAnalysisQueue();
    
    // State
    bool bIsWatching = false;
    bool bAutoAnalyzeEnabled = false;  // Disabled by default - requires Ollama with vision model
    
    // Delegate handles
    FDelegateHandle OnAssetAddedHandle;
    FDelegateHandle OnAssetRemovedHandle;
    FDelegateHandle OnAssetUpdatedHandle;
    FDelegateHandle OnAssetRenamedHandle;
    FDelegateHandle OnFilesLoadedHandle;
    
    // Analysis queue
    TArray<FString> PendingAnalysisQueue;
    FCriticalSection QueueLock;
    bool bProcessingQueue = false;
    
    // Stats
    FWatcherStats Stats;
};
