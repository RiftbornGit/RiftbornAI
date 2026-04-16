// Copyright 2025 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AgentTool.h"

/**
 * Tools for project structure awareness - modules, files, config, data tables, dependencies
 */

// Tool: get_project_modules - List all modules in the project
class RIFTBORNAI_API FGetProjectModulesTool : public FAgentTool
{
public:
    FGetProjectModulesTool();
    virtual FString GetName() const override { return TEXT("get_project_modules"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_source_files - List source files in project
class RIFTBORNAI_API FGetSourceFilesTool : public FAgentTool
{
public:
    FGetSourceFilesTool();
    virtual FString GetName() const override { return TEXT("get_source_files"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_blueprint_files - List Blueprint files in project
class RIFTBORNAI_API FGetBlueprintFilesTool : public FAgentTool
{
public:
    FGetBlueprintFilesTool();
    virtual FString GetName() const override { return TEXT("get_blueprint_files"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_config_files - List and read config files
class RIFTBORNAI_API FGetConfigFilesTool : public FAgentTool
{
public:
    FGetConfigFilesTool();
    virtual FString GetName() const override { return TEXT("get_config_files"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_data_tables - List data tables in project
class RIFTBORNAI_API FGetDataTablesTool : public FAgentTool
{
public:
    FGetDataTablesTool();
    virtual FString GetName() const override { return TEXT("get_data_tables"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_data_table_contents - Read data table rows
class RIFTBORNAI_API FGetDataTableContentsTool : public FAgentTool
{
public:
    FGetDataTableContentsTool();
    virtual FString GetName() const override { return TEXT("get_data_table_contents"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_plugin_info - Get info about enabled plugins
class RIFTBORNAI_API FGetPluginInfoTool : public FAgentTool
{
public:
    FGetPluginInfoTool();
    virtual FString GetName() const override { return TEXT("get_plugin_info"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_module_dependencies - Get dependencies for a module
class RIFTBORNAI_API FGetModuleDependenciesTool : public FAgentTool
{
public:
    FGetModuleDependenciesTool();
    virtual FString GetName() const override { return TEXT("get_module_dependencies"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: search_project_files - Search for files by name or content
class RIFTBORNAI_API FSearchProjectFilesTool : public FAgentTool
{
public:
    FSearchProjectFilesTool();
    virtual FString GetName() const override { return TEXT("search_project_files"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_build_targets - Get build targets for the project
class RIFTBORNAI_API FGetBuildTargetsTool : public FAgentTool
{
public:
    FGetBuildTargetsTool();
    virtual FString GetName() const override { return TEXT("get_build_targets"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: get_content_folders - Get content folder structure
class RIFTBORNAI_API FGetContentFoldersTool : public FAgentTool
{
public:
    FGetContentFoldersTool();
    virtual FString GetName() const override { return TEXT("get_content_folders"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Tool: search_source_code - Search source code for patterns
class RIFTBORNAI_API FSearchSourceCodeTool : public FAgentTool
{
public:
    FSearchSourceCodeTool();
    virtual FString GetName() const override { return TEXT("search_source_code"); }
    virtual FString GetDescription() const override;
    virtual TArray<FToolParameter> GetParameters() const override;
    virtual FToolResult Execute(const TMap<FString, FString>& Parameters, UWorld* World) override;
};

// Registration wrapper class
class RIFTBORNAI_API FProjectStructureAwarenessTools
{
public:
    static void RegisterTools(FAgentToolRegistry& Registry);
};
