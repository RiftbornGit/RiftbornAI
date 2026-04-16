// Copyright RiftbornAI. All Rights Reserved.
// Asset Generation Tools — AI-powered 3D model and texture download + import pipeline

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Asset Generation Tools Module
 * Downloads 3D models and textures from external URLs (AI generators, asset libraries)
 * and imports them into the UE5 content browser in a single tool call.
 *
 * Tools:
 * - generate_and_import_3d_model: Download + import a 3D model (GLB/FBX/OBJ)
 * - generate_texture: Download + import a texture (PNG/JPG/TGA/EXR)
 * - batch_download_and_import: Batch download + import from JSON URL list
 */
class RIFTBORNAI_API FAssetGenerationToolsModule : public TToolModuleBase<FAssetGenerationToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("AssetGeneration"); }
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    /** Download a 3D model from URL and import into project */
    static FClaudeToolResult Tool_GenerateAndImport3DModel(const FClaudeToolCall& Call);

    /** Download a texture from URL and import into project */
    static FClaudeToolResult Tool_GenerateTexture(const FClaudeToolCall& Call);

    /** Batch download and import multiple assets from JSON array */
    static FClaudeToolResult Tool_BatchDownloadAndImport(const FClaudeToolCall& Call);

private:
    /** Shared HTTP download logic — blocks game thread until complete or timeout */
    static bool DownloadFile(const FString& Url, const FString& LocalPath, int32 TimeoutSeconds, FString& OutError, int64& OutFileSize);

    /** Derive a sanitized asset name from a URL */
    static FString DeriveAssetNameFromUrl(const FString& Url);

    /** Ensure the download directory exists */
    static FString GetDownloadDir();
};
