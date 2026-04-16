// RiftbornBlueprintTool.h
// Blueprint-authored agent tools — the community-extension surface.
//
// Subclass URiftbornBlueprintTool in a Blueprint, override Execute in the
// graph editor, call RiftbornAI.ReloadBlueprintTools, and the agent picks
// up the new tool without a C++ recompile. That's the hot-reload story.
//
// Contract:
//   * Name must be globally unique among tools. A name collision with a
//     built-in tool will abort the registration with a log warning.
//   * Parameters are a flat key→type map; JSON-nested args go through
//     "args_json" convention (same as FPlanStep).
//   * Execute returns a success flag + result string. The scanner wraps the
//     call in an FClaudeToolFunction so the standard governance pipeline
//     (risk gating, trust scoring, proof-mode witnesses) applies uniformly.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "RiftbornBlueprintTool.generated.h"

UENUM(BlueprintType)
enum class ERiftbornBPToolParamType : uint8
{
	String,
	Integer,
	Number,
	Boolean,
};

USTRUCT(BlueprintType)
struct RIFTBORNAI_API FRiftbornBPToolParam
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	ERiftbornBPToolParamType Type = ERiftbornBPToolParamType::String;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	bool bRequired = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FString DefaultValue;
};

/**
 * Blueprint-implementable agent tool. Subclass in Blueprints and override
 * Execute to produce a working tool the RiftbornAI agent can pick.
 */
UCLASS(Blueprintable, Abstract, EditInlineNew, CollapseCategories,
	meta = (DisplayName = "Riftborn Blueprint Tool"))
class RIFTBORNAI_API URiftbornBlueprintTool : public UObject
{
	GENERATED_BODY()

public:
	/** Tool name (globally unique). Convention: lowercase snake_case. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Metadata")
	FString ToolName;

	/** Short description shown to the LLM. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Metadata")
	FString Description;

	/** Category string — used for UI grouping. Default "Blueprint". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Metadata")
	FString Category = TEXT("Blueprint");

	/** True if the tool mutates world state. Drives risk gating and the
	 *  default vision-verify recommendation set in ToolGovernance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Metadata")
	bool bMutatesWorld = false;

	/** Parameter schema. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Parameters")
	TArray<FRiftbornBPToolParam> Parameters;

	/**
	 * Execute the tool. Arguments are keyed by FRiftbornBPToolParam::Name.
	 * Override in Blueprints. Return true for success.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tool")
	bool Execute(const TMap<FString, FString>& Arguments, FString& ResultText, FString& ErrorText);
	virtual bool Execute_Implementation(const TMap<FString, FString>& Arguments, FString& ResultText, FString& ErrorText);
};

/**
 * Scans the asset registry for URiftbornBlueprintTool subclasses and
 * registers each with FClaudeToolRegistry. Idempotent — re-scanning
 * refreshes existing registrations.
 */
class RIFTBORNAI_API FRiftbornBlueprintToolScanner
{
public:
	/** Scan the asset registry and register every concrete URiftbornBlueprintTool.
	 *  Returns the number of tools registered (new or refreshed). */
	static int32 ScanAndRegister();

	/** Count of BP tools currently registered. */
	static int32 GetRegisteredCount();

private:
	static TSet<FString> RegisteredBPToolNames;
};
