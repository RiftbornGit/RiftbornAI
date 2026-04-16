// Copyright RiftbornAI. All Rights Reserved.
// Blueprint to C++ Converter - Convert Blueprint logic to native C++ code

#pragma once

#include "CoreMinimal.h"
#include "BlueprintGraphAPI.h"
#include "BlueprintToCppConverter.generated.h"

/**
 * Conversion options for Blueprint to C++ translation
 */
USTRUCT(BlueprintType)
struct FBlueprintToCppOptions
{
	GENERATED_BODY()

	/** Output class name (empty = derive from Blueprint name) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
	FString ClassName;

	/** Output module name (for API macro) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
	FString ModuleName;

	/** Include full implementation or just declarations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
	bool bGenerateImplementation = true;

	/** Add comments explaining each conversion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
	bool bAddComments = true;

	/** Use modern C++ features (auto, lambdas, structured bindings) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
	bool bUseModernCpp = true;

	/** Generate UFUNCTION/UPROPERTY macros */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
	bool bGenerateReflection = true;

	/** Include original Blueprint node references as comments */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
	bool bIncludeNodeReferences = true;

	/** Optimize generated code (inline simple operations, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
	bool bOptimize = true;
};

/**
 * Result of Blueprint to C++ conversion
 */
USTRUCT(BlueprintType)
struct FBlueprintToCppResult
{
	GENERATED_BODY()

	/** Whether conversion succeeded */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	bool bSuccess = false;

	/** Error message if failed */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FString ErrorMessage;

	/** Generated header file content */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FString HeaderContent;

	/** Generated source file content */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FString SourceContent;

	/** List of required includes */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TArray<FString> RequiredIncludes;

	/** List of forward declarations needed */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TArray<FString> ForwardDeclarations;

	/** Conversion statistics */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	int32 NodesConverted = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	int32 FunctionsGenerated = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	int32 VariablesGenerated = 0;

	/** Warnings during conversion (non-fatal) */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TArray<FString> Warnings;

	/** Nodes that couldn't be converted (with reasons) */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TMap<FString, FString> UnsupportedNodes;
};

/**
 * Represents a converted function
 */
struct FConvertedFunction
{
	FString Name;
	FString ReturnType;
	TArray<TPair<FString, FString>> Parameters; // Type, Name
	TArray<FString> Body;
	bool bIsEvent = false;
	bool bIsConst = false;
	bool bIsOverride = false;
	FString Comment;
};

/**
 * Represents a converted variable
 */
struct FConvertedVariable
{
	FString Name;
	FString Type;
	FString DefaultValue;
	bool bIsExposed = false;
	bool bIsReplicated = false;
	FString Comment;
};

/**
 * Blueprint to C++ Converter
 * 
 * Converts Blueprint visual scripting logic into equivalent C++ code.
 * This is NOT a wrapper - it generates real, optimized, native C++ code
 * that can replace the Blueprint entirely for better performance.
 * 
 * Supports:
 * - Event handlers (BeginPlay, Tick, etc.)
 * - Function calls (native and Blueprint library)
 * - Variables (get/set)
 * - Control flow (Branch, Sequence, ForEach, etc.)
 * - Math operations
 * - Latent actions (Delay, etc.)
 * - Custom events and functions
 * 
 * Limitations:
 * - Some Blueprint-only nodes may require manual adjustment
 * - Timeline nodes need special handling
 * - Complex macro nodes may not convert directly
 */
UCLASS()
class RIFTBORNAI_API UBlueprintToCppConverter : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Convert a Blueprint to C++ code
	 * 
	 * @param Blueprint - Blueprint asset to convert
	 * @param Options - Conversion options
	 * @return Conversion result with generated code
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Conversion")
	static FBlueprintToCppResult ConvertBlueprint(UBlueprint* Blueprint, const FBlueprintToCppOptions& Options);

	/**
	 * Convert a specific graph from a Blueprint
	 * 
	 * @param Blueprint - Blueprint containing the graph
	 * @param GraphName - Name of graph to convert
	 * @param Options - Conversion options
	 * @return Conversion result
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Conversion")
	static FBlueprintToCppResult ConvertGraph(UBlueprint* Blueprint, const FString& GraphName, const FBlueprintToCppOptions& Options);

	/**
	 * Preview conversion without generating files
	 * Returns a summary of what would be converted
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Conversion")
	static FString PreviewConversion(UBlueprint* Blueprint);

	/**
	 * Check if a Blueprint can be fully converted
	 * Returns true if all nodes are supported
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Conversion")
	static bool CanFullyConvert(UBlueprint* Blueprint, TArray<FString>& OutUnsupportedNodes);

private:
	// =====================================================
	// Core Conversion Pipeline
	// =====================================================

	/** Analyze Blueprint structure and plan conversion */
	static void AnalyzeBlueprint(UBlueprint* Blueprint, FBlueprintToCppResult& Result);

