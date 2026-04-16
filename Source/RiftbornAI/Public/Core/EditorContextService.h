// Copyright RiftbornAI. All Rights Reserved.
// EditorContextService - Central context aggregator for IDE copilot
// MVP: Blueprint + Level only. No scope creep.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "ClaudeToolUse.h"

/**
 * Active editor type - MVP: Blueprint or Level only
 * Everything else is "None" until overlay ships.
 */
UENUM()
enum class EActiveEditorType : uint8
{
	None,               // Unknown or unsupported
	Level,              // Level viewport / World Outliner
	Blueprint,          // Blueprint graph editor (the focus of this MVP)
	Material,           // Material editor
	Niagara,            // Niagara particle system
	UMG,                // UMG/Widget Blueprint
	StaticMesh,         // Static mesh editor
	SkeletalMesh,       // Skeletal mesh editor
	Animation,          // Animation editor
	Sound,              // Sound editor
	Sequencer,          // Sequencer
	DataAsset,          // Data asset
	Texture,            // Texture editor
	CurveEditor         // Curve editor
};

/**
 * Blueprint graph context - detailed state of Blueprint editor
 */
struct RIFTBORNAI_API FBlueprintGraphContext
{
	// === IDENTITY ===
	FString BlueprintPath;          // /Game/Characters/BP_Player
	FString BlueprintName;          // BP_Player
	FString ParentClass;            // Character, Actor, etc.
	FString BlueprintType;          // Normal, Interface, MacroLibrary, FunctionLibrary
	
	// === COMPILE STATE ===
	bool bIsCompiled = false;
	bool bHasErrors = false;
	bool bHasWarnings = false;
	int32 ErrorCount = 0;
	int32 WarningCount = 0;
	TArray<FString> CompileErrors;  // Last N compile errors
	TArray<FString> CompileWarnings;
	
	// === ACTIVE GRAPH ===
	FString ActiveGraphName;        // "EventGraph", "CalculateDamage", "OnHealthChanged"
	FString ActiveGraphType;        // "EventGraph", "FunctionGraph", "MacroGraph", "AnimGraph"
	FGuid ActiveGraphId;
	
	// === NODE SELECTION ===
	int32 SelectedNodeCount = 0;
	TArray<FGuid> SelectedNodeIds;
	TArray<FString> SelectedNodeTypes;  // "K2Node_CallFunction", "K2Node_Event", etc.
	TArray<FString> SelectedNodeNames;  // "PrintString", "BeginPlay", etc.
	TArray<FString> SelectedNodeTitles; // Human-readable: "Print String", "Event Begin Play"
	
	// === PIN CONTEXT (THE CURSOR) ===
	// This is what makes copilot cursor-anchored
	bool bHasPinContext = false;
	FString ContextPinNodeName;
	FString ContextPinName;         // "ReturnValue", "Exec", "Condition"
	FString ContextPinType;         // "exec", "bool", "float", "object", "struct"
	FString ContextPinSubType;      // For objects: "Actor", "Pawn"; for structs: "FVector"
	bool bContextPinIsInput = true;
	bool bContextPinIsConnected = false;
	FGuid ContextPinNodeId;         // GUID of the node owning the pin
	
	// === LOCAL SUBGRAPH (1-hop neighbors) ===
	// Nodes connected to selection - capped at 25
	TArray<FString> UpstreamNodeNames;   // Nodes that feed into selection
	TArray<FString> DownstreamNodeNames; // Nodes that selection feeds into
	int32 LocalSubgraphSize = 0;
	static constexpr int32 MaxSubgraphNodes = 25;
	
	// === AVAILABLE SYMBOLS ===
	TArray<FString> Variables;       // Blueprint variables
	TArray<FString> Functions;       // Blueprint functions
	TArray<FString> Macros;          // Blueprint macros
	TArray<FString> EventDispatchers;
	TArray<FString> LocalVariables;  // If inside a function
	
