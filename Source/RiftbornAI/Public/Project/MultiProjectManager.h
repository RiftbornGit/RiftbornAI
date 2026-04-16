// Copyright RiftbornAI. All Rights Reserved.
// Multi-Project Support - ENH-008

#pragma once

#include "CoreMinimal.h"
#include "ProjectContext.h"

/**
 * Registered project information
 */
struct RIFTBORNAI_API FProjectInfo
{
	/** Unique identifier for this project */
	FString ProjectId;
	
	/** Display name */
	FString ProjectName;
	
	/** Path to .uproject file */
	FString ProjectPath;
	
	/** Engine version */
	FString EngineVersion;
	
	/** Last accessed timestamp */
	FDateTime LastAccessed;
	
	/** Is this the currently active project */
	bool bIsActive = false;
	
	/** Project-specific settings/preferences */
	TMap<FString, FString> Settings;
	
	/** Cached context data */
	FProjectContextData CachedContext;
};

/**
 * Multi-Project Manager
 * 
 * Enables working with multiple Unreal projects from a single RiftbornAI instance.
 * Tracks project-specific state, enables switching between projects, and maintains
 * separate contexts for each project.
 * 
 * ENH-008: Multi-Project Support
 */
class RIFTBORNAI_API FMultiProjectManager
{
public:
	static FMultiProjectManager& Get();
	
	// =========================================================================
	// Project Registration
	// =========================================================================
	
	/**
	 * Register a project with the manager
	 * @param ProjectPath - Path to .uproject file
	 * @return Project ID
	 */
	FString RegisterProject(const FString& ProjectPath);
	
	/**
	 * Unregister a project
	 * @param ProjectId - ID returned from RegisterProject
	 * @return True if removed
	 */
	bool UnregisterProject(const FString& ProjectId);
	
	/**
	 * Get all registered projects
	 */
	TArray<FProjectInfo> GetAllProjects() const;
	
	/**
	 * Get project info by ID
	 */
	FProjectInfo* GetProject(const FString& ProjectId);
	const FProjectInfo* GetProject(const FString& ProjectId) const;
	
	// =========================================================================
	// Active Project Management
	// =========================================================================
	
	/**
	 * Switch to a different project context
	 * Note: This doesn't open the project in UE, but changes which project
	 * RiftbornAI tools operate on when using project paths.
	 * @param ProjectId - Target project ID
	 * @return True if switch successful
	 */
	bool SetActiveProject(const FString& ProjectId);
	
	/**
	 * Get the currently active project
	 */
	FProjectInfo* GetActiveProject();
	const FProjectInfo* GetActiveProject() const;
	
	/**
	 * Get the active project ID
	 */
	FString GetActiveProjectId() const;
	
	/**
	 * Detect and register the current open project
	 */
	FString DetectCurrentProject();
	
	// =========================================================================
	// Project Context
	// =========================================================================
	
	/**
	 * Get context for a specific project
	 * @param ProjectId - Project to get context for (empty = active)
	 */
	FProjectContextData GetProjectContext(const FString& ProjectId = TEXT("")) const;
	
	/**
	 * Refresh context cache for a project
	 */
	void RefreshProjectContext(const FString& ProjectId);
	
	// =========================================================================
	// Cross-Project Operations
	// =========================================================================
	
	/**
	 * Resolve an asset path relative to a project
	 * @param AssetPath - Asset path (may be relative)
	 * @param ProjectId - Project context (empty = active)
	 * @return Fully qualified asset path
	 */
	FString ResolveAssetPath(const FString& AssetPath, const FString& ProjectId = TEXT("")) const;
	
	/**
	 * Copy an asset between projects
	 * @param SourcePath - Source asset path
	 * @param SourceProjectId - Source project
	 * @param DestPath - Destination path
	 * @param DestProjectId - Destination project
	 * @return True if copy successful
	 */
	bool CopyAssetBetweenProjects(
		const FString& SourcePath, const FString& SourceProjectId,
		const FString& DestPath, const FString& DestProjectId);
	
	// =========================================================================
	// Project Settings
	// =========================================================================
	
	/**
	 * Set a project-specific setting
	 */
	void SetProjectSetting(const FString& ProjectId, const FString& Key, const FString& Value);
	
	/**
	 * Get a project-specific setting
	 */
	FString GetProjectSetting(const FString& ProjectId, const FString& Key) const;
	
	// =========================================================================
	// Persistence
	// =========================================================================
	
	/**
	 * Save project list to disk
	 */
	bool SaveProjectList();
	
	/**
	 * Load project list from disk
	 */
	bool LoadProjectList();
	
private:
	FMultiProjectManager();
	
	/** All registered projects */
	TMap<FString, FProjectInfo> Projects;
	
	/** Currently active project ID */
	FString ActiveProjectId;
	
	/** Path to save project list */
	FString GetProjectListPath() const;
	
	/** Generate a unique project ID */
	FString GenerateProjectId(const FString& ProjectPath) const;
};
