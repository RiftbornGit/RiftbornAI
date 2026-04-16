// Copyright RiftbornAI. All Rights Reserved.
// MetaSound Tools Module - UE5 MetaSounds audio graph creation and manipulation
//
// MetaSounds is UE5's node-based audio system replacing legacy SoundCues for procedural audio.
// Uses MetasoundFrontend API for graph manipulation.
//
// NOTE: Depends on MetasoundEngine and MetasoundFrontend modules.
// These are conditionally available - tools gracefully degrade if not present.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * MetaSound Tools Module
 *
 * Provides tools for UE5's MetaSounds procedural audio system:
 * - create_metasound: Create a new MetaSound asset from a template
 * - create_metasound_source: Create a MetaSound Source (playable in world)
 * - list_metasounds: List all MetaSound assets in the project
 * - get_metasound_info: Inspect MetaSound graph/document state, dependencies, interfaces, and preset metadata
 * - add_metasound_node: Add a node to a MetaSound graph
 * - connect_metasound_nodes: Connect two nodes in a MetaSound graph
 * - set_metasound_input: Set default value of a MetaSound input
 * - play_metasound_preview: Preview a MetaSound in the editor
 * - create_metasound_preset: Create a MetaSound preset from an existing MetaSound
 * - list_metasound_node_types: List available MetaSound node types for building graphs
 */
class RIFTBORNAI_API FMetaSoundToolsModule : public TToolModuleBase<FMetaSoundToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("MetaSoundTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_CreateMetaSound(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateMetaSoundSource(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListMetaSounds(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetMetaSoundInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_AddMetaSoundNode(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ConnectMetaSoundNodes(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetMetaSoundInput(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_PlayMetaSoundPreview(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateMetaSoundPreset(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListMetaSoundNodeTypes(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateSoundscapeMetaSoundSource(const FClaudeToolCall& Call);
};
