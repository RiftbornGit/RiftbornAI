// MaterialExpressionHelpers.h
// Shared utilities for material expression creation, connection, and management.
// Used by MaterialToolsModule and MaterialGraphToolsModule.

#pragma once

#include "CoreMinimal.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Factories/MaterialFactoryNew.h"
// NOTE: do NOT include "AssetToolsModule.h" here — this plugin has a
// local Public/Tools/AssetToolsModule.h that shadows the engine header
// when reached from a Public include. The persistent-color helper that
// needs FAssetToolsModule is implemented out-of-line in MaterialToolsModule.cpp
// where the .cpp's include search picks UE's engine header correctly.
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "EditorAssetLibrary.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"
#include "Misc/PackageName.h"

namespace MaterialHelpers
{

// ============================================================================
// Load & Validate
// ============================================================================

inline FString NormalizeAssetObjectPath(const FString& AssetPath)
{
    FString Normalized = AssetPath.TrimStartAndEnd();
    if (!Normalized.Contains(TEXT(".")) && Normalized.StartsWith(TEXT("/")))
    {
        const FString AssetName = FPackageName::GetLongPackageAssetName(Normalized);
        if (!AssetName.IsEmpty())
        {
            Normalized = Normalized + TEXT(".") + AssetName;
        }
    }
    return Normalized;
}

/** Load a UMaterial for editing. Returns nullptr and sets OutError on failure. */
inline UMaterial* LoadMaterialForEditing(const FString& MaterialPath, FString& OutError)
{
    if (MaterialPath.IsEmpty())
    {
        OutError = TEXT("Material path is empty");
        return nullptr;
    }

    if (MaterialPath.Contains(TEXT("..")) || MaterialPath.Contains(TEXT("\\")))
    {
        OutError = TEXT("Invalid path: path traversal detected");
        return nullptr;
    }

    const FString NormalizedPath = NormalizeAssetObjectPath(MaterialPath);
    UMaterial* Material = LoadObject<UMaterial>(nullptr, *NormalizedPath);
    if (!Material)
    {
        OutError = FString::Printf(TEXT("Material not found (must be a UMaterial, not an instance): %s"), *MaterialPath);
        return nullptr;
    }

    return Material;
}

inline UMaterialInterface* LoadMaterialInterfaceForEditing(const FString& MaterialPath, FString& OutError)
{
    if (MaterialPath.IsEmpty())
    {
        OutError = TEXT("Material path is empty");
        return nullptr;
    }

    if (MaterialPath.Contains(TEXT("..")) || MaterialPath.Contains(TEXT("\\")))
    {
        OutError = TEXT("Invalid path: path traversal detected");
        return nullptr;
    }

    const FString NormalizedPath = NormalizeAssetObjectPath(MaterialPath);
    UMaterialInterface* MaterialInterface = LoadObject<UMaterialInterface>(nullptr, *NormalizedPath);
    if (!MaterialInterface)
    {
        OutError = FString::Printf(TEXT("Material interface not found: %s"), *MaterialPath);
        return nullptr;
    }

    return MaterialInterface;
}

inline UMaterialInstanceConstant* LoadMaterialInstanceConstantForEditing(const FString& MaterialPath, FString& OutError)
{
    UMaterialInterface* MaterialInterface = LoadMaterialInterfaceForEditing(MaterialPath, OutError);
    if (!MaterialInterface)
    {
        return nullptr;
    }

    UMaterialInstanceConstant* MaterialInstance = Cast<UMaterialInstanceConstant>(MaterialInterface);
    if (!MaterialInstance)
    {
        OutError = FString::Printf(TEXT("Material instance constant not found: %s"), *MaterialPath);
        return nullptr;
    }

    return MaterialInstance;
}

// ============================================================================
// Expression Creation
// ============================================================================

/** Create a material expression of type T, add it to the material, and position it in the editor. */
template<typename T>
T* CreateExpression(UMaterial* Material, int32 EditorX = -400, int32 EditorY = 0)
{
    check(Material);
    T* Expr = NewObject<T>(Material);
    Expr->MaterialExpressionEditorX = EditorX;
    Expr->MaterialExpressionEditorY = EditorY;
    Material->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(Expr);
    return Expr;
}

/** Get the index of an expression in the material's expression collection. Returns -1 if not found. */
inline int32 GetExpressionIndex(UMaterial* Material, UMaterialExpression* Expr)
{
    return Material->GetEditorOnlyData()->ExpressionCollection.Expressions.IndexOfByKey(Expr);
}

/** Find an expression by node_id string (e.g. "node_5"). Returns nullptr if invalid. */
inline UMaterialExpression* FindExpressionByNodeId(UMaterial* Material, const FString& NodeId, FString& OutError)
{
    if (!NodeId.StartsWith(TEXT("node_")))
    {
        OutError = FString::Printf(TEXT("Invalid node_id format: '%s' (expected 'node_N')"), *NodeId);
        return nullptr;
    }

    int32 Index = FCString::Atoi(*NodeId.Mid(5));
    auto& Expressions = Material->GetEditorOnlyData()->ExpressionCollection.Expressions;

    if (Index < 0 || Index >= Expressions.Num())
    {
        OutError = FString::Printf(TEXT("Node '%s' out of range (material has %d nodes)"), *NodeId, Expressions.Num());
        return nullptr;
    }

    UMaterialExpression* Expr = Expressions[Index];
    if (!Expr)
    {
        OutError = FString::Printf(TEXT("Node '%s' is null"), *NodeId);
        return nullptr;
    }

    return Expr;
}

/** Generate a node_id string for an expression in a material. */
inline FString MakeNodeId(UMaterial* Material, UMaterialExpression* Expr)
{
    int32 Index = GetExpressionIndex(Material, Expr);
    return FString::Printf(TEXT("node_%d"), Index);
}

// ============================================================================
// Material Pin Connection
// ============================================================================

/**
 * Connect an expression to a named material input pin.
 * Supports all 21 UE 5.7 material pins: BaseColor, Normal, Metallic, Roughness, Specular,
 * EmissiveColor, Opacity, OpacityMask, AmbientOcclusion, WorldPositionOffset, SubsurfaceColor,
 * Refraction, MaterialAttributes, PixelDepthOffset, Displacement, Tangent, Anisotropy,
 * ClearCoat, ClearCoatRoughness, SurfaceThickness, FrontMaterial (Substrate).
 * Returns false and sets OutError on failure.
 */
inline bool ConnectToMaterialPin(UMaterial* Material, UMaterialExpression* Expr, const FString& PinName, FString& OutError, int32 OutputIndex = 0)
{
    check(Material && Expr);
    auto* EditorData = Material->GetEditorOnlyData();

    // Helper macro to reduce boilerplate
    #define CONNECT_PIN(Name, Member) \
        if (PinName.Equals(TEXT(#Name), ESearchCase::IgnoreCase)) \
        { \
            EditorData->Member.Expression = Expr; \
            EditorData->Member.OutputIndex = OutputIndex; \
            return true; \
        }

    CONNECT_PIN(BaseColor, BaseColor)
    CONNECT_PIN(Normal, Normal)
    CONNECT_PIN(Metallic, Metallic)
    CONNECT_PIN(Roughness, Roughness)
    CONNECT_PIN(Specular, Specular)
    CONNECT_PIN(EmissiveColor, EmissiveColor)
    CONNECT_PIN(Opacity, Opacity)
    CONNECT_PIN(OpacityMask, OpacityMask)
    CONNECT_PIN(AmbientOcclusion, AmbientOcclusion)
    CONNECT_PIN(WorldPositionOffset, WorldPositionOffset)
    CONNECT_PIN(SubsurfaceColor, SubsurfaceColor)
    CONNECT_PIN(Refraction, Refraction)
    CONNECT_PIN(MaterialAttributes, MaterialAttributes)
    CONNECT_PIN(PixelDepthOffset, PixelDepthOffset)
    CONNECT_PIN(Displacement, Displacement)
    CONNECT_PIN(Tangent, Tangent)
    CONNECT_PIN(Anisotropy, Anisotropy)
    CONNECT_PIN(ClearCoat, ClearCoat)
    CONNECT_PIN(ClearCoatRoughness, ClearCoatRoughness)
    CONNECT_PIN(SurfaceThickness, SurfaceThickness)
    // Substrate/Strata material model (UE 5.4+) — FrontMaterial pin
    CONNECT_PIN(FrontMaterial, FrontMaterial)

    #undef CONNECT_PIN

    OutError = FString::Printf(TEXT("Unknown material pin: '%s'"), *PinName);
    return false;
}

/** Disconnect a named material input pin. */
inline bool DisconnectMaterialPin(UMaterial* Material, const FString& PinName, FString& OutError)
{
    check(Material);
    auto* EditorData = Material->GetEditorOnlyData();

    #define DISCONNECT_PIN(Name, Member) \
        if (PinName.Equals(TEXT(#Name), ESearchCase::IgnoreCase)) \
        { \
            EditorData->Member.Expression = nullptr; \
            EditorData->Member.OutputIndex = 0; \
            return true; \
        }

    DISCONNECT_PIN(BaseColor, BaseColor)
    DISCONNECT_PIN(Normal, Normal)
    DISCONNECT_PIN(Metallic, Metallic)
    DISCONNECT_PIN(Roughness, Roughness)
    DISCONNECT_PIN(Specular, Specular)
    DISCONNECT_PIN(EmissiveColor, EmissiveColor)
    DISCONNECT_PIN(Opacity, Opacity)
    DISCONNECT_PIN(OpacityMask, OpacityMask)
    DISCONNECT_PIN(AmbientOcclusion, AmbientOcclusion)
    DISCONNECT_PIN(WorldPositionOffset, WorldPositionOffset)
    DISCONNECT_PIN(SubsurfaceColor, SubsurfaceColor)
    DISCONNECT_PIN(Refraction, Refraction)
    DISCONNECT_PIN(MaterialAttributes, MaterialAttributes)
    DISCONNECT_PIN(PixelDepthOffset, PixelDepthOffset)
    DISCONNECT_PIN(Displacement, Displacement)
    DISCONNECT_PIN(Tangent, Tangent)
    DISCONNECT_PIN(Anisotropy, Anisotropy)
    DISCONNECT_PIN(ClearCoat, ClearCoat)
    DISCONNECT_PIN(ClearCoatRoughness, ClearCoatRoughness)
    DISCONNECT_PIN(SurfaceThickness, SurfaceThickness)
    DISCONNECT_PIN(FrontMaterial, FrontMaterial)

    #undef DISCONNECT_PIN

    OutError = FString::Printf(TEXT("Unknown material pin: '%s'"), *PinName);
    return false;
}

// ============================================================================
// Color Parsing
// ============================================================================

/** Parse a color from name ("red", "blue") or RGB string ("0.5,0.2,0.8"). */
inline FLinearColor ParseColorNameOrRGB(const FString& ColorStr)
{
    FString Lower = ColorStr.ToLower();

    if (Lower == TEXT("red"))       return FLinearColor(1.0f, 0.0f, 0.0f);
    if (Lower == TEXT("green"))     return FLinearColor(0.0f, 1.0f, 0.0f);
    if (Lower == TEXT("blue"))      return FLinearColor(0.0f, 0.0f, 1.0f);
    if (Lower == TEXT("yellow"))    return FLinearColor(1.0f, 1.0f, 0.0f);
    if (Lower == TEXT("orange"))    return FLinearColor(1.0f, 0.5f, 0.0f);
    if (Lower == TEXT("purple"))    return FLinearColor(0.5f, 0.0f, 1.0f);
    if (Lower == TEXT("pink"))      return FLinearColor(1.0f, 0.4f, 0.7f);
    if (Lower == TEXT("cyan"))      return FLinearColor(0.0f, 1.0f, 1.0f);
    if (Lower == TEXT("magenta"))   return FLinearColor(1.0f, 0.0f, 1.0f);
    if (Lower == TEXT("white"))     return FLinearColor(1.0f, 1.0f, 1.0f);
    if (Lower == TEXT("black"))     return FLinearColor(0.0f, 0.0f, 0.0f);
    if (Lower == TEXT("gray") || Lower == TEXT("grey")) return FLinearColor(0.5f, 0.5f, 0.5f);
    if (Lower == TEXT("brown"))     return FLinearColor(0.6f, 0.3f, 0.0f);
    if (Lower == TEXT("gold"))      return FLinearColor(1.0f, 0.84f, 0.0f);
    if (Lower == TEXT("silver"))    return FLinearColor(0.75f, 0.75f, 0.75f);

    // Try R,G,B parse
    TArray<FString> Parts;
    ColorStr.ParseIntoArray(Parts, TEXT(","));
    if (Parts.Num() >= 3)
    {
        return FLinearColor(
            FCString::Atof(*Parts[0]),
            FCString::Atof(*Parts[1]),
            FCString::Atof(*Parts[2]),
            Parts.Num() >= 4 ? FCString::Atof(*Parts[3]) : 1.0f
        );
    }

    return FLinearColor(0.5f, 0.5f, 0.5f);
}

// ============================================================================
// Compile & Save
// ============================================================================

/** Trigger material recompilation and save to disk. */
inline void CompileAndSaveMaterial(UMaterial* Material)
{
    check(Material);
    Material->PreEditChange(nullptr);
    Material->PostEditChange();
    Material->MarkPackageDirty();

    UPackage* Package = Material->GetPackage();
    FString PackagePath = Package->GetName();
    FString PackageFilename = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    UPackage::SavePackage(Package, Material, *PackageFilename, SaveArgs);
}

// ============================================================================
// Shading Model / Blend Mode Mapping
// ============================================================================

/** Map a string name to EMaterialShadingModel. Returns true on success. */
inline bool ParseShadingModel(const FString& Name, EMaterialShadingModel& OutModel, FString& OutError)
{
    FString Lower = Name.ToLower();
    if (Lower == TEXT("default_lit"))           { OutModel = MSM_DefaultLit; return true; }
    if (Lower == TEXT("unlit"))                 { OutModel = MSM_Unlit; return true; }
    if (Lower == TEXT("subsurface"))            { OutModel = MSM_Subsurface; return true; }
    if (Lower == TEXT("preintegrated_skin"))    { OutModel = MSM_PreintegratedSkin; return true; }
    if (Lower == TEXT("clear_coat"))            { OutModel = MSM_ClearCoat; return true; }
    if (Lower == TEXT("subsurface_profile"))    { OutModel = MSM_SubsurfaceProfile; return true; }
    if (Lower == TEXT("two_sided_foliage"))     { OutModel = MSM_TwoSidedFoliage; return true; }
    if (Lower == TEXT("hair"))                  { OutModel = MSM_Hair; return true; }
    if (Lower == TEXT("cloth"))                 { OutModel = MSM_Cloth; return true; }
    if (Lower == TEXT("eye"))                   { OutModel = MSM_Eye; return true; }
    if (Lower == TEXT("single_layer_water"))    { OutModel = MSM_SingleLayerWater; return true; }
    if (Lower == TEXT("thin_translucent"))      { OutModel = MSM_ThinTranslucent; return true; }

    OutError = FString::Printf(TEXT("Unknown shading model: '%s'"), *Name);
    return false;
}

/** Map a string name to EBlendMode. Returns true on success. */
inline bool ParseBlendMode(const FString& Name, EBlendMode& OutMode, FString& OutError)
{
    FString Lower = Name.ToLower();
    if (Lower == TEXT("opaque"))            { OutMode = BLEND_Opaque; return true; }
    if (Lower == TEXT("masked"))            { OutMode = BLEND_Masked; return true; }
    if (Lower == TEXT("translucent"))       { OutMode = BLEND_Translucent; return true; }
    if (Lower == TEXT("additive"))          { OutMode = BLEND_Additive; return true; }
    if (Lower == TEXT("modulate"))          { OutMode = BLEND_Modulate; return true; }
    if (Lower == TEXT("alpha_composite"))   { OutMode = BLEND_AlphaComposite; return true; }
    if (Lower == TEXT("alpha_holdout"))     { OutMode = BLEND_AlphaHoldout; return true; }

    OutError = FString::Printf(TEXT("Unknown blend mode: '%s'"), *Name);
    return false;
}


// ============================================================================
// Expression Input Collection (UE 5.7+ compatible)
// ============================================================================

/**
 * Collect all expression inputs into an array.
 * Replaces the removed UMaterialExpression::GetInputs() API.
 * Uses GetInput(index) which returns nullptr when index is out of range.
 */
inline TArray<FExpressionInput*> GetExpressionInputs(UMaterialExpression* Expr)
{
    TArray<FExpressionInput*> Result;
    if (!Expr) return Result;

    for (int32 i = 0; ; ++i)
    {
        FExpressionInput* Input = Expr->GetInput(i);
        if (!Input) break;
        Result.Add(Input);
    }
    return Result;
}

inline FProperty* FindPropertyByLooseName(UStruct* OwnerStruct, const FString& PropertyName)
{
    if (!OwnerStruct || PropertyName.IsEmpty())
    {
        return nullptr;
    }

    if (FProperty* Exact = OwnerStruct->FindPropertyByName(FName(*PropertyName)))
    {
        return Exact;
    }

    const FString Normalized = PropertyName.Replace(TEXT(" "), TEXT("")).ToLower();
    for (TFieldIterator<FProperty> It(OwnerStruct); It; ++It)
    {
        FProperty* Property = *It;
        FString Candidate = Property->GetName();
        Candidate.ReplaceInline(TEXT(" "), TEXT(""));
        if (Candidate.ToLower() == Normalized)
        {
            return Property;
        }
    }

    return nullptr;
}

inline FString ExportPropertyValue(FProperty* Property, const void* Container, UObject* Owner)
{
    if (!Property || !Container)
    {
        return TEXT("");
    }

    FString ExportedValue;
    Property->ExportTextItem_Direct(
        ExportedValue,
        Property->ContainerPtrToValuePtr<void>(Container),
        nullptr,
        Owner,
        PPF_None);
    return ExportedValue;
}

inline bool ImportPropertyValue(FProperty* Property, void* ValuePtr, UObject* Owner, const FString& InputValue, FString& OutError)
{
    if (!Property || !ValuePtr)
    {
        OutError = TEXT("Property storage is invalid");
        return false;
    }

    if (Property->ImportText_Direct(*InputValue, ValuePtr, Owner, PPF_None) != nullptr)
    {
        return true;
    }

    OutError = FString::Printf(
        TEXT("Failed to parse value '%s' for property '%s' (%s)"),
        *InputValue,
        *Property->GetName(),
        *Property->GetCPPType());
    return false;
}

inline bool SaveLoadedAsset(UObject* Asset, FString& OutError)
{
    if (!Asset)
    {
        OutError = TEXT("Asset is null");
        return false;
    }

    Asset->MarkPackageDirty();
    if (UEditorAssetLibrary::SaveLoadedAsset(Asset))
    {
        return true;
    }

    UPackage* Package = Asset->GetOutermost();
    if (!Package)
    {
        OutError = FString::Printf(TEXT("Failed to save asset: %s (no package)"), *Asset->GetPathName());
        return false;
    }

    const FString PackageFilename = FPackageName::LongPackageNameToFilename(
        Package->GetName(),
        FPackageName::GetAssetPackageExtension());
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    if (UPackage::SavePackage(Package, Asset, *PackageFilename, SaveArgs))
    {
        return true;
    }

    OutError = FString::Printf(
        TEXT("Failed to save asset: %s (UEditorAssetLibrary::SaveLoadedAsset and UPackage::SavePackage both failed)"),
        *Asset->GetPathName());
    return false;
}

// ============================================================================
// Persistent Color Material (color -> MIC resolver for spawn_actor / set_actor_color)
// ============================================================================

/**
 * Resolve or create a PERSISTENT UMaterialInstanceConstant for the requested
 * color + roughness + metallic. The asset is saved as a real .uasset under
 * /Game/RiftbornGenerated/Colors/ so it survives PIE, level save, and packaging.
 *
 * First call in a project creates a shared master material M_RiftbornSolidColor
 * with named parameters (BaseColor, Roughness, Metallic). Subsequent calls
 * with any color reuse that master and only cost one lightweight MIC asset.
 *
 * Identical color+rough+metallic requests return the same MIC (deterministic
 * name from 8-bit quantized channels), so repeated spawns with the same color
 * don't flood the project with duplicate assets.
 *
 * MUST run on the game thread (LoadObject / package save).
 * Returns nullptr if asset creation/save fails; never returns a transient object.
 *
 * Implemented out-of-line in MaterialToolsModule.cpp because the body uses
 * FAssetToolsModule, and a local Public/Tools/AssetToolsModule.h shadows
 * the engine header when reached from this Public include path.
 */
RIFTBORNAI_API UMaterialInstanceConstant* ResolveOrCreatePersistentColorMaterial(
    const FLinearColor& Color, float Roughness, float Metallic);

} // namespace MaterialHelpers