	// === COMPONENT STATE ===
	TArray<FString> Components;      // Component names in this BP
	FString SelectedComponent;       // If a component is selected
	
	// === SERIALIZATION ===
	FString ToJSON() const;
	TSharedPtr<FJsonObject> ToJsonObject() const;
	
	// === HELPERS ===
	bool HasSelection() const { return SelectedNodeCount > 0; }
	bool HasPinCursor() const { return bHasPinContext; }
	bool IsInFunction() const { return ActiveGraphType == TEXT("FunctionGraph"); }
	bool IsInEventGraph() const { return ActiveGraphType == TEXT("EventGraph"); }
	FString GetPrimarySelectedNodeType() const 
	{ 
		return SelectedNodeTypes.Num() > 0 ? SelectedNodeTypes[0] : TEXT(""); 
	}
};

/**
 * Level editor context - state of the level viewport and world
 */
struct RIFTBORNAI_API FLevelEditorContext
{
	// === LEVEL IDENTITY ===
	FString CurrentLevelPath;       // /Game/Maps/MainLevel
	FString CurrentLevelName;       // MainLevel
	bool bIsUntitledLevel = false;
	
	// === ACTOR SELECTION ===
	int32 SelectedActorCount = 0;
	TArray<FString> SelectedActorNames;
	TArray<FString> SelectedActorClasses;
	TArray<FString> SelectedActorLabels; // Display names
	FVector SelectionCentroid = FVector::ZeroVector;
	FBox SelectionBounds;
	
	// === VIEWPORT STATE ===
	FVector CameraLocation = FVector::ZeroVector;
	FRotator CameraRotation = FRotator::ZeroRotator;
	FString ActiveViewportType;     // "Perspective", "Top", "Front", "Side"
	float CameraSpeed = 1.0f;
	
	// === PLAY STATE ===
	bool bIsSimulating = false;     // Simulate mode
	bool bIsPIE = false;            // Play In Editor
	bool bIsPaused = false;         // PIE paused
	float PIEElapsedTime = 0.0f;    // Seconds since PIE started
	FString PIEWorldType;           // "Game", "PIE", "Editor"
	
	// === WORLD INFO ===
	FString GameModeName;           // Active game mode class
	int32 TotalActorCount = 0;      // Actors in level
	int32 VisibleActorCount = 0;    // Currently visible
	
	// === RECENT ACTIONS ===
	TArray<FString> RecentlyModifiedActors; // Last N actors touched
	
	// === SERIALIZATION ===
	FString ToJSON() const;
	TSharedPtr<FJsonObject> ToJsonObject() const;
	
	// === HELPERS ===
	bool HasSelection() const { return SelectedActorCount > 0; }
	bool IsPlaying() const { return bIsPIE || bIsSimulating; }
	FString GetPrimarySelectedClass() const 
	{ 
		return SelectedActorClasses.Num() > 0 ? SelectedActorClasses[0] : TEXT(""); 
	}
};

/**
 * Output log context - recent messages from UE and our systems
 */
struct RIFTBORNAI_API FOutputLogContext
{
	// === RIFTBORN LOGS ===
	TArray<FString> RecentRiftbornLogs;     // Our system logs
	TArray<FString> RecentToolExecutions;    // Recent tool calls
	
	// === ERRORS & WARNINGS ===
	TArray<FString> RecentErrors;           // Last N errors (any source)
	TArray<FString> RecentWarnings;         // Last N warnings
	TArray<FString> RecentCompileErrors;    // Blueprint/C++ compile errors
	
	// === COUNTS ===
	int32 TotalErrorCount = 0;
	int32 TotalWarningCount = 0;
	int32 SessionErrorCount = 0;    // Since this editor session started
	int32 SessionWarningCount = 0;
	
	// === PIE OUTPUT ===
	TArray<FString> RecentPIELogs;  // Logs from last PIE session
	bool bPIEHadErrors = false;
	
