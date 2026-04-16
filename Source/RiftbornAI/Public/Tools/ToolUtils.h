// Copyright RiftbornAI. All Rights Reserved.
// Shared utilities for tool modules

#pragma once

#include "CoreMinimal.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "ClaudeToolUse.h"  // For FClaudeToolCall in ToolUtils helpers
#include "InputSanitization.h"

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
    inline FString NormalizeAbsolutePath(const FString& InPath)
    {
        FString Path = FPaths::ConvertRelativePathToFull(InPath.TrimStartAndEnd());
        FPaths::NormalizeFilename(Path);
        FPaths::CollapseRelativeDirectories(Path);
        return Path;
    }

    inline bool IsPathWithinDirectory(const FString& InPath, const FString& InDirectory)
    {
        const FString Path = NormalizeAbsolutePath(InPath);
        FString Directory = NormalizeAbsolutePath(InDirectory);
        FPaths::NormalizeDirectoryName(Directory);

        if (Path.IsEmpty() || Directory.IsEmpty())
        {
            return false;
        }

        if (!Path.StartsWith(Directory, ESearchCase::IgnoreCase))
        {
            return false;
        }

        return Path.Len() == Directory.Len() || Path[Directory.Len()] == TCHAR('/');
    }

    inline FString GetProjectSavedScreenshotsDirectory()
    {
        return NormalizeAbsolutePath(FPaths::ProjectSavedDir() / TEXT("Screenshots"));
    }

    inline bool HasSupportedScreenshotExtension(const FString& FullPath)
    {
        const FString Extension = FPaths::GetExtension(FullPath, false).ToLower();
        return Extension == TEXT("png") || Extension == TEXT("jpg") || Extension == TEXT("jpeg");
    }

    inline bool IsScreenshotFileReady(const FString& FullPath)
    {
        if (!HasSupportedScreenshotExtension(FullPath) || !FPaths::FileExists(FullPath))
        {
            return false;
        }

        const int64 FileSize = IFileManager::Get().FileSize(*FullPath);
        if (FileSize <= 0 || FileSize > static_cast<int64>(MAX_int32))
        {
            return false;
        }

        TArray<uint8> FileData;
        if (!FFileHelper::LoadFileToArray(FileData, *FullPath) || FileData.Num() <= 0)
        {
            return false;
        }

        const FString Extension = FPaths::GetExtension(FullPath, false).ToLower();
        if (Extension == TEXT("png"))
        {
            static const uint8 PngSignature[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
            static const uint8 PngTrailer[] = { 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82 };
            return FileData.Num() >= static_cast<int32>(sizeof(PngSignature) + sizeof(PngTrailer))
                && FMemory::Memcmp(FileData.GetData(), PngSignature, sizeof(PngSignature)) == 0
                && FMemory::Memcmp(FileData.GetData() + FileData.Num() - sizeof(PngTrailer), PngTrailer, sizeof(PngTrailer)) == 0;
        }

        return FileData.Num() >= 4
            && FileData[0] == 0xFF
            && FileData[1] == 0xD8
            && FileData[FileData.Num() - 2] == 0xFF
            && FileData[FileData.Num() - 1] == 0xD9;
    }

    inline bool TryResolveProjectSavedScreenshotPath(const FString& RawPath, FString& OutResolvedPath)
    {
        FString Candidate = RawPath.TrimStartAndEnd();
        if (Candidate.IsEmpty())
        {
            return false;
        }

        Candidate.ReplaceInline(TEXT("\\"), TEXT("/"));
        if (Candidate.Contains(TEXT("..")) || Candidate.StartsWith(TEXT("//")))
        {
            return false;
        }

        if (Candidate.StartsWith(TEXT("/Saved/"), ESearchCase::IgnoreCase))
        {
            Candidate = FPaths::ProjectDir() / Candidate.RightChop(1);
        }
        else if (Candidate.StartsWith(TEXT("Saved/"), ESearchCase::IgnoreCase))
        {
            Candidate = FPaths::ProjectDir() / Candidate;
        }
        else if (FPaths::IsRelative(Candidate))
        {
            return false;
        }

        Candidate = NormalizeAbsolutePath(Candidate);
        if (!HasSupportedScreenshotExtension(Candidate))
        {
            return false;
        }

        if (!IsPathWithinDirectory(Candidate, GetProjectSavedScreenshotsDirectory()))
        {
            return false;
        }

        OutResolvedPath = Candidate;
        return true;
    }

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
    inline FString NormalizeLongPackagePath(const FString& InPath)
    {
        FString Path = InPath.TrimStartAndEnd();
        FPaths::RemoveDuplicateSlashes(Path);

        const FString ObjectPackagePath = FPackageName::ObjectPathToPackageName(Path);
        if (!ObjectPackagePath.IsEmpty())
        {
            return ObjectPackagePath;
        }

        return Path;
    }

    inline FString CanonicalizeAssetObjectPath(const FString& InPath)
    {
        const FString PackagePath = NormalizeLongPackagePath(InPath);
        if (PackagePath.IsEmpty())
        {
            return PackagePath;
        }

        FString ObjectName = FPackageName::ObjectPathToObjectName(InPath);
        if (ObjectName.IsEmpty())
        {
            ObjectName = FPackageName::GetLongPackageAssetName(PackagePath);
        }

        return ObjectName.IsEmpty() ? PackagePath : PackagePath + TEXT(".") + ObjectName;
    }

    /** Check if a path contains traversal patterns (..) or backslashes. */
    inline bool ContainsPathTraversal(const FString& Path)
    {
        return FInputSanitization::ContainsPathTraversal(Path);
    }

    /** Validate an asset path is safe and well-formed. Returns true if valid. */
    inline bool IsValidAssetPath(const FString& Path)
    {
        return FInputSanitization::ValidateAssetPath(Path).bValid;
    }
}

