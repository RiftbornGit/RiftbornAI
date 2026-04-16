// Copyright RiftbornAI. All Rights Reserved.
// Project Tools Module - ENH-008: Multi-Project Support

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Project Tools Module
 * Provides tools for multi-project management and project context.
 * 
 * ENH-008: Multi-Project Support
 * 
 * Tools:
 * - register_project: Register an external Unreal project
 * - list_projects: List all registered projects
 * - switch_project: Switch active project context
 * - get_project_info: Get detailed info about a project
 * - get_project_context: Get current project context
 * - get_open_asset_editors: Get currently open asset editor tabs
 * - get_content_browser_state: Get current Content Browser selection state
 * - get_editor_workspace_state: Get grounded editor workspace state
 * - get_editor_control_state: Get canonical editor control state for copilot/MCP planning
 * - get_editor_focus_state: Get the most recently active asset editor and all open editor tabs
 * - get_output_log_context: Get recent output log lines and error/warning counts from the live editor log watcher
 * - get_message_log_context: Get a named UE Message Log listing with counts and recent messages
 * - drain_log_alerts: Poll the live warning/error alert feed since a cursor across output log and watched Message Logs
 * - get_notification_center_state: Get active editor notification-window state from Slate
 * - get_modal_blockers: Get currently active editor modal blockers and menu-stack blockers
 * - assert_no_modal_blockers: Fail closed if the editor currently has a blocking modal dialog open
 * - assert_output_log_clean: Fail if recent output log errors/warnings exceed the requested thresholds
 * - assert_asset_dirty_state: Assert whether a specific asset package is dirty or clean
 * - assert_actor_selection: Assert editor actor selection counts and optional required labels
 * - assert_editor_focus: Assert the currently focused asset editor type and/or asset path
 * - get_world_outliner_context: Inspect the live editor-world actor list, folders, and selection state
 * - assert_object_property_equals: Assert a reflected UObject property equals the expected exported value
 * - get_compile_diagnostics: Aggregate compile/build diagnostics from output log, Message Log, and active editor context
 * - get_blueprint_editor_context: Get live focused Blueprint editor graph context
 * - get_material_editor_context: Get live Material editor context with node selection
 * - recompile_material_asset: Recompile the active or specified material asset
 * - layout_material_asset_expressions: Auto-layout the active or specified material graph
 * - get_sequencer_editor_context: Get live Sequencer selection/context for the active Level Sequence editor
 * - assert_sequencer_selection: Assert current Sequencer selection counts
 * - add_selected_actors_to_active_sequence: Bind currently selected level actors into the active Level Sequence
 * - get_control_rig_editor_context: Get live Control Rig hierarchy selection context
 * - get_niagara_editor_context: Get live Niagara editor context using the active system view model when available
 * - open_asset_editor: Open a generic asset editor by asset path
 * - close_asset_editor: Close all editors for a specific asset
 * - focus_content_browser: Bring the primary Content Browser to front
 * - sync_content_browser_to_asset: Select an asset in the Content Browser
 * - sync_content_browser_to_folder: Navigate/select a folder in the Content Browser
 * - list_object_properties: Enumerate reflected properties on any resolved UObject
 * - get_object_property_typed: Read a reflected UObject property with type metadata
 * - set_object_property_typed: Mutate a reflected UObject property with type-aware parsing and editor notifications
 * - call_reflected_function: Invoke a reflected UObject function with typed parameter import/export
 * - save_asset: Save a specific loaded asset package through UEditorAssetSubsystem
 * - save_dirty_assets: Save dirty map/content packages through FEditorFileUtils
 * - checkout_asset: Check out a specific asset from source control through UEditorAssetSubsystem
 * - revert_asset: Revert a checked-out asset file through ISourceControlProvider
 * - get_editor_mode_state: Get available and active editor modes
 * - activate_editor_mode: Activate a level editor mode
 * - deactivate_editor_mode: Deactivate a level editor mode or return to default mode
 * - set_project_context_setting: Set a registered project setting
 * - get_project_context_setting: Get a registered project setting
 */
