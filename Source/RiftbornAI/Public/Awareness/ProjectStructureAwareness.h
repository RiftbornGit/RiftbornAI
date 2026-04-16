// ProjectStructureAwareness.h - Project organization understanding for Agents

#pragma once

#include "CoreMinimal.h"
#include "ProjectStructureAwareness.generated.h"

UENUM(BlueprintType)
enum class ECodeType : uint8
{
    CppHeader,
    CppSource,
    Blueprint,
    DataTable,
    Config,
    Shader,
    Python,
    Unknown
};

UENUM(BlueprintType)
enum class EProjectAssetCategory : uint8
{
    Core,           // Base classes, interfaces
    Character,      // Player, NPCs
    AI,             // AI controllers, BT, EQS
    Weapon,         // Weapons, projectiles
    Ability,        // GAS abilities, effects
    UI,             // Widgets, HUD
    Level,          // Maps, streaming
    Environment,    // Props, foliage, skybox
    Audio,          // Sound cues, music
    VFX,            // Particles, materials
    Animation,      // Sequences, montages
    Data,           // DataTables, DataAssets
    System,         // GameMode, GameState
    Input,          // Input actions, contexts
    Unknown
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FModuleInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString ModuleName;
    UPROPERTY() FString ModulePath;
    UPROPERTY() FString ModuleType;  // Runtime, Editor, Developer
    UPROPERTY() TArray<FString> PublicDependencies;
    UPROPERTY() TArray<FString> PrivateDependencies;
    UPROPERTY() int32 HeaderCount = 0;
    UPROPERTY() int32 SourceCount = 0;
    UPROPERTY() int32 ClassCount = 0;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FClassInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString ClassName;
    UPROPERTY() FString ParentClass;
    UPROPERTY() ECodeType CodeType = ECodeType::Unknown;
    UPROPERTY() EProjectAssetCategory Category = EProjectAssetCategory::Unknown;
    UPROPERTY() FString FilePath;
    UPROPERTY() FString Module;
    UPROPERTY() TArray<FString> Interfaces;
    UPROPERTY() bool bIsAbstract = false;
    UPROPERTY() bool bIsBlueprintable = false;
    UPROPERTY() FString Description;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FDataTableInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString TableName;
    UPROPERTY() FString AssetPath;
    UPROPERTY() FString RowStruct;
    UPROPERTY() FString RowStructName;  // Alias for compatibility
    UPROPERTY() int32 RowCount = 0;
    UPROPERTY() TArray<FString> ColumnNames;
    UPROPERTY() TArray<FString> RowNames;  // Names of all rows
    UPROPERTY() EProjectAssetCategory Category = EProjectAssetCategory::Data;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FConfigInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString ConfigName;
    UPROPERTY() FString FilePath;
    UPROPERTY() FString ConfigType;  // Game, Engine, Editor, Input, etc.
    UPROPERTY() FString FileName;
    UPROPERTY() TArray<FString> Sections;
    UPROPERTY() TMap<FString, int32> KeyValueCounts;  // Section -> key count
    UPROPERTY() int32 SettingCount = 0;
};

// Alias for compatibility
typedef FConfigInfo FConfigFileInfo;

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FSourceFileInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString FileName;
    UPROPERTY() FString RelativePath;
    UPROPERTY() FString ModuleName;
    UPROPERTY() int32 LineCount = 0;
    UPROPERTY() FDateTime LastModified;
    UPROPERTY() TArray<FString> Classes;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FProjectModuleInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString ProjectName;
    UPROPERTY() FString ProjectPath;
    UPROPERTY() FString EngineVersion;
    UPROPERTY() TArray<FString> GameModules;
    UPROPERTY() TArray<FString> PluginModules;
    UPROPERTY() TArray<FString> EngineModules;
    // ModuleDependencies stored as non-UPROPERTY to avoid TMap<FString,TArray> UHT limitation
    TMap<FString, TArray<FString>> ModuleDependencies;
};