// ============================================================================
// Parameter Extraction Helpers - Used by tool modules
// ============================================================================

namespace ToolUtils
{
    inline bool HasBoolPrefixStyle(const FString& Name)
    {
        return Name.Len() > 1 && Name[0] == TCHAR('b') && FChar::IsUpper(Name[1]);
    }

    inline void AppendBoolAliasCandidates(const FString& RequestedName, TArray<FString>& OutCandidates)
    {
        const FString Trimmed = RequestedName.TrimStartAndEnd();
        if (Trimmed.IsEmpty())
        {
            return;
        }

        OutCandidates.AddUnique(Trimmed);

        if (HasBoolPrefixStyle(Trimmed))
        {
            OutCandidates.AddUnique(Trimmed.Mid(1));
        }
        else if (FChar::IsUpper(Trimmed[0]))
        {
            OutCandidates.AddUnique(TEXT("b") + Trimmed);
        }
    }

    inline FProperty* FindPropertyByNameWithBoolAlias(const UStruct* OwnerStruct, const FString& RequestedName, FString* OutResolvedName = nullptr)
    {
        if (!OwnerStruct)
        {
            return nullptr;
        }

        if (FProperty* ExactProperty = OwnerStruct->FindPropertyByName(*RequestedName))
        {
            if (OutResolvedName)
            {
                *OutResolvedName = ExactProperty->GetName();
            }
            return ExactProperty;
        }

        TArray<FString> Candidates;
        AppendBoolAliasCandidates(RequestedName, Candidates);

        for (TFieldIterator<FProperty> It(OwnerStruct); It; ++It)
        {
            FProperty* Property = *It;
            if (!Property)
            {
                continue;
            }

            for (const FString& Candidate : Candidates)
            {
                if (Property->GetName().Equals(Candidate, ESearchCase::IgnoreCase))
                {
                    if (OutResolvedName)
                    {
                        *OutResolvedName = Property->GetName();
                    }
                    return Property;
                }
            }
        }

        return nullptr;
    }

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

    inline bool TryParseFlexibleVectorText(const FString& RawValue, FVector& OutVector)
    {
        const FString Trimmed = RawValue.TrimStartAndEnd();
        if (Trimmed.IsEmpty())
        {
            return false;
        }

        if (OutVector.InitFromString(Trimmed))
        {
            return true;
        }

        const auto ParseVectorComponent = [](const FString& RawComponent, float& OutValue) -> bool
        {
            const FString Component = RawComponent.TrimStartAndEnd();
            if (Component.IsEmpty())
            {
                return false;
            }

            double ParsedValue = 0.0;
            if (!LexTryParseString(ParsedValue, *Component) || !FMath::IsFinite(ParsedValue))
            {
                return false;
            }

            OutValue = static_cast<float>(ParsedValue);
            return true;
        };

        const auto TryParseTriplet = [&ParseVectorComponent](const TArray<FString>& Components, FVector& OutParsedVector) -> bool
        {
            if (Components.Num() != 3)
            {
                return false;
            }

            float X = 0.0f;
            float Y = 0.0f;
            float Z = 0.0f;
            if (!ParseVectorComponent(Components[0], X)
                || !ParseVectorComponent(Components[1], Y)
                || !ParseVectorComponent(Components[2], Z))
            {
                return false;
            }

            OutParsedVector = FVector(X, Y, Z);
            return true;
        };

        FString Stripped = Trimmed;
        if ((Stripped.StartsWith(TEXT("(")) && Stripped.EndsWith(TEXT(")")))
            || (Stripped.StartsWith(TEXT("[")) && Stripped.EndsWith(TEXT("]"))))
        {
            Stripped = Stripped.Mid(1, Stripped.Len() - 2).TrimStartAndEnd();
        }

        TArray<FString> Components;
        Stripped.ParseIntoArray(Components, TEXT(","), true);
        if (TryParseTriplet(Components, OutVector))
        {
            return true;
        }

        Components.Reset();
        Stripped.ParseIntoArrayWS(Components);
        if (TryParseTriplet(Components, OutVector))
        {
            return true;
        }

        TSharedPtr<FJsonObject> JsonObject;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Trimmed);
        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            bool bSawSupportedField = false;
            const auto ReadNumber = [&JsonObject, &bSawSupportedField](const TCHAR* PrimaryKey, const TCHAR* AliasKey, float DefaultValue) -> float
            {
                double Value = DefaultValue;
                if (JsonObject->TryGetNumberField(PrimaryKey, Value))
                {
                    bSawSupportedField = true;
                    return static_cast<float>(Value);
                }
                if (AliasKey && JsonObject->TryGetNumberField(AliasKey, Value))
                {
                    bSawSupportedField = true;
                    return static_cast<float>(Value);
                }
                return DefaultValue;
            };

