// ProjectConfigToolsModule.h
// Project configuration and Build.cs management tools.
// Automates module dependency management,
// DataTable asset creation, C++ component wiring, and project capability verification.
//
// Tools: modify_build_cs, get_build_cs_info, ensure_module_dependencies,
//        create_datatable_asset, add_cpp_component_to_actor, verify_project_capabilities

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Project Config Tools Module
 *
 * Manages project-level configuration that code generation tools depend on:
 *
 * - modify_build_cs:              Add module dependencies to the project's Build.cs file
 * - get_build_cs_info:            Read current module dependencies from Build.cs
 * - ensure_module_dependencies:   Map #include files to required modules and add them
 * - create_datatable_asset:       Create a UDataTable asset in the editor with optional row data
 * - add_cpp_component_to_actor:   Wire a component into an existing C++ actor class
 * - verify_project_capabilities:  Diagnostic checklist for MMO feature readiness
 */
class RIFTBORNAI_API FProjectConfigToolsModule : public TToolModuleBase<FProjectConfigToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("ProjectConfigTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// Tool implementations
	static FClaudeToolResult Tool_ModifyBuildCs(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_GetBuildCsInfo(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_EnsureModuleDependencies(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_CreateDataTableAsset(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_AddCppComponentToActor(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_VerifyProjectCapabilities(const FClaudeToolCall& Call);

private:
	// ---------------------------------------------------------------------------
	// Build.cs helpers
	// ---------------------------------------------------------------------------

	/** Locate the project's Build.cs file path. Returns empty string on failure. */
	static FString FindBuildCsPath();

	/** Parse module names from a dependency line like: PublicDependencyModuleNames.AddRange(new string[] { "Core", "Engine" }); */
	static TArray<FString> ParseModuleNamesFromLine(const FString& Line);

	/**
	 * Parse all dependencies from Build.cs content.
	 * @param Content     Full text of Build.cs
	 * @param OutPublic   Populated with public module names
	 * @param OutPrivate  Populated with private module names
	 * @param OutPCH      PCH usage mode string (if found)
	 */
	static void ParseBuildCsDependencies(
		const FString& Content,
		TArray<FString>& OutPublic,
		TArray<FString>& OutPrivate,
		FString& OutPCH);

	/**
	 * Insert module names into a Build.cs dependency block.
	 * @param InOutContent   Build.cs content (modified in place)
	 * @param Modules        Module names to add
	 * @param bPublic        true = PublicDependencyModuleNames, false = Private
	 * @param OutAdded       Module names that were actually inserted (not already present)
	 * @return true if content was modified
	 */
	static bool InsertModulesIntoBuildCs(
		FString& InOutContent,
		const TArray<FString>& Modules,
		bool bPublic,
		TArray<FString>& OutAdded);

	/** Create a backup of a file (appends .bak timestamp). Returns backup path. */
	static FString BackupFile(const FString& FilePath);

	// ---------------------------------------------------------------------------
	// Include-to-module mapping
	// ---------------------------------------------------------------------------

	/** Map a C++ header include to its required UE module name. Returns empty if no mapping. */
	static FString MapIncludeToModule(const FString& IncludePath);

	// ---------------------------------------------------------------------------
	// Source file helpers (for add_cpp_component_to_actor)
	// ---------------------------------------------------------------------------

	/** Find the .h and .cpp files for a given class name in the project Source directory. */
	static bool FindClassSourceFiles(
		const FString& ClassName,
		FString& OutHeaderPath,
		FString& OutSourcePath);

	/** Insert a #include directive into a file if not already present. */
	static bool EnsureInclude(FString& InOutContent, const FString& IncludePath);

	/** Insert a UPROPERTY + member declaration into a header class body. */
	static bool InsertPropertyDeclaration(
		FString& InOutHeader,
		const FString& ClassName,
		const FString& ComponentType,
		const FString& ComponentName);

	/** Insert CreateDefaultSubobject into the constructor body of a .cpp file. */
	static bool InsertCreateDefaultSubobject(
		FString& InOutSource,
		const FString& ClassName,
		const FString& ComponentType,
		const FString& ComponentName);
};
