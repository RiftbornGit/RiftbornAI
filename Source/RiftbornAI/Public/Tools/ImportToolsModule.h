// Copyright RiftbornAI. All Rights Reserved.
// Import Tools Module - Asset import from external files

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Import Tools Module
 * Provides tools for importing external assets (FBX, textures, audio) into the project.
 * 
 * Tools included:
 * - import_fbx: Import FBX file as StaticMesh or SkeletalMesh
 * - import_texture: Import image file as Texture2D
 * - import_audio: Import audio file as SoundWave
 * - import_assets: Batch import multiple assets
 * - create_texture_from_color: Create a solid color texture
 */
class RIFTBORNAI_API FImportToolsModule : public TToolModuleBase<FImportToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("ImportTools"); }
    
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
    
    // Tool implementations
    static FClaudeToolResult Tool_ImportFBX(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ImportTexture(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ImportAudio(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ImportAssets(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateTextureFromColor(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetImportableFormats(const FClaudeToolCall& Call);
    
private:
    // Validate file path for security
    static bool IsPathSafe(const FString& Path, FString& OutError);
    
    // Get project content directory
    static FString GetContentDirectory();
};