	// === SERIALIZATION ===
	FString ToJSON() const;
	TSharedPtr<FJsonObject> ToJsonObject() const;
	
	// === HELPERS ===
	bool HasErrors() const { return RecentErrors.Num() > 0 || RecentCompileErrors.Num() > 0; }
	FString GetMostRecentError() const 
	{ 
		if (RecentCompileErrors.Num() > 0) return RecentCompileErrors.Last();
		if (RecentErrors.Num() > 0) return RecentErrors.Last();
		return TEXT("");
	}
};

/**
 * Material editor context - state of the material being edited
 */
struct RIFTBORNAI_API FMaterialEditorContext
{
	// === IDENTITY ===
	FString MaterialPath;           // /Game/Materials/M_MyMaterial
	FString MaterialName;           // M_MyMaterial
	FString MaterialDomain;         // Surface, DeferredDecal, LightFunction, Volume, PostProcess, UI
	FString BlendMode;              // Opaque, Masked, Translucent, Additive, Modulate
	FString ShadingModel;           // DefaultLit, Unlit, Subsurface, ClearCoat, etc.
	
	// === COMPILE STATE ===
	bool bIsCompiled = false;
	bool bHasErrors = false;
	int32 ErrorCount = 0;
	TArray<FString> CompileErrors;
	
	// === NODE GRAPH ===
	int32 TotalNodeCount = 0;
	int32 TextureSampleCount = 0;
	int32 ParameterCount = 0;
	TArray<FString> TextureParameterNames;  // Named texture params
	TArray<FString> ScalarParameterNames;   // Named scalar params
	TArray<FString> VectorParameterNames;   // Named vector params
	
	// === SELECTION ===
	int32 SelectedNodeCount = 0;
	TArray<FString> SelectedNodeTypes;
	
	// === SERIALIZATION ===
	FString ToJSON() const;
	TSharedPtr<FJsonObject> ToJsonObject() const;
	
	// === HELPERS ===
	bool HasSelection() const { return SelectedNodeCount > 0; }
	bool IsTranslucent() const { return BlendMode == TEXT("Translucent") || BlendMode == TEXT("Additive"); }
};

/**
 * Niagara editor context - state of the particle system being edited
 */
struct RIFTBORNAI_API FNiagaraEditorContext
{
	// === IDENTITY ===
	FString SystemPath;             // /Game/FX/NS_Explosion
	FString SystemName;             // NS_Explosion
	
	// === EMITTERS ===
	int32 EmitterCount = 0;
	TArray<FString> EmitterNames;
	
	// === SYSTEM STATS ===
	int32 TotalModuleCount = 0;
	bool bUsesGPU = false;          // Any GPU emitters?
	bool bIsLooping = false;
	float SystemDuration = 0.0f;    // Duration in seconds (0 = infinite)
	
	// === COMPILE STATE ===
	bool bIsCompiled = false;
	bool bHasErrors = false;
	int32 ErrorCount = 0;
	TArray<FString> CompileErrors;
	
	// === SELECTION ===
	FString ActiveEmitterName;      // Currently focused emitter
	int32 SelectedModuleCount = 0;
	TArray<FString> SelectedModuleNames;
	
	// === SERIALIZATION ===
	FString ToJSON() const;
	TSharedPtr<FJsonObject> ToJsonObject() const;
	
	// === HELPERS ===
	bool HasSelection() const { return SelectedModuleCount > 0; }
};

/**
 * Unified Editor Context
 * 
 * Single source of truth for all copilot context.
 * Call GetContext() from:
 * - Chat panel (every message)
 * - Inline completion (every keystroke, cached)
 * - Plan generation (every proposal)
 * - Tool selection (brain context)
 */
struct RIFTBORNAI_API FEditorContext
{
	// === ACTIVE EDITOR ===
	EActiveEditorType ActiveEditorType = EActiveEditorType::None;
	FString ActiveAssetPath;        // Full path to active asset
	FString ActiveAssetName;        // Just the name
	FString ActiveAssetClass;       // Blueprint, Material, etc.
	
