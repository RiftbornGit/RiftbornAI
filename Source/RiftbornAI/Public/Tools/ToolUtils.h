// Copyright RiftbornAI. All Rights Reserved.
// Shared utilities for tool modules

#pragma once

#include "CoreMinimal.h"
#include "UObject/Package.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "ClaudeToolUse.h"  // For FClaudeToolCall in ToolUtils helpers

// ============================================================================
// Safe Package Creation Helper - Prevents partial load crashes (UE-234567)
// ============================================================================
// Use this instead of raw CreatePackage() to avoid:
// - "Asset cannot be saved as it has only been partially loaded" crashes
// - Double-slash path issues
// - Duplicate asset creation attempts
// ============================================================================

struct FSafePackageResult
{
    UPackage* Package = nullptr;
    bool bSuccess = false;
    FString ErrorMessage;
    bool bAlreadyExists = false;
    FString SanitizedPath;  // The validated/sanitized package path (use this for subsequent operations)
};

/**
 * Creates a package safely, with full validation to prevent crashes.
 * @param InPackagePath - The package path (e.g., /Game/Folder/AssetName)
 * @param AssetTypeName - Human-readable asset type for error messages
 * @return FSafePackageResult with Package pointer or error details
 */
inline FSafePackageResult CreatePackageSafe(const FString& InPackagePath, const FString& AssetTypeName = TEXT("Asset"))
{
    FSafePackageResult Result;

    // Must be on GameThread for UE object operations
    if (!IsInGameThread())
    {
        Result.ErrorMessage = FString::Printf(TEXT("%s creation must be called from GameThread"), *AssetTypeName);
        return Result;
    }

    // Sanitize path - remove double slashes
    FString PackagePath = InPackagePath;
    FPaths::RemoveDuplicateSlashes(PackagePath);

    // SECURITY: Block path traversal attempts
    if (PackagePath.Contains(TEXT("..")) || PackagePath.Contains(TEXT("\\")))
    {
        Result.ErrorMessage = FString::Printf(TEXT("Path traversal not allowed for %s: %s"), *AssetTypeName, *InPackagePath);
        return Result;
    }

    // SECURITY: Ensure path starts with /Game/ (Unreal Content directory)
    if (!PackagePath.StartsWith(TEXT("/Game/")))
    {
        // If no /Game/ prefix, prepend default location
        if (!PackagePath.StartsWith(TEXT("/")))
        {
            PackagePath = TEXT("/Game/") + PackagePath;
        }
        else if (!PackagePath.StartsWith(TEXT("/Game")))
        {
            Result.ErrorMessage = FString::Printf(TEXT("%s path must be within /Game/: %s"), *AssetTypeName, *InPackagePath);
            return Result;
        }
    }
    
    // VALIDATION: Check if path maps to a valid filesystem path
    FString DiskPath;
    if (!FPackageName::TryConvertLongPackageNameToFilename(PackagePath, DiskPath))
    {
        Result.ErrorMessage = FString::Printf(TEXT("Invalid package path for %s: %s"), *AssetTypeName, *PackagePath);
        return Result;
    }

    // Check if a package is already loaded at this path (could be partially loaded)
    UPackage* ExistingPackage = FindPackage(nullptr, *PackagePath);
    if (ExistingPackage)
    {
        Result.bAlreadyExists = true;
        Result.ErrorMessage = FString::Printf(TEXT("%s already exists in memory: %s"), *AssetTypeName, *PackagePath);
        return Result;
    }

    // Check if asset already exists on disk
    if (FPackageName::DoesPackageExist(PackagePath))
    {
        Result.bAlreadyExists = true;
        Result.ErrorMessage = FString::Printf(TEXT("%s already exists at: %s"), *AssetTypeName, *PackagePath);
        return Result;
    }
    
    // Check if file exists on disk (extra safety)
    FString AssetPath = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
    if (FPaths::FileExists(AssetPath))
    {
        Result.bAlreadyExists = true;
        Result.ErrorMessage = FString::Printf(TEXT("%s file already exists on disk: %s"), *AssetTypeName, *AssetPath);
        return Result;
    }

    // Create the package
    UPackage* Package = CreatePackage(*PackagePath);
    if (!Package)
    {
        Result.ErrorMessage = FString::Printf(TEXT("Failed to create package for %s at: %s"), *AssetTypeName, *PackagePath);
        return Result;
    }

    // CRITICAL: Fully load the package to prevent partial-load crashes
    Package->FullyLoad();

    Result.Package = Package;
    Result.SanitizedPath = PackagePath;  // Return the sanitized path for subsequent operations
    Result.bSuccess = true;
    return Result;
}