class RIFTBORNAI_API FProjectToolsModule : public TToolModuleBase<FProjectToolsModule>
{
public:
	/** Module name for registration */
	static FString StaticModuleName() { return TEXT("ProjectTools"); }
	
	/** Register all Project tools with the registry */
	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
	
	// =========================================================================
	// Tool Implementations
	// =========================================================================
	
	/** Register an external Unreal project */
	static FClaudeToolResult Tool_RegisterProject(const FClaudeToolCall& Call);
	
	/** List all registered projects */
	static FClaudeToolResult Tool_ListProjects(const FClaudeToolCall& Call);
	
	/** Switch active project context */
	static FClaudeToolResult Tool_SwitchProject(const FClaudeToolCall& Call);
	
	/** Get detailed info about a project */
	static FClaudeToolResult Tool_GetProjectInfo(const FClaudeToolCall& Call);
	
	/** Get current project context */
	static FClaudeToolResult Tool_GetProjectContext(const FClaudeToolCall& Call);

	/** Get currently open asset editors */
	static FClaudeToolResult Tool_GetOpenAssetEditors(const FClaudeToolCall& Call);

	/** Get current Content Browser state */
	static FClaudeToolResult Tool_GetContentBrowserState(const FClaudeToolCall& Call);

	/** Get grounded editor workspace state */
	static FClaudeToolResult Tool_GetEditorWorkspaceState(const FClaudeToolCall& Call);

	/** Get canonical editor control state */
	static FClaudeToolResult Tool_GetEditorControlState(const FClaudeToolCall& Call);

	/** Get the most recently active editor/asset focus state */
	static FClaudeToolResult Tool_GetEditorFocusState(const FClaudeToolCall& Call);

	/** Focus or open a specific asset editor */
	static FClaudeToolResult Tool_FocusAssetEditor(const FClaudeToolCall& Call);

	/** Focus a specific editor tab by tab id */
	static FClaudeToolResult Tool_FocusEditorTab(const FClaudeToolCall& Call);

	/** Get recent output log lines and counts from the log watcher */
	static FClaudeToolResult Tool_GetOutputLogContext(const FClaudeToolCall& Call);

	/** Get a named UE Message Log listing */
	static FClaudeToolResult Tool_GetMessageLogContext(const FClaudeToolCall& Call);

	/** Get warning/error alert deltas since a cursor across output log and watched Message Logs */
	static FClaudeToolResult Tool_DrainLogAlerts(const FClaudeToolCall& Call);

	/** Get editor notification-center state */
	static FClaudeToolResult Tool_GetNotificationCenterState(const FClaudeToolCall& Call);

	/** Get active modal/menu blockers from Slate/editor GUI state */
	static FClaudeToolResult Tool_GetModalBlockers(const FClaudeToolCall& Call);

	/** Assert the editor is not blocked by a modal dialog */
	static FClaudeToolResult Tool_AssertNoModalBlockers(const FClaudeToolCall& Call);

	/** Assert the output log stays within requested error/warning thresholds */
	static FClaudeToolResult Tool_AssertOutputLogClean(const FClaudeToolCall& Call);

	/** Summarize current session verification state */
	static FClaudeToolResult Tool_GetVerificationStatus(const FClaudeToolCall& Call);

	/** Assert whether an asset package is dirty or clean */
	static FClaudeToolResult Tool_AssertAssetDirtyState(const FClaudeToolCall& Call);

	/** Assert actor selection counts and optional labels */
	static FClaudeToolResult Tool_AssertActorSelection(const FClaudeToolCall& Call);

	/** Assert active editor focus type and/or asset */
	static FClaudeToolResult Tool_AssertEditorFocus(const FClaudeToolCall& Call);

	/** Inspect the current editor-world outliner context */
	static FClaudeToolResult Tool_GetWorldOutlinerContext(const FClaudeToolCall& Call);

