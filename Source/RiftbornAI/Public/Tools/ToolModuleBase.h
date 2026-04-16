// Copyright RiftbornAI. All Rights Reserved.
// Base class for modular tool registration

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"

/**
 * Base class for modular tool registration.
 * Each tool category (Blueprint, Level, Asset, etc.) should inherit from this
 * and implement RegisterTools().
 * 
 * Usage:
 *   class FBlueprintTools : public IToolModule
 *   {
 *   public:
 *       virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
 *       
 *       // Tool implementations
 *       static FString Tool_CreateBlueprint(const TMap<FString, FString>& Args);
 *       static FString Tool_OpenBlueprint(const TMap<FString, FString>& Args);
 *   };
 */
class RIFTBORNAI_API IToolModule
{
public:
    virtual ~IToolModule() = default;
    
    /** Register all tools this module provides */
    virtual void RegisterTools(FClaudeToolRegistry& Registry) = 0;
    
    /** Get module name for logging/debugging */
    virtual FString GetModuleName() const = 0;
    
    /** Get tool count for statistics */
    virtual int32 GetToolCount() const = 0;
};

/**
 * Template helper for quickly creating tool modules.
 */
template<typename T>
class TToolModuleBase : public IToolModule
{
public:
    virtual FString GetModuleName() const override { return T::StaticModuleName(); }
    
protected:
    int32 RegisteredToolCount = 0;
    
    /** Helper to register a tool and increment counter */
    void RegisterToolInternal(FClaudeToolRegistry& Registry, const FClaudeTool& Tool, FOnExecuteTool Handler)
    {
        Registry.RegisterTool(Tool, Handler);
        RegisteredToolCount++;
    }
    
    /** Helper to register a tool with category and increment counter */
    void RegisterToolWithCategory(FClaudeToolRegistry& Registry, FClaudeTool& Tool, const FString& Category, FOnExecuteTool Handler)
    {
        Tool.Category = Category;
        Registry.RegisterTool(Tool, Handler);
        RegisteredToolCount++;
    }
    
public:
    virtual int32 GetToolCount() const override { return RegisteredToolCount; }
};

/**
 * Macro to simplify tool module creation.
 * 
 * DECLARE_TOOL_MODULE(BlueprintTools)
 * expands to a class with proper boilerplate.
 */
#define DECLARE_TOOL_MODULE(ModuleName) \
    class RIFTBORNAI_API F##ModuleName##Tools : public TToolModuleBase<F##ModuleName##Tools> \
    { \
    public: \
        static FString StaticModuleName() { return TEXT(#ModuleName); } \
        virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

#define END_TOOL_MODULE() };

/**
 * Legacy module registry for dynamic tool-module loading.
 *
 * The production tool surface registers modules explicitly through the
 * built-in registry bootstrap. This registry is intentionally unused in the
 * governed/public path until the architecture is intentionally unified.
 */
class RIFTBORNAI_API FToolModuleRegistry
{
public:
    static FToolModuleRegistry& Get()
    {
        static FToolModuleRegistry Instance;
        return Instance;
    }
    
    /** Register a tool module */
    void RegisterModule(TSharedPtr<IToolModule> Module)
    {
        if (Module.IsValid())
        {
            Modules.Add(Module);
        }
    }
    
    /** Register all tools from all modules */
    void RegisterAllModuleTools(FClaudeToolRegistry& Registry)
    {
        for (auto& Module : Modules)
        {
            if (Module.IsValid())
            {
                Module->RegisterTools(Registry);
                UE_LOG(LogTemp, Log, TEXT("ToolModuleRegistry: Registered %d tools from %s"), 
                    Module->GetToolCount(), *Module->GetModuleName());
            }
        }
    }
    
    /** Get total registered tool count */
    int32 GetTotalToolCount() const
    {
        int32 Total = 0;
        for (const auto& Module : Modules)
        {
            if (Module.IsValid())
            {
                Total += Module->GetToolCount();
            }
        }
        return Total;
    }
    
    /** Get module names for debugging */
    TArray<FString> GetModuleNames() const
    {
        TArray<FString> Names;
        for (const auto& Module : Modules)
        {
            if (Module.IsValid())
            {
                Names.Add(Module->GetModuleName());
            }
        }
        return Names;
    }
    
private:
    TArray<TSharedPtr<IToolModule>> Modules;
};