// ============================================================================
// Actor Lookup Helper - Used by 40+ tool modules
// O(1) via FActorLabelCache. Falls back to TActorIterator on cache miss.
// ============================================================================

#include "Tools/ActorLabelCache.h"

namespace ToolUtils
{
    /** Find an actor by its editor label in the given world. O(1) cached. */
    inline AActor* FindActorByLabel(UWorld* World, const FString& ActorLabel)
    {
        if (!World || ActorLabel.IsEmpty()) return nullptr;
        return FActorLabelCache::Get().FindByLabel(World, ActorLabel);
    }

    /** Find an actor by label in the current editor world. O(1) cached. */
    inline AActor* FindActorByLabel(const FString& ActorLabel)
    {
        UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
        return FindActorByLabel(World, ActorLabel);
    }
}

// ============================================================================
// Security Helpers
// ============================================================================

namespace ToolUtils
{
    /** Check if a path contains traversal patterns (..) or backslashes. */
    inline bool ContainsPathTraversal(const FString& Path)
    {
        return Path.Contains(TEXT("..")) || Path.Contains(TEXT("\\"));
    }

    /** Validate an asset path is safe and well-formed. Returns true if valid. */
    inline bool IsValidAssetPath(const FString& Path)
    {
        if (Path.IsEmpty()) return false;
        if (ContainsPathTraversal(Path)) return false;
        if (!Path.StartsWith(TEXT("/Game/")) && !Path.StartsWith(TEXT("/Engine/")) && !Path.StartsWith(TEXT("/Script/"))) return false;
        return true;
    }
}

// ============================================================================
// Parameter Extraction Helpers - Used by tool modules
// ============================================================================

namespace ToolUtils
{
    /** Get a string parameter from the tool call arguments */
    inline FString GetStringParam(const struct FClaudeToolCall& Call, const TCHAR* ParamName, const FString& Default = FString())
    {
        const FString& Val = Call.Arguments.FindRef(ParamName);
        return Val.IsEmpty() ? Default : Val;
    }

    /** Get an integer parameter with default value */
    inline int32 SafeAtoi(const FString& Str, int32 Default = 0)
    {
        if (Str.IsEmpty()) return Default;

        int32 Value = Default;
        return LexTryParseString(Value, *Str) ? Value : Default;
    }

    /** Get an integer parameter with default value */
    inline int32 GetIntParam(const struct FClaudeToolCall& Call, const TCHAR* ParamName, int32 Default = 0)
    {
        const FString Val = Call.Arguments.FindRef(ParamName);
        return SafeAtoi(Val, Default);
    }

    /** Parse a float from string with NaN/Infinity protection. */
    inline float SafeAtof(const FString& Str, float Default = 0.0f)
    {
        if (Str.IsEmpty()) return Default;
        float V = FCString::Atof(*Str);
        return FMath::IsFinite(V) ? V : Default;
    }

    /** Get a float parameter with default value. Rejects NaN/Infinity. */
    inline float GetFloatParam(const struct FClaudeToolCall& Call, const TCHAR* ParamName, float Default = 0.0f)
    {
        return SafeAtof(Call.Arguments.FindRef(ParamName), Default);
    }

    /** Get a bool parameter with default value */
    inline bool GetBoolParam(const struct FClaudeToolCall& Call, const TCHAR* ParamName, bool Default = false)
    {
        const FString Val = Call.Arguments.FindRef(ParamName);
        if (Val.IsEmpty()) return Default;
        return Val.Equals(TEXT("true"), ESearchCase::IgnoreCase) || Val == TEXT("1");
    }
}

// ============================================================================
