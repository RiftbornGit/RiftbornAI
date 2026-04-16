// Copyright RiftbornAI. All Rights Reserved.
// Niagara Tools Module - VFX/Particle system creation and manipulation

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Niagara Tools Module
 * Provides tools for creating and spawning Niagara particle systems.
 *
 * Tools included:
 * - create_niagara_system: Create a new Niagara system asset
 * - spawn_niagara_at_location: Spawn a Niagara system at a world location
 * - spawn_niagara_attached: Spawn a Niagara system attached to an actor
 * - list_niagara_systems: List available Niagara systems
 * - create_niagara_from_template: Create from built-in templates
 * - spawn_forest_air_particles: Spawn a reusable ambient forest particle zone
 * - edit_forest_air_particles: Retune an existing ambient forest particle zone
 * - set_niagara_parameter: Set parameter on spawned component
 * - preview_niagara: Preview a system in the viewport
 * - get_niagara_emitters: List emitters in a system
 * - get_niagara_parameters: List user parameters
 * - activate_niagara: Activate/deactivate a component
 * - reset_niagara: Reset a component to initial state
 *
 * Phase 1 Observability Tools:
 * - inspect_niagara_system: Deep introspection (modules, params, renderers)
 * - diff_niagara_systems: Compare two systems
 * - save_niagara_snapshot: Save snapshot to JSON file
 *
 * Phase 2 Authoring Tools (NiagaraToolsModule_Authoring.cpp):
 * - add_niagara_emitter: Add an emitter to a system from a template
 * - configure_niagara_emitter: Set emitter-level parameters (spawn rate, lifetime, velocity, etc.)
 * - set_niagara_renderer: Configure renderer on an emitter (sprite/mesh/ribbon)
 */
class RIFTBORNAI_API FNiagaraToolsModule : public TToolModuleBase<FNiagaraToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("NiagaraTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    // Tool implementations
    static FClaudeToolResult Tool_CreateNiagaraSystem(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SpawnNiagaraAtLocation(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SpawnNiagaraAttached(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ListNiagaraSystems(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CreateNiagaraFromTemplate(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SpawnForestAirParticles(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EditForestAirParticles(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetNiagaraParameter(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_PreviewNiagara(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetNiagaraEmitters(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetNiagaraParameters(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ActivateNiagara(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ResetNiagara(const FClaudeToolCall& Call);

    // Phase 1 Observability Tools
    static FClaudeToolResult Tool_InspectNiagaraSystem(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_DiffNiagaraSystems(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SaveNiagaraSnapshot(const FClaudeToolCall& Call);

    // Phase 2 Authoring Tools (NiagaraToolsModule_Authoring.cpp)
    static FClaudeToolResult Tool_AddNiagaraEmitter(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_ConfigureNiagaraEmitter(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetNiagaraRenderer(const FClaudeToolCall& Call);
};
