// Copyright RiftbornAI. All Rights Reserved.
// Shared path security helpers for ToolImpl_*.cpp satellite files.
// Extracted from ClaudeToolUse.cpp static helpers during H1b/H1c decomposition.
//
// NOTE: FSafePackageResult, CreatePackageSafe, IsValidPackagePath are in Tools/ToolUtils.h
// NOTE: GetEditorActorSubsystem is available via Editor.h / EditorActorSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "EngineUtils.h"         // TActorIterator
#include "GameFramework/Actor.h" // AActor

namespace ToolImplHelpers
{

/**
 * Validates and sanitizes a file path to prevent path traversal attacks.
 * Ensures all paths are confined within the project directory.
 */
inline bool ValidateAndSanitizePath(const FString& RelativePath, FString& OutFullPath, FString& OutError)
{
	for (int32 i = 0; i < RelativePath.Len(); i++)
	{
		if (RelativePath[i] == TCHAR('\0'))
		{
			OutError = TEXT("Path contains null bytes - potential security attack");
			return false;
		}
	}
	
	if (RelativePath.Contains(TEXT("..")))
	{
		OutError = TEXT("Path traversal (..) not allowed - security violation");
		return false;
	}
	
	if (RelativePath.StartsWith(TEXT("\\\\")) || RelativePath.StartsWith(TEXT("//")))
	{
		OutError = TEXT("UNC/network paths not allowed - security violation");
		return false;
	}
	
	if (RelativePath.Len() > 1 && RelativePath[1] == TEXT(':'))
	{
		OutError = TEXT("Absolute paths not allowed - use relative paths from project directory");
		return false;
	}
	
	if (RelativePath.StartsWith(TEXT("/")))
	{
		OutError = TEXT("Absolute paths not allowed - use relative paths from project directory");
		return false;
	}
	
	FString ProjectDir = FPaths::ProjectDir();
	OutFullPath = ProjectDir / RelativePath;
	
	FPaths::CollapseRelativeDirectories(OutFullPath);
	
	FString NormalizedProjectDir = FPaths::ConvertRelativePathToFull(ProjectDir);
	FString NormalizedFullPath = FPaths::ConvertRelativePathToFull(OutFullPath);
	
	FPaths::NormalizeDirectoryName(NormalizedProjectDir);
	FPaths::NormalizeDirectoryName(NormalizedFullPath);
	
	if (!NormalizedFullPath.StartsWith(NormalizedProjectDir))
	{
		OutError = FString::Printf(TEXT("Path escapes project directory - security violation. Resolved: %s, Project: %s"), 
			*NormalizedFullPath, *NormalizedProjectDir);
		return false;
	}
	
	return true;
}

/**
 * Validates an asset path (Content paths like /Game/...).
 * These should stay within the /Game/ hierarchy.
 */
inline bool ValidateAssetPath(const FString& AssetPath, FString& OutError)
{
	for (int32 i = 0; i < AssetPath.Len(); i++)
	{
		if (AssetPath[i] == TCHAR('\0'))
		{
			OutError = TEXT("Asset path contains null bytes - potential security attack");
			return false;
		}
	}
	if (AssetPath.Contains(TEXT("..")))
	{
		OutError = TEXT("Asset path traversal (..) not allowed");
		return false;
	}
	if (!AssetPath.StartsWith(TEXT("/")))
	{
		OutError = TEXT("Asset paths must start with /");
		return false;
	}
	if (AssetPath.StartsWith(TEXT("/Engine/")) && !AssetPath.StartsWith(TEXT("/Engine/BasicShapes")))
	{
		OutError = TEXT("Cannot modify Engine content - only project content allowed");
		return false;
	}
	return true;
}

/**
 * Shared actor lookup by label with case-insensitive and prefix fallback.
 * Use this instead of per-module copies to get consistent matching behavior.
 *
 * Search order: exact match → case-insensitive → prefix match.
 */
inline AActor* FindActorByLabelRobust(UWorld* World, const FString& RawLabel)
{
	if (!World) return nullptr;

	const FString Label = RawLabel.TrimStartAndEnd();
	if (Label.IsEmpty()) return nullptr;

	// Pass 1: Exact match (fastest)
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->GetActorLabel() == Label) return *It;
	}

	// Pass 2: Case-insensitive
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->GetActorLabel().Equals(Label, ESearchCase::IgnoreCase)) return *It;
	}

	// Pass 3: Prefix (e.g. "Sun" matches "Sun_Forest")
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->GetActorLabel().StartsWith(Label, ESearchCase::IgnoreCase)) return *It;
	}

	return nullptr;
}

} // namespace ToolImplHelpers
