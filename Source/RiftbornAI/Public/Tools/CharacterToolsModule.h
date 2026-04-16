// CharacterToolsModule.h
// Character and mannequin asset tools for RiftbornAI
// Tools: use_manny_mesh, get_engine_mannequins, set_character_mesh

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * Character Tools Module
 * 
 * Provides tools for working with character meshes, especially UE5 engine mannequins:
 * 
 * - use_manny_mesh: Apply UE5 Manny skeletal mesh to a character
 * - get_engine_mannequins: List available mannequin asset families and paths
 * - set_character_mesh: Set any skeletal mesh on a character
 * - copy_engine_mannequin_to_project: Duplicate primary Manny/Quinn assets into the project for customization
 * - set_playable_character_input_assets: Point a playable character Blueprint at specific Enhanced Input assets
 *
 * The Manny and Quinn asset families in MoverExamples content include meshes,
 * animation blueprints, materials, and shared rig assets that can be copied
 * into the project or assigned directly by path.
 */
class RIFTBORNAI_API FCharacterToolsModule : public TToolModuleBase<FCharacterToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("CharacterTools"); }
    
    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;
    
    // Tool implementations
    static FClaudeToolResult Tool_UseMannyMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetEngineMannequins(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetCharacterMesh(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_CopyEngineMannequinToProject(const FClaudeToolCall& Call);
    
    // NEW: Third Person Character tools
    static FClaudeToolResult Tool_CreateCharacterFromThirdPerson(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetThirdPersonTemplatePaths(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SpawnThirdPersonCharacter(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetPlayableCharacterInputAssets(const FClaudeToolCall& Call);
    
    // Mannequin asset paths (from engine)
    struct FMannequinPaths
    {
        // Skeletal Meshes - Manny (Male)
        static constexpr const TCHAR* SKM_Manny = TEXT("/MoverExamples/Characters/Mannequins/Meshes/SKM_Manny_Simple");
        static constexpr const TCHAR* SK_Mannequin = TEXT("/MoverExamples/Characters/Mannequins/Meshes/SK_Mannequin");
        
        // Skeletal Meshes - Quinn (Female)
        static constexpr const TCHAR* SKM_Quinn = TEXT("/MoverExamples/Characters/Mannequins/Meshes/SKM_Quinn_Simple");
        static constexpr const TCHAR* SK_Quinn = TEXT("/MoverExamples/Characters/Mannequins/Meshes/SK_Quinn");
        
        // Animation Blueprints
        static constexpr const TCHAR* ABP_Manny = TEXT("/MoverExamples/Characters/Mannequins/Animations/ABP_Manny");
        static constexpr const TCHAR* ABP_MannyExtended = TEXT("/MoverExamples/Characters/Mannequins/Animations/ABP_MannyExtended");
        static constexpr const TCHAR* ABP_Quinn = TEXT("/MoverExamples/Characters/Mannequins/Animations/ABP_Quinn");
        
        // Materials - Manny
        static constexpr const TCHAR* MI_Manny_01 = TEXT("/MoverExamples/Characters/Mannequins/Materials/MI_Manny_01");
        static constexpr const TCHAR* MI_Manny_02 = TEXT("/MoverExamples/Characters/Mannequins/Materials/MI_Manny_02");
        
        // Materials - Quinn
        static constexpr const TCHAR* MI_Quinn_01 = TEXT("/MoverExamples/Characters/Mannequins/Materials/MI_Quinn_01");
        static constexpr const TCHAR* MI_Quinn_02 = TEXT("/MoverExamples/Characters/Mannequins/Materials/MI_Quinn_02");
        
        // Key Animations
        static constexpr const TCHAR* MM_Idle = TEXT("/MoverExamples/Characters/Mannequins/Animations/MM_Idle");
        static constexpr const TCHAR* MM_Run_Fwd = TEXT("/MoverExamples/Characters/Mannequins/Animations/MM_Run_Fwd");
        static constexpr const TCHAR* MM_Walk_Fwd = TEXT("/MoverExamples/Characters/Mannequins/Animations/MM_Walk_Fwd");
        static constexpr const TCHAR* MM_Jump = TEXT("/MoverExamples/Characters/Mannequins/Animations/MM_Jump");
        static constexpr const TCHAR* MM_Fall_Loop = TEXT("/MoverExamples/Characters/Mannequins/Animations/MM_Fall_Loop");
        static constexpr const TCHAR* MM_Land = TEXT("/MoverExamples/Characters/Mannequins/Animations/MM_Land");
        
        // Blend Spaces
        static constexpr const TCHAR* BS_WalkRun = TEXT("/MoverExamples/Characters/Mannequins/Animations/BS_MM_WalkRun");
        static constexpr const TCHAR* BS_WalkRunStrafe = TEXT("/MoverExamples/Characters/Mannequins/Animations/BS_MM_WalkRunStrafe");
        
        // IK Rig
        static constexpr const TCHAR* IK_Mannequin = TEXT("/MoverExamples/Characters/Mannequins/Rig/IK_Mannequin");
        
        // Control Rig
        static constexpr const TCHAR* CR_Mannequin_Body = TEXT("/MoverExamples/Characters/Mannequins/Rig/CR_Mannequin_Body");
        
        // Physics Asset
        static constexpr const TCHAR* PA_Mannequin = TEXT("/MoverExamples/Characters/Mannequins/Rig/PA_Mannequin");
    };
    
    // Third Person Character template paths (from engine Content)
    struct FThirdPersonPaths
    {
        // The main Third Person Character Blueprint
        static constexpr const TCHAR* BP_ThirdPersonCharacter = TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter");
        
        // Alternative paths where Third Person template may exist
        static constexpr const TCHAR* BP_ThirdPersonCharacter_Engine = TEXT("/Engine/ThirdPerson/Blueprints/BP_ThirdPersonCharacter");
        static constexpr const TCHAR* BP_ThirdPersonCharacter_Template = TEXT("/Game/ThirdPersonBP/Blueprints/ThirdPersonCharacter");
        
        // Third Person skeletal mesh (uses same mannequin)
        static constexpr const TCHAR* SK_ThirdPerson = TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny");
        
        // Third Person Animation Blueprint
        static constexpr const TCHAR* ABP_ThirdPerson = TEXT("/Game/Characters/Mannequins/Animations/ABP_Manny");
        
        // Input Mapping Contexts
        static constexpr const TCHAR* IMC_Default = TEXT("/Game/ThirdPerson/Input/IMC_Default");
        static constexpr const TCHAR* IA_Move = TEXT("/Game/ThirdPerson/Input/Actions/IA_Move");
        static constexpr const TCHAR* IA_Look = TEXT("/Game/ThirdPerson/Input/Actions/IA_Look");
        static constexpr const TCHAR* IA_Jump = TEXT("/Game/ThirdPerson/Input/Actions/IA_Jump");
        
        // GameMode
        static constexpr const TCHAR* BP_ThirdPersonGameMode = TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode");
    };
    
    // Result of ResolveMannequinAssets — everything the tools need to dress a Character.
    // Any of the pointer fields may be null if nothing suitable was discovered; callers
    // must check before using.
    struct FResolvedMannequinAssets
    {
        class USkeletalMesh* MeshAsset = nullptr;
        class UClass* AnimInstanceClass = nullptr;
        FString MeshObjectPath;
        FString AnimBlueprintObjectPath;
        FString DiscoverySource; // e.g. "project", "mover_examples", "asset_registry"
        bool bMeshFound = false;
        bool bAnimFound = false;
    };

    // Discover the best-available Manny or Quinn skeletal mesh and animation blueprint
    // for this project. Tries hardcoded /MoverExamples/ paths first for back-compat,
    // then falls back to an Asset Registry search for meshes/anim blueprints whose
    // names match the mannequin naming conventions. Prefers /Game/ paths.
    static FResolvedMannequinAssets ResolveMannequinAssets(bool bUseQuinn);

private:
    // Helper to find skeletal mesh component on character
    static class USkeletalMeshComponent* FindSkeletalMeshComponent(class AActor* Actor);

    // Helper to set mesh with animation blueprint
    static bool SetMeshWithAnimBP(class USkeletalMeshComponent* MeshComp, const FString& MeshPath, const FString& AnimBPPath);

    // Helper to validate engine plugin is available
    static bool IsMoverExamplesPluginAvailable();
    static bool AreMoverExamplesAssetsAvailable();
};
