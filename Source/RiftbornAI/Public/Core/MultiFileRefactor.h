// Copyright RiftbornAI. All Rights Reserved.
// MultiFileRefactor - Atomic multi-file editing operations
// Gap #5 (renumbered): Multi-file refactoring

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"

/**
 * A single file edit within a multi-file refactoring operation.
 * Each edit is a search-and-replace within one file.
 */
struct RIFTBORNAI_API FFileEdit
{
	/** Relative path from project root (e.g., "Source/MyGame/MyClass.cpp") */
	FString RelativePath;
	
	/** Resolved absolute path */
	FString AbsolutePath;
	
	/** Text to find */
	FString OldText;
	
	/** Text to replace with */
	FString NewText;
	
	/** Description of this edit */
	FString Description;
	
	/** Original file content (for rollback) */
	FString OriginalContent;
	
	/** New file content (after edit) */
	FString NewContent;
	
	/** Was this edit applied? */
	bool bApplied = false;
	
	/** Did this edit succeed? */
	bool bSuccess = false;
	
	/** Error message if failed */
	FString ErrorMessage;
};

/**
 * Result of a multi-file refactoring operation.
 */
struct RIFTBORNAI_API FRefactorResult
{
	bool bSuccess = false;
	int32 FilesModified = 0;
	int32 TotalEdits = 0;
	int32 FailedEdits = 0;
	TArray<FString> ModifiedFiles;
	TArray<FString> Errors;
	TArray<FString> Warnings;
	
	/** Backup directory (for rollback) */
	FString BackupDirectory;
	
	/** Can be rolled back? */
	bool bCanRollback = false;
};

/**
 * Multi-File Refactoring Engine
 * 
 * Performs atomic multi-file edits with backup and rollback.
 * All edits in a refactoring operation either ALL succeed or ALL are rolled back.
 * 
 * Operations:
 *   - RenameSymbol: Rename a class/function/variable across all files
 *   - MoveToModule: Move a class to a different module
 *   - ExtractFunction: Extract code into a new function
 *   - BatchEdit: Apply multiple search-and-replace edits atomically
 */
class RIFTBORNAI_API FMultiFileRefactor
{
public:
	static FMultiFileRefactor& Get();
	
	// =========================================================================
	// High-Level Operations
	// =========================================================================
	
	/**
	 * Rename a symbol (class, function, variable) across all source files.
	 * Searches both Source/ and Plugins/ directories.
	 * 
	 * @param OldName Current symbol name
	 * @param NewName New symbol name
	 * @param FilePattern Glob pattern (default: *.cpp and *.h)
	 * @return Result with files modified and any errors
	 */
	FRefactorResult RenameSymbol(
		const FString& OldName,
		const FString& NewName,
		const FString& FilePattern = TEXT(""));
	
	/**
	 * Apply a batch of edits atomically.
	 * If any edit fails validation, NONE are applied.
	 * 
	 * @param Edits Array of file edits to apply
	 * @param Description Human-readable description for the refactoring
	 * @return Result with details
	 */
	FRefactorResult BatchEdit(
		TArray<FFileEdit>& Edits,
		const FString& Description);
	
	/**
	 * Dry-run a batch of edits — validate without applying.
	 * 
	 * @param Edits Array of file edits to validate
	 * @return Result with validation errors (if any)
	 */
	FRefactorResult DryRunBatchEdit(const TArray<FFileEdit>& Edits);
	
	/**
	 * Rollback the last refactoring operation.
	 * Uses the backup directory to restore original files.
	 * 
	 * @return True if rollback succeeded
	 */
	bool RollbackLast();
	
	/**
	 * Find all occurrences of a string across project source files.
	 * 
	 * @param SearchText Text to find
	 * @param FilePattern File pattern to search (default: *.cpp;*.h)
	 * @param MaxResults Maximum results to return
	 * @return Array of "filepath:line:column: context" strings
	 */
	TArray<FString> FindInProject(
		const FString& SearchText,
		const FString& FilePattern = TEXT(""),
		int32 MaxResults = 100);
	
	// =========================================================================
	// Tool Registration
	// =========================================================================
	
	/** Register refactoring tools with the tool registry */
	static void RegisterTools();
	
	// Tool implementations
	static FClaudeToolResult Tool_MultiFileEdit(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_FindInProject(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RenameSymbolInProject(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_PreviewRefactor(const FClaudeToolCall& Call);
	static FClaudeToolResult Tool_RollbackRefactor(const FClaudeToolCall& Call);

private:
	FMultiFileRefactor() = default;
	
	/** Collect all source files matching pattern */
	TArray<FString> CollectSourceFiles(const FString& FilePattern) const;
	
	/** Create backup of files before modification */
	FString CreateBackup(const TArray<FFileEdit>& Edits);
	
	/** Restore files from backup directory */
	bool RestoreFromBackup(const FString& BackupDir);
	
	/** Validate a single edit (check file exists, old_text found, etc.) */
	bool ValidateEdit(FFileEdit& Edit, FString& OutError) const;
	
	/** Apply a single edit to disk */
	bool ApplyEdit(FFileEdit& Edit);
	
	/** Last backup directory for rollback */
	FString LastBackupDirectory;
	
	/** Lock for thread safety */
	mutable FCriticalSection RefactorLock;
};