	/** Assert a reflected UObject property equals the expected exported value */
	static FClaudeToolResult Tool_AssertObjectPropertyEquals(const FClaudeToolCall& Call);

	/** Aggregate compile/build diagnostics */
	static FClaudeToolResult Tool_GetCompileDiagnostics(const FClaudeToolCall& Call);

	/** Get live focused Blueprint editor graph context */
	static FClaudeToolResult Tool_GetBlueprintEditorContext(const FClaudeToolCall& Call);

	/** List graphs in the active or specified Blueprint */
	static FClaudeToolResult Tool_ListBlueprintGraphs(const FClaudeToolCall& Call);

	/** List nodes in a Blueprint graph */
	static FClaudeToolResult Tool_ListBlueprintNodes(const FClaudeToolCall& Call);

	/** Find Blueprint nodes by title/class/name substring */
	static FClaudeToolResult Tool_FindBlueprintNodes(const FClaudeToolCall& Call);

	/** Get compile diagnostics for the active or specified Blueprint */
	static FClaudeToolResult Tool_GetBlueprintCompileDiagnostics(const FClaudeToolCall& Call);

	/** Assert that a Blueprint compiles without errors */
	static FClaudeToolResult Tool_AssertBlueprintCompiles(const FClaudeToolCall& Call);

	/** Focus a Blueprint graph node in the active or specified Blueprint */
	static FClaudeToolResult Tool_FocusBlueprintNode(const FClaudeToolCall& Call);

	/** Get live Material editor context */
	static FClaudeToolResult Tool_GetMaterialEditorContext(const FClaudeToolCall& Call);

	/** List expressions in the active or specified material */
	static FClaudeToolResult Tool_ListMaterialExpressions(const FClaudeToolCall& Call);

	/** Inspect a single material expression */
	static FClaudeToolResult Tool_InspectMaterialExpression(const FClaudeToolCall& Call);

	/** Assert that a material compiles without shader errors */
	static FClaudeToolResult Tool_AssertMaterialCompiles(const FClaudeToolCall& Call);

	/** Recompile a material asset */
	static FClaudeToolResult Tool_RecompileMaterialAsset(const FClaudeToolCall& Call);

	/** Auto-layout a material asset's expressions */
	static FClaudeToolResult Tool_LayoutMaterialAssetExpressions(const FClaudeToolCall& Call);

	/** Get live Sequencer editor context */
	static FClaudeToolResult Tool_GetSequencerEditorContext(const FClaudeToolCall& Call);

	/** List all bindings in the active Level Sequence */
	static FClaudeToolResult Tool_ListSequenceBindings(const FClaudeToolCall& Call);

	/** Assert that a named binding exists in the active Level Sequence */
	static FClaudeToolResult Tool_AssertSequenceBindingExists(const FClaudeToolCall& Call);

	/** Assert the current Sequencer selection shape */
	static FClaudeToolResult Tool_AssertSequencerSelection(const FClaudeToolCall& Call);

	/** Add currently selected level actors to the active sequence */
	static FClaudeToolResult Tool_AddSelectedActorsToActiveSequence(const FClaudeToolCall& Call);

	/** Get live Control Rig editor context */
	static FClaudeToolResult Tool_GetControlRigEditorContext(const FClaudeToolCall& Call);

	/** List controls in the active or specified Control Rig */
	static FClaudeToolResult Tool_ListControlRigControls(const FClaudeToolCall& Call);

	/** Assert the current Control Rig selection */
	static FClaudeToolResult Tool_AssertControlRigSelection(const FClaudeToolCall& Call);

	/** Select a Control Rig control in the active or specified rig */
	static FClaudeToolResult Tool_SelectControlRigControl(const FClaudeToolCall& Call);

	/** Get live Niagara editor context */
	static FClaudeToolResult Tool_GetNiagaraEditorContext(const FClaudeToolCall& Call);

	/** Get detailed Niagara stack context */
	static FClaudeToolResult Tool_GetNiagaraStackContext(const FClaudeToolCall& Call);