	// === SPECIFIC CONTEXTS ===
	// Only populated if relevant editor is active
	TOptional<FBlueprintGraphContext> BlueprintContext;
	TOptional<FLevelEditorContext> LevelContext;
	TOptional<FMaterialEditorContext> MaterialContext;
	TOptional<FNiagaraEditorContext> NiagaraContext;
	
	// === ALWAYS AVAILABLE ===
	FOutputLogContext OutputLogContext;
	
	// === METADATA ===
	FDateTime GatheredAt;
	float GatherDurationMs = 0.0f;  // How long it took to gather
	int32 ContextVersion = 1;       // Schema version for compatibility
	
	// === SERIALIZATION ===
	FString ToJSON() const;
	TSharedPtr<FJsonObject> ToJsonObject() const;
	
	// === COMPACT SERIALIZATION ===
	// Token-efficient version for prompt injection
	FString ToCompactJSON() const;
	
	// === HELPERS ===
	bool HasBlueprintContext() const { return BlueprintContext.IsSet(); }
	bool HasLevelContext() const { return LevelContext.IsSet(); }
	bool HasMaterialContext() const { return MaterialContext.IsSet(); }
	bool HasNiagaraContext() const { return NiagaraContext.IsSet(); }
	bool HasSelection() const 
	{ 
		return (BlueprintContext.IsSet() && BlueprintContext->HasSelection()) ||
		       (LevelContext.IsSet() && LevelContext->HasSelection()) ||
		       (MaterialContext.IsSet() && MaterialContext->HasSelection()) ||
		       (NiagaraContext.IsSet() && NiagaraContext->HasSelection());
	}
	bool HasErrors() const { return OutputLogContext.HasErrors(); }
	
	FString GetActiveEditorTypeName() const;
};

/**
 * Editor Context Service
 * 
 * Central aggregator for all editor state. Thread-safe, cached.
 * 
 * Usage:
 *   FEditorContext Context = FEditorContextService::Get().GetContext();
 *   FString ContextJSON = Context.ToJSON();  // For prompt injection
 */
class RIFTBORNAI_API FEditorContextService
{
public:
	static FEditorContextService& Get();
	
	// =========================================================================
	// MAIN API
	// =========================================================================
	
	/**
	 * Get current editor context
	 * 
	 * @param bForceRefresh - If true, bypass cache and re-gather all context
	 * @return Complete editor context snapshot
	 * 
	 * NOTE: This is the primary entry point. Uses caching by default.
	 * Force refresh only when you know state has changed significantly.
	 */
	FEditorContext GetContext(bool bForceRefresh = false);
	
	/**
	 * Get context as JSON string for prompt injection
	 * Convenience wrapper around GetContext().ToJSON()
	 */
	FString GetContextJSON(bool bForceRefresh = false);
	
	/**
	 * Get compact context JSON (token-efficient)
	 * Use this for inline completion where tokens matter
	 */
	FString GetCompactContextJSON(bool bForceRefresh = false);
	
	// =========================================================================
	// SPECIFIC CONTEXT GETTERS
	// =========================================================================
	
	/**
	 * Get Blueprint context if BP editor is active
	 * @return Blueprint context or empty optional if not in BP editor
	 */
	TOptional<FBlueprintGraphContext> GetBlueprintContext();
	
	/**
	 * Get Level context if level editor is active
	 * @return Level context or empty optional if not in level editor
	 */
	TOptional<FLevelEditorContext> GetLevelContext();
	
	/**
	 * Get currently active editor type
	 */
	EActiveEditorType GetActiveEditorType();
	
	/**
	 * Get the active Blueprint asset (if in BP editor)
	 * @return Blueprint pointer or nullptr
	 */
	class UBlueprint* GetActiveBlueprintAsset();
	
