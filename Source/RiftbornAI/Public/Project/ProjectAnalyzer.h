// ProjectAnalyzer.h
// Scans and understands the actual UE project structure
// This is REAL analysis - not faking or guessing

#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/KismetEditorUtilities.h"

/**
 * Information about a C++ class in the project
 */
struct FProjectClass
{
    FString ClassName;
    FString BaseClassName;
    FString HeaderPath;
    FString SourcePath;
    FString ModuleName;
    
    // Reflection info
    TArray<FString> Properties;
    TArray<FString> Functions;
    TArray<FString> Events;
    
    // Metadata
    bool bIsBlueprinted = false;
    bool bIsAbstract = false;
    FString Category;
};

/**
 * Information about a Blueprint asset
 */
struct FProjectBlueprint
{
    FString AssetPath;
    FString AssetName;
    FString ParentClassName;
    
    // Components
    TArray<FString> Components;
    
    // Variables
    TArray<FString> Variables;
    
    // Functions/Events
    TArray<FString> Functions;
    TArray<FString> CustomEvents;
    
    // Graph complexity
    int32 TotalNodes = 0;
    int32 TotalConnections = 0;
};

/**
 * Information about a level/map
 */
struct FProjectLevel
{
    FString LevelPath;
    FString LevelName;
    
    // Actors in level
    TArray<FString> ActorClasses;
    int32 TotalActors = 0;
    
    // Level info
    bool bIsPersistent = false;
    TArray<FString> SubLevels;
};

/**
 * Complete project snapshot
 */
struct FProjectSnapshot
{
    // Project info
    FString ProjectName;
    FString ProjectPath;
    FString EngineVersion;
    
    // Modules
    TArray<FString> GameModules;
    TArray<FString> PluginModules;
    
    // Assets by type
    TMap<FString, int32> AssetCounts;  // Type -> Count
    
    // Key classes
    TArray<FProjectClass> Classes;
    TArray<FProjectBlueprint> Blueprints;
    TArray<FProjectLevel> Levels;
    
    // Quick lookups
    TMap<FString, int32> ClassNameToIndex;
    TMap<FString, int32> BlueprintPathToIndex;
    
    // Stats
    FDateTime SnapshotTime;
    float ScanDurationSeconds = 0.0f;
};

/**
 * File change information
 */
struct FRiftbornFileChange
{
    FString FilePath;
    FString ChangeType;  // "Created", "Modified", "Deleted"
    FDateTime Timestamp;
    int32 LinesAdded = 0;
    int32 LinesRemoved = 0;
    FString PatchRef;
};

/**
 * Project Analyzer - REAL project scanning
 * No faking - actually reads and parses project files
 */
class RIFTBORNAI_API FProjectAnalyzer
{
public:
    FProjectAnalyzer();
    ~FProjectAnalyzer();
    
    // === Project Scanning ===
    
    /** Full project scan - may take time */
    FProjectSnapshot ScanProject(const FString& ProjectPath);
    
    /** Quick scan - just essentials */
    FProjectSnapshot QuickScan(const FString& ProjectPath);
    
    /** Incremental update based on file changes */
    void UpdateSnapshot(FProjectSnapshot& Snapshot, const TArray<FRiftbornFileChange>& Changes);
    
    // === Class Analysis ===
    
    /** Scan all C++ classes */
    TArray<FProjectClass> ScanCppClasses(const FString& SourcePath);
    
    /** Parse a single header file */
    FProjectClass ParseHeaderFile(const FString& HeaderPath);
    
    /** Find class by name */
    const FProjectClass* FindClass(const FProjectSnapshot& Snapshot, const FString& ClassName);
    
    // === Blueprint Analysis ===
    
    /** Scan all Blueprint assets */
    TArray<FProjectBlueprint> ScanBlueprints();
    
    /** Analyze a single Blueprint */
    FProjectBlueprint AnalyzeBlueprint(const FString& AssetPath);
    
    /** Find Blueprint by name */
    const FProjectBlueprint* FindBlueprint(const FProjectSnapshot& Snapshot, const FString& Name);
    
    // === Level Analysis ===
    
    /** Scan all levels */
    TArray<FProjectLevel> ScanLevels();
    
    /** Analyze current level */
    FProjectLevel AnalyzeCurrentLevel();
    
    // === Asset Queries ===
    
    /** Find assets by type */
    TArray<FAssetData> FindAssetsByType(const FString& AssetType);
    
    /** Find assets by name pattern */
    TArray<FAssetData> FindAssetsByName(const FString& Pattern);
    
    /** Get asset dependencies */
    TArray<FString> GetAssetDependencies(const FString& AssetPath);
    
    /** Get asset referencers */
    TArray<FString> GetAssetReferencers(const FString& AssetPath);
    
    // === Code Analysis ===
    
    /** Find all files using a class */
    TArray<FString> FindClassUsages(const FString& ClassName);
    
    /** Find function implementations */
    TArray<FString> FindFunctionImplementations(const FString& FunctionName);
    
    /** Analyze include graph for a file */
    TArray<FString> GetIncludeTree(const FString& HeaderPath);
    
    // === Context Building ===
    
    /** Get relevant context for a query */
    FString GetRelevantContext(const FProjectSnapshot& Snapshot, const FString& Query, int32 MaxTokens = 4000);
    
    /** Build context for class creation */
    FString BuildClassCreationContext(const FProjectSnapshot& Snapshot, const FString& BaseClass);
    
    /** Build context for Blueprint creation */
    FString BuildBlueprintCreationContext(const FProjectSnapshot& Snapshot, const FString& ParentClass);
    
    // === File System ===
    
    /** Watch for file changes */
    void StartFileWatcher(const FString& ProjectPath);
    
    /** Stop file watching */
    void StopFileWatcher();
    
    /** Get pending changes */
    TArray<FRiftbornFileChange> GetPendingChanges();
    
private:
    /** Parse UCLASS declaration */
    void ParseUClassDeclaration(const FString& Line, FProjectClass& OutClass);
    
    /** Parse UPROPERTY declaration */
    void ParseUPropertyDeclaration(const FString& Line, FProjectClass& OutClass);
    
    /** Parse UFUNCTION declaration */
    void ParseUFunctionDeclaration(const FString& Line, FProjectClass& OutClass);
    
    /** Get Asset Registry */
    IAssetRegistry& GetAssetRegistry();
    
    /** Cache for parsed files */
    TMap<FString, FProjectClass> ParsedHeaders;
    
    /** File change queue */
    TArray<FRiftbornFileChange> PendingChanges;
    FCriticalSection ChangeLock;
    
    /** File watcher handle */
    FDelegateHandle DirectoryWatcherHandle;
    bool bIsWatching = false;
};

/**
 * Utility functions for project analysis
 */
namespace ProjectAnalysisUtils
{
    /** Check if path is a UE source file */
    RIFTBORNAI_API bool IsSourceFile(const FString& Path);
    
    /** Check if path is a header file */
    RIFTBORNAI_API bool IsHeaderFile(const FString& Path);
    
    /** Get module name from path */
    RIFTBORNAI_API FString GetModuleFromPath(const FString& Path);
    
    /** Convert asset path to file path */
    RIFTBORNAI_API FString AssetPathToFilePath(const FString& AssetPath);
    
    /** Convert file path to asset path */
    RIFTBORNAI_API FString FilePathToAssetPath(const FString& FilePath);
    
    /** Format code for LLM context */
    RIFTBORNAI_API FString FormatCodeForContext(const FString& Code, int32 MaxLines = 100);
}
