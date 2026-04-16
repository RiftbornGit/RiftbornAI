// UEEditorIntegration.h
// REAL UE Editor control - actually executes commands
// No faking - uses real UE APIs to manipulate the editor

#pragma once

#include "CoreMinimal.h"
#include "Editor.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/Blueprint.h"

DECLARE_DELEGATE_TwoParams(FOnEditorActionComplete, bool /*bSuccess*/, const FString& /*Result*/);

/**
 * Result of an editor action
 */
struct FEditorActionResult
{
    bool bSuccess = false;
    FString Message;
    FString CreatedAssetPath;
    TArray<FString> Warnings;
    TArray<FString> Errors;
    float ExecutionTime = 0.0f;
};

/**
 * Blueprint creation parameters
 */
struct FBlueprintCreationParams
{
    FString AssetName;
    FString AssetPath = TEXT("/Game/Blueprints");
    FString ParentClassName;
    UClass* ParentClass = nullptr;
    
    // Initial components to add
    TArray<FString> ComponentClasses;
    
    // Initial variables
    TMap<FString, FString> Variables;  // Name -> Type
    
    // Open in editor after creation
    bool bOpenInEditor = true;
};

/**
 * C++ class creation parameters
 */
struct FCppClassCreationParams
{
    FString ClassName;
    FString BaseClass;
    FString ModuleName;
    
    // Code content
    FString HeaderContent;
    FString SourceContent;
    
    // Location
    FString TargetPath;  // Empty = default module location
    
    // Generation settings
    bool bAddToProject = true;
    bool bTriggerCompile = false;
};

/**
 * Level manipulation parameters
 */
struct FLevelActionParams
{
    FString LevelPath;
    FString ActorClass;
    FVector Location = FVector::ZeroVector;
    FRotator Rotation = FRotator::ZeroRotator;
    FVector Scale = FVector::OneVector;
    FString ActorLabel;
};

/**
 * UE Editor Integration
 * REAL control over Unreal Editor - not faking or simulating
 */
class RIFTBORNAI_API FUEEditorIntegration
{
public:
    FUEEditorIntegration();
    ~FUEEditorIntegration();
    
    // === Blueprint Operations ===
    
    /** Create a new Blueprint asset */
    FEditorActionResult CreateBlueprint(const FBlueprintCreationParams& Params);
    
    /** Open a Blueprint in the editor */
    FEditorActionResult OpenBlueprint(const FString& AssetPath);
    
    /** Compile a Blueprint */
    FEditorActionResult CompileBlueprint(const FString& AssetPath);
    
    /** Add a component to a Blueprint */
    FEditorActionResult AddComponentToBlueprint(const FString& AssetPath, const FString& ComponentClass);
    
    /** Add a variable to a Blueprint */
    FEditorActionResult AddVariableToBlueprint(const FString& AssetPath, const FString& VarName, const FString& VarType);
    
    /** Delete a Blueprint */
    FEditorActionResult DeleteBlueprint(const FString& AssetPath);
    
    /** Duplicate a Blueprint */
    FEditorActionResult DuplicateBlueprint(const FString& SourcePath, const FString& NewName, const FString& DestPath);
    
    // === C++ Operations ===
    
    /** Create new C++ class files */
    FEditorActionResult CreateCppClass(const FCppClassCreationParams& Params);
    
    /** Regenerate project files */
    FEditorActionResult RegenerateProjectFiles();
    
    /** Trigger hot reload / Live Coding */
    FEditorActionResult TriggerHotReload();
    
    /** Write to source file */
    FEditorActionResult WriteSourceFile(const FString& FilePath, const FString& Content);
    
    /** Read source file */
    FString ReadSourceFile(const FString& FilePath);
    
    // === Asset Operations ===
    
    /** Create any asset */
    FEditorActionResult CreateAsset(const FString& AssetName, const FString& AssetPath, UClass* AssetClass);
    
    /** Delete any asset */
    FEditorActionResult DeleteAsset(const FString& AssetPath);
    
    /** Rename asset */
    FEditorActionResult RenameAsset(const FString& OldPath, const FString& NewName);
    