	/** Extract and convert all variables */
	static TArray<FConvertedVariable> ConvertVariables(UBlueprint* Blueprint, FBlueprintToCppResult& Result);

	/** Extract and convert all functions */
	static TArray<FConvertedFunction> ConvertFunctions(UBlueprint* Blueprint, const FBlueprintToCppOptions& Options, FBlueprintToCppResult& Result);

	/** Convert a single graph to a function */
	static FConvertedFunction ConvertGraphToFunction(UBlueprint* Blueprint, UEdGraph* Graph, const FBlueprintToCppOptions& Options, FBlueprintToCppResult& Result);

	/** Trace execution flow and generate code */
	static TArray<FString> GenerateCodeFromExecutionFlow(
		const TArray<FBlueprintNodeInfo>& Nodes,
		const TArray<FBlueprintPinConnection>& Connections,
		const FString& StartNodeID,
		const FBlueprintToCppOptions& Options,
		FBlueprintToCppResult& Result);

	// =====================================================
	// Node Converters (Pattern Matching)
	// =====================================================

	/** Convert a K2 node to C++ code */
	static TArray<FString> ConvertNode(
		const FBlueprintNodeInfo& Node,
		const TMap<FString, FBlueprintNodeInfo>& NodeMap,
		const TArray<FBlueprintPinConnection>& Connections,
		const FBlueprintToCppOptions& Options,
		FBlueprintToCppResult& Result);

	/** Convert function call node */
	static TArray<FString> ConvertFunctionCallNode(const FBlueprintNodeInfo& Node, const TArray<FBlueprintPinConnection>& Connections, FBlueprintToCppResult& Result);

	/** Convert branch (if) node */
	static TArray<FString> ConvertBranchNode(const FBlueprintNodeInfo& Node, const TArray<FBlueprintPinConnection>& Connections, FBlueprintToCppResult& Result);

	/** Convert sequence node */
	static TArray<FString> ConvertSequenceNode(const FBlueprintNodeInfo& Node, const TArray<FBlueprintPinConnection>& Connections, FBlueprintToCppResult& Result);

	/** Convert variable get node */
	static TArray<FString> ConvertVariableGetNode(const FBlueprintNodeInfo& Node, FBlueprintToCppResult& Result);

	/** Convert variable set node */
	static TArray<FString> ConvertVariableSetNode(const FBlueprintNodeInfo& Node, const TArray<FBlueprintPinConnection>& Connections, FBlueprintToCppResult& Result);

	/** Convert math operation node */
	static TArray<FString> ConvertMathNode(const FBlueprintNodeInfo& Node, const TArray<FBlueprintPinConnection>& Connections, FBlueprintToCppResult& Result);

	/** Convert event node (entry point) */
	static FString ConvertEventNode(const FBlueprintNodeInfo& Node, FBlueprintToCppResult& Result);

	/** Convert delay/latent node */
	static TArray<FString> ConvertLatentNode(const FBlueprintNodeInfo& Node, const TArray<FBlueprintPinConnection>& Connections, FBlueprintToCppResult& Result);

	/** Convert foreach loop node */
	static TArray<FString> ConvertForEachNode(const FBlueprintNodeInfo& Node, const TArray<FBlueprintPinConnection>& Connections, FBlueprintToCppResult& Result);

	// =====================================================
	// Code Generation Helpers
	// =====================================================

	/** Generate header file content */
	static FString GenerateHeaderFile(
		const FString& ClassName,
		const FString& ParentClass,
		const TArray<FConvertedVariable>& Variables,
		const TArray<FConvertedFunction>& Functions,
		const FBlueprintToCppOptions& Options,
		FBlueprintToCppResult& Result);

	/** Generate source file content */
	static FString GenerateSourceFile(
		const FString& ClassName,
		const TArray<FConvertedFunction>& Functions,
		const FBlueprintToCppOptions& Options,
		FBlueprintToCppResult& Result);

	/** Convert Blueprint type to C++ type */
	static FString ConvertPinTypeToType(const FString& PinType);

	/** Get C++ operator for math node */
	static FString GetMathOperator(const FString& NodeClass);

	/** Generate include for a type */
	static FString GetIncludeForType(const FString& TypeName);

	/** Format code with proper indentation */
	static FString FormatCode(const TArray<FString>& Lines, int32 BaseIndent = 0);

	/** Extract function name from node title */
	static FString ExtractFunctionName(const FString& NodeTitle);

	/** Get input pin value (literal or variable reference) */
	static FString GetPinInputValue(
		const FBlueprintNodeInfo& Node,
		const FString& PinName,
		const TArray<FBlueprintPinConnection>& Connections,
		const TMap<FString, FBlueprintNodeInfo>& NodeMap);

	/** Check if node is supported for conversion */
	static bool IsNodeSupported(const FBlueprintNodeInfo& Node, FString& OutReason);

	/** Get event function signature */
	static void GetEventSignature(const FString& EventName, FString& OutReturnType, TArray<TPair<FString, FString>>& OutParams);
};
