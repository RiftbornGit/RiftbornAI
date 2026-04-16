// Copyright RiftbornAI. All Rights Reserved.
// FileSystemTools - Tools for writing C++ code, config files, etc.
//
// This enables the agent to actually write code files, not just plan them.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"

/**
 * Result of a file operation
 */
struct RIFTBORNAI_API FFileOperationResult
{
    bool bSuccess = false;
    FString FilePath;
    FString Message;
    FString ErrorMessage;
};

/**
 * C++ file generation context
 */
struct RIFTBORNAI_API FCppFileContext
{
    FString ModuleName;
    FString ClassName;
    FString ParentClass;
    FString IncludePath;
    TArray<FString> AdditionalIncludes;
    TArray<FString> ForwardDeclarations;
    bool bIsActorClass = false;
    bool bIsComponentClass = false;
    bool bGenerateReflection = true;  // UCLASS/UPROPERTY etc.
};

/**
 * FFileSystemTools
 * 
 * Provides tools for:
 * - Writing C++ header/source files
 * - Modifying .Build.cs for new modules
 * - Writing config files
 * - Safe file operations with backup
 */
class RIFTBORNAI_API FFileSystemTools
{
public:
    static FFileSystemTools& Get();
    
    // =========================================================================
    // INITIALIZATION
    // =========================================================================
    
    /** Set the project root */
    void SetProjectRoot(const FString& InProjectRoot);
    
    /** Get current project root */
    FString GetProjectRoot() const { return ProjectRoot; }
    
    // =========================================================================
    // C++ FILE GENERATION
    // =========================================================================
    
    /**
     * Write a C++ header file
     * @param RelativePath - Path relative to Source/ (e.g., "MyGame/Public/MyClass.h")
     * @param Content - Full header content
     * @return Result of operation
     */
    FFileOperationResult WriteHeader(const FString& RelativePath, const FString& Content);
    
    /**
     * Write a C++ source file
     */
    FFileOperationResult WriteSource(const FString& RelativePath, const FString& Content);
    
    /**
     * Generate a complete C++ class (header + source)
     */
    FFileOperationResult GenerateCppClass(const FCppFileContext& Context, const FString& HeaderContent, const FString& SourceContent);
    
    /**
     * Generate a UCLASS header from template
     */
    FString GenerateUClassHeader(const FCppFileContext& Context);
    
    /**
     * Generate a UCLASS source from template
     */
    FString GenerateUClassSource(const FCppFileContext& Context);
    
    // =========================================================================
    // BUILD CONFIGURATION
    // =========================================================================
    
    /**
     * Add a module dependency to a .Build.cs file
     */
    FFileOperationResult AddModuleDependency(const FString& ModuleName, const FString& DependencyName);
    
    /**
     * Create a new module (folder structure + .Build.cs)
     */
    FFileOperationResult CreateModule(const FString& ModuleName, const TArray<FString>& Dependencies);
    
    // =========================================================================
    // CONFIG FILES
    // =========================================================================
    
    /**
     * Write to DefaultGame.ini or other config
     */
    FFileOperationResult WriteConfig(const FString& ConfigName, const FString& Section, const TMap<FString, FString>& Values);
    
    /**
     * Append to a config file
     */
    FFileOperationResult AppendConfig(const FString& ConfigName, const FString& Content);
    
    // =========================================================================
    // GENERIC FILE OPERATIONS
    // =========================================================================
    
    /**
     * Read a file
     */
    FFileOperationResult ReadFile(const FString& RelativePath, FString& OutContent);
    
    /**
     * Write any file (with safety checks)
     */
    FFileOperationResult WriteFile(const FString& RelativePath, const FString& Content, bool bCreateBackup = true);
    
    /**
     * Check if a file exists
     */
    bool FileExists(const FString& RelativePath) const;
    
    /**
     * Create a backup of a file
     */
    FFileOperationResult BackupFile(const FString& RelativePath);
    
    /**
     * Delete a file (with backup)
     */
    FFileOperationResult DeleteFile(const FString& RelativePath, bool bCreateBackup = true);
    
    // =========================================================================
    // TOOL INTEGRATION
    // =========================================================================
    
    /** Register file system tools with the tool registry */
    static void RegisterTools();
    
    /** Tool handlers */
    static FClaudeToolResult Tool_WriteFile(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ReadFile(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EditFile(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ApplyUnifiedDiff(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateCppClass(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListSourceFiles(const FClaudeToolCall& Call);
    
private:
    FFileSystemTools() = default;
    
    FString ProjectRoot;
    
    // Safety checks
    bool IsPathSafe(const FString& RelativePath) const;
    FString GetAbsolutePath(const FString& RelativePath) const;
    FString SanitizePath(const FString& Path) const;
};