    /** Move asset */
    FEditorActionResult MoveAsset(const FString& OldPath, const FString& NewPath);
    
    /** Duplicate asset */
    FEditorActionResult DuplicateAsset(const FString& SourcePath, const FString& DestPath, const FString& NewName);
    
    /** Save asset */
    FEditorActionResult SaveAsset(const FString& AssetPath);
    
    /** Save all dirty assets */
    FEditorActionResult SaveAllAssets();
    
    /** Import asset from file */
    FEditorActionResult ImportAsset(const FString& SourceFile, const FString& DestPath);
    
    // === Level Operations ===
    
    /** Load a level */
    FEditorActionResult LoadLevel(const FString& LevelPath);
    
    /** Create new level */
    FEditorActionResult CreateLevel(const FString& LevelPath, const FString& TemplatePath = TEXT(""));
    
    /** Save current level */
    FEditorActionResult SaveCurrentLevel();
    
    /** Spawn actor in level */
    FEditorActionResult SpawnActor(const FLevelActionParams& Params);
    
    /** Delete selected actors */
    FEditorActionResult DeleteSelectedActors();
    
    /** Select actor by label */
    FEditorActionResult SelectActor(const FString& ActorLabel);
    
    /** Get selected actors */
    TArray<AActor*> GetSelectedActors();
    
    /** Duplicate selected actors */
    FEditorActionResult DuplicateSelectedActors();
    
    // === Editor State ===
    
    /** Get current level name */
    FString GetCurrentLevelName();
    
    /** Get current level path */
    FString GetCurrentLevelPath();
    
    /** Is PIE running */
    bool IsPIERunning();
    
    /** Start PIE */
    FEditorActionResult StartPIE();
    
    /** Stop PIE */
    FEditorActionResult StopPIE();
    
    /** Is compiling */
    bool IsCompiling();
    
    /** Get last compilation errors */
    TArray<FString> GetCompilationErrors();
    
    // === Content Browser ===
    
    /** Navigate Content Browser to path */
    FEditorActionResult NavigateContentBrowser(const FString& Path);
    
    /** Refresh Content Browser */
    FEditorActionResult RefreshContentBrowser();
    
    // === Console Commands ===
    
    /** Execute console command */
    FEditorActionResult ExecuteConsoleCommand(const FString& Command);
    
    // === Project Settings ===
    
    /** Get project setting value */
    FString GetProjectSetting(const FString& Category, const FString& Section, const FString& Key);
    
    /** Set project setting value */
    FEditorActionResult SetProjectSetting(const FString& Category, const FString& Section, const FString& Key, const FString& Value);
    
    // === Utilities ===
    
    /** Check if asset exists */
    bool DoesAssetExist(const FString& AssetPath);
    
    /** Get asset class */
    UClass* GetAssetClass(const FString& AssetPath);
    
    /** Load asset synchronously */
    UObject* LoadAssetSync(const FString& AssetPath);
    
    /** Get valid asset name */
    FString SanitizeAssetName(const FString& Name);
    
    /** Get unique asset path */
    FString GetUniqueAssetPath(const FString& BasePath, const FString& BaseName);
    
private:
    /** Helper to create folder structure */
    bool EnsureFolderExists(const FString& Path);
    
    /** Get Asset Tools */
    class IAssetTools& GetAssetTools();
    
    /** Get Level Editor */
    class FLevelEditorModule& GetLevelEditor();
    
    /** Get Content Browser */
    class FContentBrowserModule& GetContentBrowser();
};

/**
 * UE Editor Action Queue
 * Executes editor actions safely on game thread
 */
class RIFTBORNAI_API FEditorActionQueue
{
public:
    static FEditorActionQueue& Get();
    
    /** Queue an action to execute on game thread */
    void QueueAction(TFunction<void()> Action);
    
    /** Queue action and wait for completion */
    void QueueActionAndWait(TFunction<void()> Action);
    
    /** Process pending actions (call from tick) */
    void ProcessQueue();
    
    /** Is queue empty */
    bool IsEmpty() const;
    
private:
    FEditorActionQueue() = default;
    
    TArray<TFunction<void()>> PendingActions;
    mutable FCriticalSection QueueLock;
};