// Use engine's EBuildTargetType from GenericPlatformMisc.h
// Note: Values are Game, Editor, Client, Server, Program

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FBuildTargetInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString TargetName;
    UPROPERTY() FString TargetFilePath;
    UPROPERTY() TArray<FString> SupportedPlatforms;
    UPROPERTY() TArray<FString> SupportedConfigurations;
    // Target type stored as int for flexibility
    UPROPERTY() int32 TargetType = 0;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FPluginInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString PluginName;
    UPROPERTY() FString FriendlyName;
    UPROPERTY() FString Version;
    UPROPERTY() FString Category;
    UPROPERTY() FString Description;
    UPROPERTY() FString CreatedBy;
    UPROPERTY() bool bIsEnabled = false;
    UPROPERTY() bool bIsInstalled = false;
    UPROPERTY() bool bIsEnginePlugin = false;
    UPROPERTY() TArray<FString> Modules;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FContentFolderInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString FolderName;
    UPROPERTY() FString FolderPath;
    UPROPERTY() int32 AssetCount = 0;
    UPROPERTY() int32 TotalAssetCount = 0;  // Including children
    UPROPERTY() TMap<FName, int32> AssetTypeCount;
    // ChildFolders is non-UPROPERTY to avoid recursive struct limitation
    TArray<FContentFolderInfo> ChildFolders;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FSourceSearchResult
{
    GENERATED_BODY()
    
    UPROPERTY() FString FilePath;
    UPROPERTY() int32 LineNumber = 0;
    UPROPERTY() int32 ColumnNumber = 0;
    UPROPERTY() FString LineContent;
    UPROPERTY() FString ContextBefore;
    UPROPERTY() FString ContextAfter;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FFolderInfo
{
    GENERATED_BODY()
    
    UPROPERTY() FString FolderPath;
    UPROPERTY() FString FolderName;
    UPROPERTY() EProjectAssetCategory Category = EProjectAssetCategory::Unknown;
    UPROPERTY() int32 AssetCount = 0;
    UPROPERTY() int32 SubfolderCount = 0;
    UPROPERTY() TArray<FString> AssetTypes;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FProjectStructure
{
    GENERATED_BODY()
    
    UPROPERTY() FString ProjectName;
    UPROPERTY() FString ProjectPath;
    UPROPERTY() FString EngineVersion;
    UPROPERTY() TArray<FModuleInfo> Modules;
    UPROPERTY() TArray<FFolderInfo> ContentFolders;
    UPROPERTY() TArray<FConfigInfo> Configs;
    UPROPERTY() int32 TotalClasses = 0;
    UPROPERTY() int32 TotalBlueprints = 0;
    UPROPERTY() int32 TotalAssets = 0;
};


class RIFTBORNAI_API FProjectStructureAwareness
{
public:
    static FProjectStructureAwareness& Get();
    
    // Project overview
    FProjectStructure GetProjectStructure() const;
    FString GetProjectName() const;
    FString GetProjectPath() const;
    FString GetEngineVersion() const;
    
    // Modules
    TArray<FModuleInfo> GetModules() const;
    FModuleInfo GetModuleInfo(const FString& ModuleName) const;
    TArray<FString> GetModuleDependencies(const FString& ModuleName) const;
    
    // Classes
    TArray<FClassInfo> GetClasses() const;
    TArray<FClassInfo> GetClassesByCategory(EProjectAssetCategory Category) const;
    TArray<FClassInfo> GetClassesByParent(const FString& ParentClass) const;
    FClassInfo GetClassInfo(const FString& ClassName) const;
    FString GetClassFilePath(const FString& ClassName) const;
    bool IsClassBlueprintable(const FString& ClassName) const;
    
    // Blueprints
    TArray<FClassInfo> GetBlueprints() const;
    TArray<FClassInfo> GetBlueprintsByCategory(EProjectAssetCategory Category) const;
    FString GetBlueprintParentClass(const FString& BlueprintName) const;
    
    // Data Tables
    TArray<FDataTableInfo> GetDataTables() const;
    FDataTableInfo GetDataTableInfo(const FString& TableName) const;
    TArray<FString> GetDataTableRows(const FString& TableName) const;
    
    // Configs
    TArray<FConfigInfo> GetConfigs() const;
    TArray<FConfigInfo> GetConfigFiles() const { return GetConfigs(); }  // Alias
    FString GetConfigValue(const FString& ConfigName, const FString& Section, const FString& Key) const;
    
    // Source Files
    TArray<FSourceFileInfo> GetSourceFiles(const FString& ModuleName = TEXT("")) const;
    
    // Project Modules (extended info)
    FProjectModuleInfo GetProjectModules() const;
    
    // Build Targets
    TArray<FBuildTargetInfo> GetBuildTargets() const;
    
    // Plugins
    TArray<FPluginInfo> GetPlugins() const;
    
    // Content Structure
    FContentFolderInfo GetContentStructure(const FString& StartPath, int32 MaxDepth) const;
    
    // Source Code Search
    TArray<FSourceSearchResult> SearchSourceCode(const FString& Query, const FString& FilePattern, bool bCaseSensitive, int32 MaxResults) const;
    
    // Folders
    TArray<FFolderInfo> GetContentFolders() const;
    FFolderInfo GetFolderInfo(const FString& FolderPath) const;
    EProjectAssetCategory GuessFolderCategory(const FString& FolderPath) const;
    FString GetRecommendedFolder(EProjectAssetCategory Category) const;
    
    // Naming conventions
    FString SuggestClassName(const FString& Description, EProjectAssetCategory Category) const;
    FString SuggestAssetName(const FString& Description, const FString& AssetType) const;
    bool ValidateNamingConvention(const FString& Name, ECodeType CodeType) const;
    
    // Utility
    static FString CodeTypeToString(ECodeType Type);
    static FString CategoryToString(EProjectAssetCategory Category);
    static EProjectAssetCategory ParseCategory(const FString& Str);
    
    // World-aware overload
    static FProjectStructureAwareness& Get(UWorld* World) { return Get(); }
    
private:
    FProjectStructureAwareness();
    void RefreshCache() const;
    
    mutable FProjectStructure CachedStructure;
    mutable double LastCacheTime = 0.0;
};
