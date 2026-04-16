// Copyright RiftbornAI. All Rights Reserved.
// Thin world-rule tools built on top of UE 5.7 World Conditions.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "GameplayTagContainer.h"
#include "Tools/ToolModuleBase.h"
#include "WorldConditionBase.h"
#include "WorldConditionSchema.h"
#include "WorldRuleToolsModule.generated.h"

UCLASS()
/**
 * URiftbornWorldRuleSchema
 * 
 * Thin world-rule tools built on top of UE 5.7 World Conditions.
 */
class RIFTBORNAI_API URiftbornWorldRuleSchema : public UWorldConditionSchema
{
    GENERATED_BODY()

public:
    explicit URiftbornWorldRuleSchema(const FObjectInitializer& ObjectInitializer);

    virtual bool IsStructAllowed(const UScriptStruct* InScriptStruct) const override;

    FWorldConditionContextDataRef GetActorRef() const { return ActorRef; }
    FWorldConditionContextDataRef GetTargetLocationRef() const { return TargetLocationRef; }

private:
    FWorldConditionContextDataRef ActorRef;
    FWorldConditionContextDataRef TargetLocationRef;
};

USTRUCT(meta=(Hidden))
struct RIFTBORNAI_API FRiftbornActorGameplayTagWorldCondition : public FWorldConditionBase
{
    GENERATED_BODY()

    virtual TObjectPtr<const UStruct>* GetRuntimeStateType() const override { return nullptr; }
    virtual bool Initialize(const UWorldConditionSchema& Schema) override;
    virtual FWorldConditionResult IsTrue(const FWorldConditionContext& Context) const override;
    virtual FText GetDescription() const override;

    UPROPERTY(EditAnywhere, Category="WorldRule")
    FGameplayTag RequiredTag;

protected:
    FWorldConditionContextDataRef ActorRef;
    bool bCanCacheResult = true;
};

USTRUCT(meta=(Hidden))
struct RIFTBORNAI_API FRiftbornActorDistanceWorldCondition : public FWorldConditionBase
{
    GENERATED_BODY()

    virtual TObjectPtr<const UStruct>* GetRuntimeStateType() const override { return nullptr; }
    virtual bool Initialize(const UWorldConditionSchema& Schema) override;
    virtual FWorldConditionResult IsTrue(const FWorldConditionContext& Context) const override;
    virtual FText GetDescription() const override;

    UPROPERTY(EditAnywhere, Category="WorldRule")
    double MaxDistance = 0.0;

protected:
    FWorldConditionContextDataRef ActorRef;
    FWorldConditionContextDataRef TargetLocationRef;
    bool bCanCacheResult = false;
};

class RIFTBORNAI_API FWorldRuleToolsModule : public TToolModuleBase<FWorldRuleToolsModule>
{
public:
    static FString StaticModuleName() { return TEXT("WorldRuleTools"); }

    virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

    static FClaudeToolResult Tool_GetWorldRuleSchemaInfo(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EvaluateActorGameplayTagWorldRule(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EvaluateActorDistanceWorldRule(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_EvaluateWorldRuleBundle(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetWorldRuleBundleInfo(const FClaudeToolCall& Call);
};