	/** List Niagara modules by emitter and stage */
	static FClaudeToolResult Tool_ListNiagaraModules(const FClaudeToolCall& Call);

	/** Assert that a Niagara system compiles without script sync errors */
	static FClaudeToolResult Tool_AssertNiagaraCompiles(const FClaudeToolCall& Call);

	/** Get live PCG graph editor context */
	static FClaudeToolResult Tool_GetPCGEditorContext(const FClaudeToolCall& Call);

	/** List nodes in the active or specified PCG graph */
	static FClaudeToolResult Tool_ListPCGNodes(const FClaudeToolCall& Call);

	/** Inspect a single node in the active or specified PCG graph */
	static FClaudeToolResult Tool_InspectPCGNode(const FClaudeToolCall& Call);

	/** Assert that a PCG graph is structurally valid */
	static FClaudeToolResult Tool_AssertPCGGraphValid(const FClaudeToolCall& Call);

	/** Open an asset in its editor */
	static FClaudeToolResult Tool_OpenAssetEditor(const FClaudeToolCall& Call);

	/** Close all editors for an asset */
	static FClaudeToolResult Tool_CloseAssetEditor(const FClaudeToolCall& Call);

	/** Focus the primary Content Browser */
	static FClaudeToolResult Tool_FocusContentBrowser(const FClaudeToolCall& Call);

	/** Sync the Content Browser to an asset */
	static FClaudeToolResult Tool_SyncContentBrowserToAsset(const FClaudeToolCall& Call);

	/** Sync the Content Browser to a folder */
	static FClaudeToolResult Tool_SyncContentBrowserToFolder(const FClaudeToolCall& Call);

	/** Enumerate reflected properties on a UObject */
	static FClaudeToolResult Tool_ListObjectProperties(const FClaudeToolCall& Call);

	/** Read a reflected UObject property */
	static FClaudeToolResult Tool_GetObjectPropertyTyped(const FClaudeToolCall& Call);

	/** Mutate a reflected UObject property */
	static FClaudeToolResult Tool_SetObjectPropertyTyped(const FClaudeToolCall& Call);

	/** Invoke a reflected UObject function */
	static FClaudeToolResult Tool_CallReflectedFunction(const FClaudeToolCall& Call);

	/** Save a loaded asset package */
	static FClaudeToolResult Tool_SaveAsset(const FClaudeToolCall& Call);

	/** Save dirty map/content packages */
	static FClaudeToolResult Tool_SaveDirtyAssets(const FClaudeToolCall& Call);

	/** Check out an asset from source control */
	static FClaudeToolResult Tool_CheckoutAsset(const FClaudeToolCall& Call);

	/** Revert an asset through source control */
	static FClaudeToolResult Tool_RevertAsset(const FClaudeToolCall& Call);

	/** Get available and active editor modes */
	static FClaudeToolResult Tool_GetEditorModeState(const FClaudeToolCall& Call);

	/** Activate an editor mode */
	static FClaudeToolResult Tool_ActivateEditorMode(const FClaudeToolCall& Call);

	/** Deactivate an editor mode */
	static FClaudeToolResult Tool_DeactivateEditorMode(const FClaudeToolCall& Call);
	
	/** Unregister a project */
	static FClaudeToolResult Tool_UnregisterProject(const FClaudeToolCall& Call);
	
	/** Set a registered project setting */
	static FClaudeToolResult Tool_SetProjectSetting(const FClaudeToolCall& Call);
	
	/** Get a registered project setting */
	static FClaudeToolResult Tool_GetProjectSetting(const FClaudeToolCall& Call);
	
	/** Get recent lines from Unreal Engine output log */
	static FClaudeToolResult Tool_GetRecentLogs(const FClaudeToolCall& Call);
	
	// =========================================================================
	// Project Insights — Comprehensive Analysis (satellite: ProjectToolsModule_Insights.cpp)
	// =========================================================================
	
	/** Generate a 7-module project insights report */
	static FClaudeToolResult Tool_GenerateProjectInsights(const FClaudeToolCall& Call);
};
