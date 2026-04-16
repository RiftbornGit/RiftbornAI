// AssetPipeline.h - Asset Creation and Management from Natural Language
// Creates meshes, textures, materials, and other assets from descriptions

#pragma once

#include "CoreMinimal.h"
#include "AssetPipeline.generated.h"

// ============================================================================
// ASSET TYPES
// ============================================================================

UENUM(BlueprintType)
enum class EAssetCategory : uint8
{
    StaticMesh,
    SkeletalMesh,
    Texture,
    Material,
    MaterialInstance,
    ParticleSystem,
    Sound,
    Animation,
    Blueprint,
    DataAsset,
    Level,
    Widget
};

UENUM(BlueprintType)
enum class EMeshPrimitive : uint8
{
    Cube,
    Sphere,
    Cylinder,
    Cone,
    Plane,
    Capsule,
    Torus,
    Custom
};

UENUM(BlueprintType)
enum class EAPipeTextureType : uint8
{
    Diffuse,
    Normal,
    Roughness,
    Metallic,
    AO,
    Emissive,
    Height,
    Mask
};

UENUM(BlueprintType)
enum class EAPipeMaterialDomain : uint8
{
    Surface,
    PostProcess,
    LightFunction,
    Volume,
    UI,
    DeferredDecal
};

// ============================================================================
// ASSET SPECIFICATION STRUCTURES
// ============================================================================

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FMeshSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EMeshPrimitive BasePrimitive = EMeshPrimitive::Cube;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector Dimensions = FVector(100.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 Subdivisions = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bGenerateCollision = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bGenerateLODs = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 LODCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString MaterialSlot;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FTextureSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EAPipeTextureType TextureType = EAPipeTextureType::Diffuse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 Width = 1024;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 Height = 1024;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FLinearColor BaseColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString PatternType;  // "noise", "gradient", "checker", "solid"

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float NoiseScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bSRGB = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bGenerateMips = true;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FMaterialSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EAPipeMaterialDomain Domain = EAPipeMaterialDomain::Surface;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FLinearColor BaseColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Metallic = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Roughness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Specular = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FLinearColor EmissiveColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float EmissiveStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Opacity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bTwoSided = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bTranslucent = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TMap<EAPipeTextureType, FString> TextureSlots;  // Texture paths per slot
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAssetImportSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString SourcePath;  // External file path

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString DestinationPath;  // UE content path

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EAssetCategory AssetType = EAssetCategory::StaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bOverwriteExisting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TMap<FString, FString> ImportOptions;
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAPipeAssetResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bSuccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EAssetCategory AssetType = EAssetCategory::StaticMesh;
};

// ============================================================================
// ASSET PIPELINE
// ============================================================================

class RIFTBORNAI_API FAssetPipeline
{
public:
    static FAssetPipeline& Get()
    {
        static FAssetPipeline Instance;
        return Instance;
    }

    // ========================================================================
    // MESH CREATION
    // ========================================================================

    // Create mesh from description: "a large stone pillar 3 meters tall"
    FMeshSpec ParseMeshDescription(const FString& Description);

    // Generate procedural mesh from spec
    FAPipeAssetResult CreateProceduralMesh(const FMeshSpec& Spec, const FString& DestPath);

    // Combine multiple meshes into one
    FAPipeAssetResult CombineMeshes(const TArray<FString>& MeshPaths, const FString& DestPath);

    // ========================================================================
    // TEXTURE CREATION
    // ========================================================================

    // Create texture from description: "rough stone texture with moss"
    FTextureSpec ParseTextureDescription(const FString& Description);

    // Generate procedural texture from spec
    FAPipeAssetResult CreateProceduralTexture(const FTextureSpec& Spec, const FString& DestPath);

    // Create normal map from height map
    FAPipeAssetResult GenerateNormalMap(const FString& HeightMapPath, const FString& DestPath, float Strength = 1.0f);

    // ========================================================================
    // MATERIAL CREATION
    // ========================================================================

    // Create material from description: "shiny metal with scratches"
    FMaterialSpec ParseMaterialDescription(const FString& Description);

    // Generate material from spec
    FAPipeAssetResult CreateMaterial(const FMaterialSpec& Spec, const FString& DestPath);

    // Create material instance with parameter overrides
    FAPipeAssetResult CreateMaterialInstance(const FString& ParentMaterial, const TMap<FString, float>& ScalarParams,
        const TMap<FString, FLinearColor>& VectorParams, const FString& DestPath);

    // ========================================================================
    // ASSET IMPORT
    // ========================================================================

    // Import external asset (FBX, PNG, WAV, etc.)
    FAPipeAssetResult ImportAsset(const FAssetImportSpec& Spec);

    // Batch import multiple assets
    TArray<FAPipeAssetResult> BatchImport(const TArray<FAssetImportSpec>& Specs);

    // ========================================================================
    // ASSET MANAGEMENT
    // ========================================================================

    // Find assets by pattern
    TArray<FString> FindAssets(const FString& SearchPattern, EAssetCategory Category);

    // Duplicate asset
    FAPipeAssetResult DuplicateAsset(const FString& SourcePath, const FString& DestPath);

    // Rename asset
    bool RenameAsset(const FString& OldPath, const FString& NewName);

    // Delete asset
    bool DeleteAsset(const FString& AssetPath);

    // Check if asset exists
    bool AssetExists(const FString& AssetPath);

    // Get asset type from path
    EAssetCategory GetAssetType(const FString& AssetPath);

    // ========================================================================
    // UTILITY
    // ========================================================================

    // Generate unique asset name
    FString GenerateUniqueAssetName(const FString& BaseName, const FString& Directory);

    // Validate asset path
    bool ValidateAssetPath(const FString& Path);

    // Get default content directory for asset type
    FString GetDefaultDirectory(EAssetCategory Category);

private:
    FAssetPipeline() = default;

    // Internal helpers
    EMeshPrimitive ParsePrimitiveFromText(const FString& Text);
    FLinearColor ParseColorFromText(const FString& Text);
    float ParseSizeFromText(const FString& Text);
};