            OutVector = FVector(
                ReadNumber(TEXT("x"), TEXT("X"), 0.0f),
                ReadNumber(TEXT("y"), TEXT("Y"), 0.0f),
                ReadNumber(TEXT("z"), TEXT("Z"), 0.0f));
            return bSawSupportedField;
        }

        return false;
    }

    inline bool TryParseFlexibleRotatorText(const FString& RawValue, FRotator& OutRotator)
    {
        const FString Trimmed = RawValue.TrimStartAndEnd();
        if (Trimmed.IsEmpty())
        {
            return false;
        }

        if (OutRotator.InitFromString(Trimmed))
        {
            return true;
        }

        TArray<FString> Components;
        Trimmed.ParseIntoArray(Components, TEXT(","), true);
        if (Components.Num() == 3)
        {
            const auto ParseRotatorComponent = [](const FString& RawComponent, float& OutValue) -> bool
            {
                const FString Component = RawComponent.TrimStartAndEnd();
                if (Component.IsEmpty())
                {
                    return false;
                }

                double ParsedValue = 0.0;
                if (!LexTryParseString(ParsedValue, *Component) || !FMath::IsFinite(ParsedValue))
                {
                    return false;
                }

                OutValue = static_cast<float>(ParsedValue);
                return true;
            };

            float Pitch = 0.0f;
            float Yaw = 0.0f;
            float Roll = 0.0f;
            if (!ParseRotatorComponent(Components[0], Pitch)
                || !ParseRotatorComponent(Components[1], Yaw)
                || !ParseRotatorComponent(Components[2], Roll))
            {
                return false;
            }

            OutRotator = FRotator(Pitch, Yaw, Roll);
            return true;
        }

        TSharedPtr<FJsonObject> JsonObject;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Trimmed);
        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            bool bSawSupportedField = false;
            const auto ReadNumber = [&JsonObject](const TCHAR* PrimaryKey, const TCHAR* AliasKey, float DefaultValue) -> float
            {
                double Value = DefaultValue;
                if (JsonObject->TryGetNumberField(PrimaryKey, Value))
                {
                    return static_cast<float>(Value);
                }
                if (AliasKey && JsonObject->TryGetNumberField(AliasKey, Value))
                {
                    return static_cast<float>(Value);
                }
                return DefaultValue;
            };

            if (JsonObject->HasTypedField<EJson::Number>(TEXT("pitch"))
                || JsonObject->HasTypedField<EJson::Number>(TEXT("p"))
                || JsonObject->HasTypedField<EJson::Number>(TEXT("yaw"))
                || JsonObject->HasTypedField<EJson::Number>(TEXT("y"))
                || JsonObject->HasTypedField<EJson::Number>(TEXT("roll"))
                || JsonObject->HasTypedField<EJson::Number>(TEXT("r")))
            {
                bSawSupportedField = true;
            }

            if (!bSawSupportedField)
            {
                return false;
            }

            OutRotator = FRotator(
                ReadNumber(TEXT("pitch"), TEXT("p"), 0.0f),
                ReadNumber(TEXT("yaw"), TEXT("y"), 0.0f),
                ReadNumber(TEXT("roll"), TEXT("r"), 0.0f));
            return true;
        }

        return false;
    }
}

// ============================================================================
