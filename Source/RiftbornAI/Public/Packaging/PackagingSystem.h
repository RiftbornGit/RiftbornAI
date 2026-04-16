// PackagingSystem.h - Build, Package, and Deploy Games
// Handles project packaging, platform targeting, and deployment from descriptions

#pragma once

#include "CoreMinimal.h"
#include "PackagingSystem.generated.h"

// ============================================================================
// BUILD TYPES
// ============================================================================

UENUM(BlueprintType)
enum class ETargetPlatform : uint8
{
    Windows,
    Mac,
    Linux,
    Android,
    iOS,
    PS5,
    XboxSeriesX,
    Switch,
    Steam,          // Windows with Steam integration
    EpicGames,      // Windows with EOS
    WebGL           // Browser-based
};

UENUM(BlueprintType)
enum class EPackageBuildConfig : uint8
{
    Debug,
    DebugGame,
    Development,
    Shipping,
    Testing   // Renamed from Test to avoid conflict with UE Test macro
};

UENUM(BlueprintType)
enum class ECompressionType : uint8
{
    None,
    Zlib,
    Oodle,
    LZ4
};

UENUM(BlueprintType)
enum class EPackageStatus : uint8
{
    NotStarted,
    Cooking,
    Packaging,
    Compressing,
    Signing,
    Uploading,
    Complete,
    Failed
};

// ============================================================================
// PACKAGING STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FPackagePlatformSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    ETargetPlatform Platform = ETargetPlatform::Windows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString TargetArchitecture = TEXT("x64");  // x64, ARM64, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString MinOSVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bSupportsRayTracing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bSupportsNanite = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bSupportsLumen = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 MinMemoryMB = 4096;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 MinVRAMMB = 2048;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FVersionInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 Major = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 Minor = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 Patch = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 BuildNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString BuildTag;  // "alpha", "beta", "rc1", etc.

    FString ToString() const
    {
        FString Version = FString::Printf(TEXT("%d.%d.%d"), Major, Minor, Patch);
        if (!BuildTag.IsEmpty())
        {
            Version += TEXT("-") + BuildTag;
        }
        return Version;
    }
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FPackagingProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ProfileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<ETargetPlatform> TargetPlatforms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EPackageBuildConfig Configuration = EPackageBuildConfig::Shipping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    ECompressionType Compression = ECompressionType::Oodle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bUsePak = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bEncryptPak = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString EncryptionKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bSignPackage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bCreateInstaller = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bIncludeDebugSymbols = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bCompressTextures = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 TextureMaxSize = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FString> ExcludedFolders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FString> MapsToInclude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVersionInfo Version;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FStoreMetadata
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString GameTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ShortDescription;  // ~150 chars

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString FullDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FString> Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Developer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Publisher;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Genre;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ContentRating;  // "E", "T", "M", etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FString> Features;  // "Single-player", "Multi-player", etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FString> Languages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString IconPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FString> ScreenshotPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString TrailerUrl;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FDeploymentTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString TargetName;  // "Steam", "Itch.io", "Epic", etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString AppId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString DepotId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Username;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Branch = TEXT("default");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TMap<FString, FString> Credentials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bAutoPublish = false;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FPackageResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bSuccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString OutputPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int64 PackageSizeBytes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float DurationSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FString> Warnings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FString> Errors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EPackageStatus Status = EPackageStatus::NotStarted;
};

// ============================================================================
// PACKAGING SYSTEM
// ============================================================================

class RIFTBORNAI_API FPackagingSystem
{
public:
    static FPackagingSystem& Get()
    {
        static FPackagingSystem Instance;
        return Instance;
    }

    // ========================================================================
    // CONFIGURATION
    // ========================================================================

    // Create packaging profile from description:
    // "Steam release for Windows with compression and encryption"
    FPackagingProfile GenerateProfile(const FString& Description);

    // Configure platform settings
    FPackagePlatformSettings GetPlatformSettings(ETargetPlatform Platform);

    // Get recommended settings for target audience.
    // Accepted labels include the legacy "AAA" audience string for compatibility.
    FPackagingProfile GetRecommendedProfile(const FString& Audience);  // "indie", "AAA", "mobile"

    // ========================================================================
    // PACKAGING
    // ========================================================================

    // Package the project
    FPackageResult PackageProject(const FPackagingProfile& Profile, const FString& OutputDir);

    // Cook content only (no packaging)
    FPackageResult CookContent(ETargetPlatform Platform, const TArray<FString>& Maps);

    // Package specific maps
    FPackageResult PackageMaps(const TArray<FString>& Maps, const FPackagingProfile& Profile, const FString& OutputDir);

    // Create DLC package
    FPackageResult CreateDLC(const FString& DLCName, const TArray<FString>& ContentPaths, const FPackagingProfile& Profile);

    // ========================================================================
    // DEPLOYMENT
    // ========================================================================

    // Deploy to Steam
    FPackageResult DeployToSteam(const FDeploymentTarget& Target, const FString& BuildPath);

    // Deploy to Itch.io
    FPackageResult DeployToItch(const FDeploymentTarget& Target, const FString& BuildPath);

    // Deploy to Epic Games Store
    FPackageResult DeployToEpic(const FDeploymentTarget& Target, const FString& BuildPath);

    // General deployment
    FPackageResult Deploy(const FDeploymentTarget& Target, const FString& BuildPath);

    // ========================================================================
    // METADATA
    // ========================================================================

    // Generate store metadata from game content
    FStoreMetadata GenerateStoreMetadata(const FString& ProjectPath);

    // Export metadata to platform format
    FString ExportMetadataForSteam(const FStoreMetadata& Metadata);
    FString ExportMetadataForItch(const FStoreMetadata& Metadata);
    FString ExportMetadataForEpic(const FStoreMetadata& Metadata);

    // ========================================================================
    // VERSION MANAGEMENT
    // ========================================================================

    // Get current version
    FVersionInfo GetCurrentVersion();

    // Bump version
    void BumpVersion(bool bMajor = false, bool bMinor = false, bool bPatch = true);

    // Set version
    void SetVersion(const FVersionInfo& Version);

    // Generate changelog
    FString GenerateChangelog(const FVersionInfo& FromVersion, const FVersionInfo& ToVersion);

    // ========================================================================
    // UTILITIES
    // ========================================================================

    // Estimate package size
    int64 EstimatePackageSize(const FPackagingProfile& Profile);

    // Validate project for packaging
    TArray<FString> ValidateForPackaging(ETargetPlatform Platform);

    // Get UAT command for packaging
    FString GetUATCommand(const FPackagingProfile& Profile, const FString& OutputDir);

    // Get required SDKs for platform
    TArray<FString> GetRequiredSDKs(ETargetPlatform Platform);

    // Check if platform SDK is installed
    bool IsPlatformSDKInstalled(ETargetPlatform Platform);

    // Get platform name string
    static FString GetPlatformName(ETargetPlatform Platform);

private:
    FPackagingSystem() = default;

    FVersionInfo CurrentVersion;
};
