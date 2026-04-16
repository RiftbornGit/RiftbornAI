// ClaudeToolUse_Builtins.h
// Extracted from ClaudeToolUse.h: FRiftbornBuiltinTools class

#pragma once

#include "CoreMinimal.h"

// Forward declarations — full types in ClaudeToolUse.h
struct FClaudeToolCall;
struct FClaudeToolResult;

/**
 * Built-in Unreal Engine tools for Claude
 */
class RIFTBORNAI_API FRiftbornBuiltinTools
{
public:
	// Register all built-in tools
	static void RegisterAllTools();
	
	// Reset the registration flag (call before RegisterAllTools to force re-registration)
	static void ResetRegistrationFlag();

	// ========================
	// TOOL HANDLERS - Public so modules can register them
	// ========================
	
	// Blueprint operations

	// System
	static FClaudeToolResult Tool_ReloadToolRegistry(const FClaudeToolCall& Call);

	// Level operations
	
	// Transform operations
	
	// Component operations
	
	// Editor operations

	// Asset operations
	static FClaudeToolResult Tool_ListAssets(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_LoadAsset(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateAsset(const FClaudeToolCall& Call);
	
	// Semantic Asset Resolution (Natural Language)
	static FClaudeToolResult Tool_ResolveAsset(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetAssetCategories(const FClaudeToolCall& Call);
	
	// Material operations
	
	// Mesh operations
	static FClaudeToolResult Tool_ImportAsset(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ImportAssetFromUrl(const FClaudeToolCall& Call);

	// Code generation
	static FClaudeToolResult Tool_ExecutePython(const FClaudeToolCall& Call);
	
	// File operations
	static FClaudeToolResult Tool_ListDirectory(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SearchFiles(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GrepContent(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetProjectInfo(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetToolStats(const FClaudeToolCall& Call);  // Tool introspection
	static FClaudeToolResult Tool_GetClassHierarchy(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetSourceFile(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateSourceFile(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_EditSourceFile(const FClaudeToolCall& Call);

	// Project analysis
	static FClaudeToolResult Tool_GetProjectStructure(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SearchCode(const FClaudeToolCall& Call);
	
	// Play control
	static FClaudeToolResult Tool_PlayInEditor(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_StopPlay(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetGameModeOverride(const FClaudeToolCall& Call);
	
	// ========================
	// PROMPT PIPELINE TOOLS
	// ========================
	static FClaudeToolResult Tool_ExecutePromptPipeline(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_PlanPromptPipeline(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetPipelineStatus(const FClaudeToolCall& Call);
	
	// Enhancement 3: Multi-file refactoring
	static FClaudeToolResult Tool_RenameClass(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_BatchAddComponent(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_FindReplaceInProject(const FClaudeToolCall& Call);
	
	// Enhancement 5: Blueprint scripting
	static FClaudeToolResult Tool_AddBlueprintEvent(const FClaudeToolCall& Call);
	
	// Enhancement 6: Asset import
	
	// Selection/Context
	static FClaudeToolResult Tool_SelectActorsByClass(const FClaudeToolCall& Call);
	
	// ========================
	// NEW COMPREHENSIVE TOOLS
	// ========================
	
	// Level Management
	static FClaudeToolResult Tool_CreateLevel(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SaveLevelAs(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetOpenLevels(const FClaudeToolCall& Call);
	
	// Asset Management
	
	// Blueprint Configuration
	static FClaudeToolResult Tool_SetDefaultPawnClass(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_MakeCharacterPlayable(const FClaudeToolCall& Call);  // HIGH-LEVEL COMPOSITE TOOL
	static FClaudeToolResult Tool_SetPlayerControllerClass(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetBlueprintVariables(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetBlueprintVariableDefault(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetBlueprintFunctions(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RemoveBlueprintNode(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ReplaceBlueprintNode(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetBlueprintPinDefault(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RenameBlueprintVariable(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ReparentBlueprint(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_BatchCompileBlueprints(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RepairBlueprintCompileErrors(const FClaudeToolCall& Call);
	
	// Component Management
	
	// Input System
	static FClaudeToolResult Tool_CreateInputAction(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateInputMappingContext(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AddInputMapping(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AddInputTrigger(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AddInputModifier(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetInputActionPlayerMappable(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ListInputMappings(const FClaudeToolCall& Call);
	
	// Animation
	static FClaudeToolResult Tool_SetSkeletalMesh(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetAnimationBlueprint(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetAnimationAssets(const FClaudeToolCall& Call);
	
	// Physics & Collision
	static FClaudeToolResult Tool_SetCollision(const FClaudeToolCall& Call);
	
	// Widget/UI
	static FClaudeToolResult Tool_GetWidgetBlueprints(const FClaudeToolCall& Call);
	
	// Camera & Viewport
	static FClaudeToolResult Tool_SetViewportLocation(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetViewportInfo(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetViewportExposureState(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetViewportExposureState(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetEditorGuiState(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ListEditorWindows(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_FocusEditorWindow(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_MinimizeEditorWindow(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RestoreEditorWindow(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ResizeEditorWindow(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CloseEditorWindow(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SaveViewportBookmark(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_LoadViewportBookmark(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ListViewportBookmarks(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SaveSceneCheckpoint(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RestoreSceneCheckpoint(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_TakeScreenshot(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AnalyzeSceneScreenshot(const FClaudeToolCall& Call);
	/** Async sibling — same observable behaviour, but does NOT block the
	 *  game thread during screenshot readback or vision-LLM round-trip.
	 *  Registered via FClaudeToolRegistry::RegisterToolAsync. */
	static void Tool_AnalyzeSceneScreenshotAsync(const FClaudeToolCall& Call, FOnAsyncToolComplete OnComplete);
	static FClaudeToolResult Tool_ObserveUEProject(const FClaudeToolCall& Call);
	/** Async sibling — runs the GT-bound observation work inline (it's
	 *  already fast), defers the vision LLM call to a worker thread. Same
	 *  observable JSON, just doesn't freeze the editor on the vision step. */
	static void Tool_ObserveUEProjectAsync(const FClaudeToolCall& Call, FOnAsyncToolComplete OnComplete);
	static FClaudeToolResult Tool_LookAtAndCapture(const FClaudeToolCall& Call);
	/** Async sibling — same observable behaviour, but does NOT block the
	 *  game thread during screenshot readback or vision-LLM round-trip.
	 *  Registered via FClaudeToolRegistry::RegisterToolAsync. */
	static void Tool_LookAtAndCaptureAsync(const FClaudeToolCall& Call, FOnAsyncToolComplete OnComplete);
	static FClaudeToolResult Tool_CaptureViewportSync(const FClaudeToolCall& Call);
	/** Async sibling — vision LLM call moves to a worker thread. */
	static void Tool_CaptureViewportSyncAsync(const FClaudeToolCall& Call, FOnAsyncToolComplete OnComplete);
	static FClaudeToolResult Tool_CaptureViewportSafe(const FClaudeToolCall& Call);
	/** Async sibling — delegates to Tool_CaptureViewportSyncAsync. */
	static void Tool_CaptureViewportSafeAsync(const FClaudeToolCall& Call, FOnAsyncToolComplete OnComplete);
	static FClaudeToolResult Tool_GetWorldStateDigest(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_DiffWorldStateDigest(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetSceneGraph(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetSystemicWorldState(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ExplainSystemicWorldState(const FClaudeToolCall& Call);
	
	// Project Configuration
	static FClaudeToolResult Tool_GetProjectSettings(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetProjectSetting(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ListSettingsProfiles(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetSettingsProfile(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetSettingsProfileValue(const FClaudeToolCall& Call);
	
	// Console & Debug
	static FClaudeToolResult Tool_ExecuteConsoleCommand(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetOutputLog(const FClaudeToolCall& Call);
	
	// DataTable & Gameplay
	
	// World Settings
	static FClaudeToolResult Tool_GetWorldSettings(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetWorldSetting(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetKillZ(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetWorldGravity(const FClaudeToolCall& Call);
	
	// Blueprint Defaults (CDO)
	static FClaudeToolResult Tool_GetBlueprintDefaults(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetBlueprintDefault(const FClaudeToolCall& Call);
	
	// Project Maps & Modes
	static FClaudeToolResult Tool_SetGameInstanceClass(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetAllGameModes(const FClaudeToolCall& Call);
	
	// Editor Preferences
	static FClaudeToolResult Tool_GetEditorPreference(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetEditorPreference(const FClaudeToolCall& Call);
	
	// Lighting
	static FClaudeToolResult Tool_CreateLight(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_BuildLighting(const FClaudeToolCall& Call);
	
	// Audio
	static FClaudeToolResult Tool_GetAudioAssets(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetAudioComponent(const FClaudeToolCall& Call);
	
	// AI & Navigation
	static FClaudeToolResult Tool_CreateNavMeshBounds(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetNavMeshInfo(const FClaudeToolCall& Call);
	
	// Sequencer
	static FClaudeToolResult Tool_GetSequences(const FClaudeToolCall& Call);
	
	// Niagara/Particles
	static FClaudeToolResult Tool_GetNiagaraAssets(const FClaudeToolCall& Call);
	
	// C++ Code Generation Tools
	static FClaudeToolResult Tool_GenerateActorClassFromDescription(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GenerateCharacterClassFromDescription(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GenerateComponentClassFromDescription(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GenerateSubsystemClassFromDescription(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GenerateGameModeClassFromDescription(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GenerateControllerClassFromDescription(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ValidateCppClassSpec(const FClaudeToolCall& Call);
	
	// Quality of Life Tools
	static FClaudeToolResult Tool_GenerateInlineSuggestions(const FClaudeToolCall& Call);
	
	// =========================================================================
	// PROACTIVE VERIFICATION & RECOVERY (Tier 3)
	// Expose playtest, visual verify, error recovery, and analysis to the LLM
	// =========================================================================
	static FClaudeToolResult Tool_RunQuickPlaytest(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_VisualVerifyScene(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetErrorRecoverySuggestions(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RunFullProjectAnalysis(const FClaudeToolCall& Call);
	
	// =========================================================================
	// Tier 4: Cross-session intelligence — memory management, tool reliability,
	// session browsing, error recall, and learned recipe search
	// =========================================================================
	static FClaudeToolResult Tool_PinProjectContext(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_UnpinProjectContext(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SearchPastSessions(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetToolReliability(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RecallErrorSolutions(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetLearnedRecipes(const FClaudeToolCall& Call);
	
	// Implementation Roadmap
	static FClaudeToolResult Tool_CreateRoadmap(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetRoadmapTemplate(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SuggestNextSteps(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetToolsForFeature(const FClaudeToolCall& Call);
	
	// Full Game Creation
	static FClaudeToolResult Tool_CreatePlayableGame(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AnalyzeGameDescription(const FClaudeToolCall& Call);
	
	// Tool Help System
	static FClaudeToolResult Tool_DescribeTool(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ListAllTools(const FClaudeToolCall& Call);
	
	// P2: Meta-tool dispatch system
	static FClaudeToolResult Tool_UseSpecializedTool(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ListToolsInCategory(const FClaudeToolCall& Call);
	
	// Landscape
	static FClaudeToolResult Tool_CreateLandscape(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetLandscapeInfo(const FClaudeToolCall& Call);
	
	// Foliage
	static FClaudeToolResult Tool_GetFoliageTypes(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AddFoliageInstance(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RemoveFoliageInstance(const FClaudeToolCall& Call);
	
	// Landscape Materials
	static FClaudeToolResult Tool_CreateLandscapeMaterial(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ApplyLandscapeMaterial(const FClaudeToolCall& Call);
	
	// Packaging & Build
	static FClaudeToolResult Tool_CookProject(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_PackageProject(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetBuildStatus(const FClaudeToolCall& Call);
	
	// Source Control
	static FClaudeToolResult Tool_CheckoutFile(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SubmitChanges(const FClaudeToolCall& Call);
	
	// Plugins
	static FClaudeToolResult Tool_DisablePlugin(const FClaudeToolCall& Call);
	
	// ============================================================================
	// BATCH 1: Gameplay Tags, GAS, Animation
	// ============================================================================
	
	// Gameplay Tags
	static FClaudeToolResult Tool_AddGameplayTag(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetGameplayTags(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetActorGameplayTags(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateGameplayTagTable(const FClaudeToolCall& Call);
	
	// Gameplay Ability System
	static FClaudeToolResult Tool_CreateGameplayAbility(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateGameplayEffect(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateAttributeSet(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetGASAssets(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AddAbilityToActor(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ConfigureGameplayEffect(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ConfigureGEStacking(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetAbilityCooldown(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetAbilityCost(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetAbilityPolicies(const FClaudeToolCall& Call);
	
	// Animation Expansion
	static FClaudeToolResult Tool_CreateAnimMontage(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateBlendSpace1D(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateAnimComposite(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetSkeletons(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RetargetAnimation(const FClaudeToolCall& Call);
	
	// ============================================================================
	// BATCH 2: Collision & Physics
	// ============================================================================
	
	// Collision
	static FClaudeToolResult Tool_SetCollisionResponse(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetCollisionChannels(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AddCollisionShape(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateCollisionPreset(const FClaudeToolCall& Call);
	
	// Physics Assets
	static FClaudeToolResult Tool_CreatePhysicsAsset(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetPhysicsAssets(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetPhysicsConstraint(const FClaudeToolCall& Call);
	
	// ============================================================================
	// BATCH 3: Level Streaming & World Partition
	// ============================================================================
	
	// Level Streaming
	
	// World Partition
	static FClaudeToolResult Tool_CreateDataLayer(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetActorDataLayer(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetWorldPartitionInfo(const FClaudeToolCall& Call);
	
	// ============================================================================
	// BATCH 4: AI Perception & EQS
	// ============================================================================
	
	// AI Perception
	static FClaudeToolResult Tool_AddAISense(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ConfigureAIPerception(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetAISenseConfig(const FClaudeToolCall& Call);
	
	// Environment Query System
	static FClaudeToolResult Tool_CreateEnvQuery(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetEnvQueries(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AddEQSTest(const FClaudeToolCall& Call);
	
	// ============================================================================
	// BATCH 5: Rendering & Post Process
	// ============================================================================
	
	// Post Process
	static FClaudeToolResult Tool_CreatePostProcessVolume(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetPostProcessSettings(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetPostProcessVolumes(const FClaudeToolCall& Call);
	
	// Render Targets
	static FClaudeToolResult Tool_CreateRenderTarget(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetRenderTargets(const FClaudeToolCall& Call);
	
	// Lighting/Rendering Settings
	static FClaudeToolResult Tool_SetLumenSettings(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetExposureSettings(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetSkyAtmosphere(const FClaudeToolCall& Call);
	
	// ============================================================================
	// BATCH 6: Data Assets & Replication
	// ============================================================================
	
	// Data Assets
	static FClaudeToolResult Tool_CreateDataAsset(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreatePrimaryDataAsset(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetDataAssets(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetDataAssetProperty(const FClaudeToolCall& Call);
	
	// Replication
	
	// ============================================================================
	// BATCH 7: Editor Utilities & Testing
	// ============================================================================
	
	// Editor Utilities
	static FClaudeToolResult Tool_CreateEditorUtilityWidget(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RunEditorUtility(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateBlutility(const FClaudeToolCall& Call);
	
	// Testing
	
	// Subsystems
	static FClaudeToolResult Tool_GetSubsystems(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CallSubsystemFunction(const FClaudeToolCall& Call);
	
	// ============================================================================
	// BATCH 8: Console Variables, Validation, Profiling, Packaging
	// ============================================================================
	
	// Console Variables (ExecuteConsoleCommand exists above)
	static FClaudeToolResult Tool_GetConsoleVariables(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetConsoleVariable(const FClaudeToolCall& Call);
	
	// Asset Validation
	static FClaudeToolResult Tool_FixRedirectors(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_FindBrokenReferences(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetAssetAudit(const FClaudeToolCall& Call);

	// Validation Framework Tools
	static FClaudeToolResult Tool_ValidateLevelPrerequisites(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ValidateBlueprintHealth(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ValidateMaterialReferences(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ValidateDependencyChain(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ValidatePathWritable(const FClaudeToolCall& Call);

	// Memory & Profiling
	static FClaudeToolResult Tool_GetMemoryStats(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_StartProfiling(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_StopProfiling(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetPerformanceMetrics(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_TakeMemorySnapshot(const FClaudeToolCall& Call);
	
	// Brain Learning Metrics
	static FClaudeToolResult Tool_GetBrainMetrics(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetBrainToolStats(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetBrainReport(const FClaudeToolCall& Call);
	
	// Advanced Packaging
	static FClaudeToolResult Tool_GetPackagingSettings(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetPackagingSetting(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ValidateForPackaging(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetCookStats(const FClaudeToolCall& Call);
	
	// ============================================================================
	// BATCH 9: AI Agent, Hot Reload, Remote Debug, Cooking, Localization, Online
	// ============================================================================
	
	// Hot Reload
	static FClaudeToolResult Tool_HotReloadCpp(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetCompilationStatus(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RecompileBlueprints(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RefreshAllNodes(const FClaudeToolCall& Call);

	// Build Monitoring (ToolImpl_BuildMonitor.cpp)
	static FClaudeToolResult Tool_DiagnoseCrash(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetBuildEvents(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetBuildErrors(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetErrorSummary(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetErrorsSince(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_WaitForBuild(const FClaudeToolCall& Call);

	// Crash Pattern Database (CrashPatternDatabaseTools.cpp)
	static FClaudeToolResult Tool_RecordCrashResolution(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetKnownCrashPatterns(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_LookupCrashPattern(const FClaudeToolCall& Call);

	// Modal Dismissal (ToolImpl_BuildMonitor.cpp — Phase 2)
	static FClaudeToolResult Tool_DismissModalDialog(const FClaudeToolCall& Call);

	// Editor Control (ToolImpl_EditorControl.cpp — Phase 3)
	static FClaudeToolResult Tool_SetViewportMode(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_NavigateContentBrowser(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetEditorPref(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ClickEditorButton(const FClaudeToolCall& Call);
	
	// Remote / PIE Debugging
	static FClaudeToolResult Tool_StartPIE(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_StopPIE(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetPIEState(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_VerifyGameplay(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetBreakpoint(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetCallStack(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_InspectObject(const FClaudeToolCall& Call);
	
	// Latent Async Verification Tools (Priority 8D)
	static FClaudeToolResult Tool_WaitForPredicate(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RunPieSession(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetLatentJobStatus(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CancelLatentJob(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_IsPieRunning(const FClaudeToolCall& Call);
	
	// Asset Cooking (Per-Platform)
	static FClaudeToolResult Tool_CookAsset(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetCookProgress(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CancelCook(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetCookedAssets(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_ClearCookedData(const FClaudeToolCall& Call);
	
	// Localization
	static FClaudeToolResult Tool_CreateStringTable(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AddStringTableEntry(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetStringTableEntries(const FClaudeToolCall& Call);
	// import_localization / export_localization REMOVED — theatre tools (returned instructions, not actions)
	static FClaudeToolResult Tool_GetSupportedCultures(const FClaudeToolCall& Call);
	
	// Online Subsystem
	static FClaudeToolResult Tool_GetOnlineSubsystemInfo(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateSession(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_FindSessions(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_JoinSession(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_DestroySession(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetSessionState(const FClaudeToolCall& Call);
	
	// ============================================================================
	// OS-LEVEL TOOLS - System monitoring and file indexing for AI OS capabilities
	// ============================================================================
	
	// System Information
	static FClaudeToolResult Tool_GetSystemInfo(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetCPUUsage(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetMemoryUsage(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetDiskUsage(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetGPUInfo(const FClaudeToolCall& Call);
	
	// Process Management
	static FClaudeToolResult Tool_ListProcesses(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetProcessInfo(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_KillProcess(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_StartProcess(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetRunningEditors(const FClaudeToolCall& Call);
	
	// File Indexing & Semantic Search
	static FClaudeToolResult Tool_IndexProjectFiles(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SemanticSearchFiles(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetFileMetadata(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetRecentFiles(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetFileHistory(const FClaudeToolCall& Call);
	
	// Environment & Configuration
	static FClaudeToolResult Tool_GetAuthDiagnostics(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetEnvironmentVariable(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_SetEnvironmentVariable(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetSystemPath(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetInstalledPrograms(const FClaudeToolCall& Call);
	
	// Terminal & Shell
	static FClaudeToolResult Tool_ExecuteShellCommand(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetTerminalOutput(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetCommandHistory(const FClaudeToolCall& Call);
	
	// Network & Connectivity
	static FClaudeToolResult Tool_GetNetworkInterfaces(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CheckHostReachable(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetActiveConnections(const FClaudeToolCall& Call);
	
	// Build System Integration (NOTE: Tool_GetBuildStatus already declared in Packaging & Build section)
	static FClaudeToolResult Tool_TriggerBuild(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetCompilerOutput(const FClaudeToolCall& Call);
	
	// Git Integration
	
	// ============================================================================
	// BLUEPRINT ERROR DETECTION & AUTO-FIX
	// ============================================================================
	
	// Error Detection
	static FClaudeToolResult Tool_GetBlueprintErrors(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetAllCompilationErrors(const FClaudeToolCall& Call);
	
	// Error Analysis & Planning
	static FClaudeToolResult Tool_AnalyzeBlueprintError(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_PlanBlueprintFix(const FClaudeToolCall& Call);
	
	// Auto-Fix Execution  
	static FClaudeToolResult Tool_FixBlueprintError(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RecompileBlueprint(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_DeleteBrokenBlueprint(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AutoFixAllBlueprints(const FClaudeToolCall& Call);
	
	// =========================================================================
	// Test Fixture Tools (RIFTBORN_TEST_MODE=1 required)
	// These tools create deterministic failure scenarios for repair testing
	// =========================================================================
	static FClaudeToolResult Tool_CreateBrokenTestBpMissingVariable(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RepairMissingVariable(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateBrokenTestBpMissingParent(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RepairMissingParent(const FClaudeToolCall& Call);
};