	/**
	 * Get selected nodes in the active graph
	 */
	TArray<class UEdGraphNode*> GetSelectedNodes();
	
	/**
	 * Get recent output log entries
	 * @param MaxErrors - Maximum error lines to return
	 * @param MaxWarnings - Maximum warning lines to return
	 */
	FOutputLogContext GetOutputLogContext(int32 MaxErrors = 10, int32 MaxWarnings = 5);
	
	// =========================================================================
	// CACHE CONTROL
	// =========================================================================
	
	/**
	 * Invalidate cache (call when major editor state changes)
	 * Examples: asset saved, PIE started/stopped, BP compiled
	 */
	void InvalidateCache();
	
	/**
	 * Set cache duration in seconds
	 * Lower = more responsive but more CPU
	 * Higher = less CPU but potentially stale
	 * @param Seconds - Cache duration (default 0.5s)
	 */
	void SetCacheDuration(float Seconds);
	
	/**
	 * Get cache statistics for debugging
	 */
	struct FCacheStats
	{
		int32 CacheHits = 0;
		int32 CacheMisses = 0;
		int32 Invalidations = 0;
		float AverageGatherTimeMs = 0.0f;
		FDateTime LastGatherTime;
	};
	FCacheStats GetCacheStats() const { return CacheStats; }
	
	// =========================================================================
	// EVENTS
	// =========================================================================
	
	/**
	 * Subscribe to context changes (for reactive UIs)
	 * Fires when context changes significantly (not on every cache miss)
	 */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnContextChanged, const FEditorContext&);
	FOnContextChanged OnContextChanged;
	
	// =========================================================================
	// DEBUG
	// =========================================================================
	
	/**
	 * Dump current context to log
	 */
	void DebugDumpContext();
	
	/**
	 * Register event handlers for editor changes
	 * Called once at module startup
	 */
	void RegisterEditorEventHandlers();
	
	/**
	 * Unregister event handlers
	 * Called at module shutdown
	 */
	void UnregisterEditorEventHandlers();

private:
	FEditorContextService();
	~FEditorContextService();
	
	// Context gathering (called on cache miss)
	FEditorContext GatherContext();
	FBlueprintGraphContext GatherBlueprintContext();
	FLevelEditorContext GatherLevelContext();
	FOutputLogContext GatherOutputLogContext();
	FMaterialEditorContext GatherMaterialContext();
	FNiagaraEditorContext GatherNiagaraContext();
	
	// Editor type detection
	EActiveEditorType DetermineActiveEditorType();
	FString GetActiveAssetPath();
	
	// Blueprint helpers - now public, declared above
	class UEdGraph* GetActiveGraph();
	
	// Level helpers
	class UWorld* GetEditorWorld();
	TArray<class AActor*> GetSelectedActors();
	
	// Cache
	FEditorContext CachedContext;
	FDateTime CacheTime;
	float CacheDurationSeconds = 0.5f;
	bool bCacheValid = false;
	FCriticalSection CacheLock;
	FCacheStats CacheStats;
	
	// Significant change detection
	uint32 LastContextHash = 0;
	uint32 ComputeContextHash(const FEditorContext& Context);
	
	// Event delegate handles
	FDelegateHandle OnBlueprintCompiledHandle;
	FDelegateHandle OnAssetEditorOpenedHandle;
	FDelegateHandle OnAssetEditorClosedHandle;
	FDelegateHandle OnBlueprintFocusChangedHandle;
	FDelegateHandle OnBlueprintSelectionChangedHandle;
	FDelegateHandle OnBlueprintCursorChangedHandle;
	TMap<UBlueprint*, FDelegateHandle> BlueprintSelectionHandles;
	bool bEventHandlersRegistered = false;
	
	// Track last active Blueprint (for multi-editor focus detection)
	TWeakObjectPtr<UBlueprint> LastActiveBlueprintWeak;
	void SetLastActiveBlueprint(UBlueprint* BP);
};
