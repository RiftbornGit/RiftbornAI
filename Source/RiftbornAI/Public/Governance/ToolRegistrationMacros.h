// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"

/**
 * Macros for simplified tool registration
 * 
 * Usage:
 *   RIFTBORN_TOOL_BEGIN("spawn_actor", "Spawn an actor in the level")
 *       RIFTBORN_PARAM_STRING_REQ("class_name", "Actor class or Blueprint path")
 *       RIFTBORN_PARAM_NUMBER_OPT("x", "X position", "0")
 *       RIFTBORN_PARAM_NUMBER_OPT("y", "Y position", "0") 
 *       RIFTBORN_PARAM_NUMBER_OPT("z", "Z position", "0")
 *   RIFTBORN_TOOL_END(Tool_SpawnActor)
 */

// Helper to create a tool parameter (works with USTRUCT)
inline FClaudeToolParameter MakeToolParam(const FString& Name, EClaudeToolParamType Type, const FString& Description, bool bRequired, const FString& Default = TEXT(""))
{
	FClaudeToolParameter Param;
	Param.Name = Name;
	Param.Type = Type;
	Param.Description = Description;
	Param.bRequired = bRequired;
	Param.DefaultValue = Default;
	return Param;
}

// Begin tool definition - pass raw string literals 
// Example: RIFTBORN_TOOL_BEGIN("spawn_actor", "Spawn an actor in the level")
#define RIFTBORN_TOOL_BEGIN(ToolName, Description) \
	{ \
		FClaudeTool _Tool; \
		_Tool.Name = FString(ToolName); \
		_Tool.Description = FString(Description);

// Required string parameter
// Example: RIFTBORN_PARAM_STRING_REQ("class_name", "Actor class or Blueprint path")
#define RIFTBORN_PARAM_STRING_REQ(ParamName, Description) \
		_Tool.Parameters.Add(MakeToolParam(FString(ParamName), EClaudeToolParamType::String, FString(Description), true));

// Optional string parameter with default
#define RIFTBORN_PARAM_STRING_OPT(ParamName, Description, Default) \
		_Tool.Parameters.Add(MakeToolParam(FString(ParamName), EClaudeToolParamType::String, FString(Description), false, FString(Default)));

// Required number parameter
#define RIFTBORN_PARAM_NUMBER_REQ(ParamName, Description) \
		_Tool.Parameters.Add(MakeToolParam(FString(ParamName), EClaudeToolParamType::Number, FString(Description), true));

// Optional number parameter with default
#define RIFTBORN_PARAM_NUMBER_OPT(ParamName, Description, Default) \
		_Tool.Parameters.Add(MakeToolParam(FString(ParamName), EClaudeToolParamType::Number, FString(Description), false, FString(Default)));

// Required integer parameter
#define RIFTBORN_PARAM_INT_REQ(ParamName, Description) \
		_Tool.Parameters.Add(MakeToolParam(FString(ParamName), EClaudeToolParamType::Integer, FString(Description), true));

// Alias for INTEGER (some code uses this)
#define RIFTBORN_PARAM_INTEGER_REQ(ParamName, Description) RIFTBORN_PARAM_INT_REQ(ParamName, Description)
#define RIFTBORN_PARAM_INTEGER_OPT(ParamName, Description, Default) RIFTBORN_PARAM_INT_OPT(ParamName, Description, Default)

// Optional integer parameter with default
#define RIFTBORN_PARAM_INT_OPT(ParamName, Description, Default) \
		_Tool.Parameters.Add(MakeToolParam(FString(ParamName), EClaudeToolParamType::Integer, FString(Description), false, FString(Default)));

// Required boolean parameter
#define RIFTBORN_PARAM_BOOL_REQ(ParamName, Description) \
		_Tool.Parameters.Add(MakeToolParam(FString(ParamName), EClaudeToolParamType::Boolean, FString(Description), true));

// Optional boolean parameter with default
#define RIFTBORN_PARAM_BOOL_OPT(ParamName, Description, Default) \
		_Tool.Parameters.Add(MakeToolParam(FString(ParamName), EClaudeToolParamType::Boolean, FString(Description), false, FString(Default)));

// End tool definition and register with handler
#define RIFTBORN_TOOL_END(HandlerFunc) \
		Registry.RegisterTool(_Tool, FOnExecuteTool::CreateStatic(&FRiftbornBuiltinTools::HandlerFunc)); \
	}

// Set tool category (call before RIFTBORN_TOOL_END)
#define RIFTBORN_TOOL_CATEGORY(CategoryName) \
		_Tool.Category = TEXT(CategoryName);

// Set tool visibility (call before RIFTBORN_TOOL_END)
#define RIFTBORN_TOOL_VISIBILITY(Vis) \
		_Tool.Visibility = EToolVisibility::Vis;

// Set tool risk level (call before RIFTBORN_TOOL_END)
#define RIFTBORN_TOOL_RISK(RiskLevel) \
		_Tool.Risk = EToolRisk::RiskLevel;

// Set tool cost hint (call before RIFTBORN_TOOL_END)
#define RIFTBORN_TOOL_COST(CostLevel) \
		_Tool.Cost = EToolCost::CostLevel;

// End tool definition for modular tools (specify full class::method)
#define RIFTBORN_TOOL_END_MODULE(FullHandler) \
		Registry.RegisterTool(_Tool, FOnExecuteTool::CreateStatic(&FullHandler)); \
	}

// End tool definition with no parameters
#define RIFTBORN_TOOL_END_NO_PARAMS(HandlerFunc) \
		Registry.RegisterTool(_Tool, FOnExecuteTool::CreateStatic(&FRiftbornBuiltinTools::HandlerFunc)); \
	}

// Simple tool with no parameters
#define RIFTBORN_TOOL_SIMPLE(ToolName, Description, HandlerFunc) \
	{ \
		FClaudeTool _Tool; \
		_Tool.Name = TEXT(#ToolName); \
		_Tool.Description = TEXT(Description); \
		Registry.RegisterTool(_Tool, FOnExecuteTool::CreateStatic(&FRiftbornBuiltinTools::HandlerFunc)); \
	}

/**
 * Helper to get required string argument with error handling
 */
inline bool GetRequiredArg(const FClaudeToolCall& Call, const TCHAR* ArgName, FString& OutValue, FClaudeToolResult& OutResult)
{
	const FString* Value = Call.Arguments.Find(ArgName);
	if (!Value || Value->IsEmpty())
	{
		OutResult.bSuccess = false;
		OutResult.ErrorMessage = FString::Printf(TEXT("Missing required parameter: %s"), ArgName);
		return false;
	}
	OutValue = *Value;
	return true;
}

/**
 * Helper to get optional string argument with default
 */
inline FString GetOptionalArg(const FClaudeToolCall& Call, const TCHAR* ArgName, const FString& DefaultValue = TEXT(""))
{
	const FString* Value = Call.Arguments.Find(ArgName);
	return (Value && !Value->IsEmpty()) ? *Value : DefaultValue;
}

/**
 * Helper to get optional number argument with default
 */
inline float GetOptionalNumber(const FClaudeToolCall& Call, const TCHAR* ArgName, float DefaultValue = 0.0f)
{
	const FString* Value = Call.Arguments.Find(ArgName);
	return (Value && !Value->IsEmpty()) ? FCString::Atof(**Value) : DefaultValue;
}

/**
 * Helper to get optional integer argument with default
 */
inline int32 GetOptionalInt(const FClaudeToolCall& Call, const TCHAR* ArgName, int32 DefaultValue = 0)
{
	const FString* Value = Call.Arguments.Find(ArgName);
	return (Value && !Value->IsEmpty()) ? FCString::Atoi(**Value) : DefaultValue;
}

/**
 * Helper to get optional boolean argument with default
 */
inline bool GetOptionalBool(const FClaudeToolCall& Call, const TCHAR* ArgName, bool DefaultValue = false)
{
	const FString* Value = Call.Arguments.Find(ArgName);
	if (!Value || Value->IsEmpty())
	{
		return DefaultValue;
	}
	return Value->ToBool();
}

/**
 * Create a success result
 */
inline FClaudeToolResult MakeSuccessResult(const FString& ToolUseId, const FString& Message)
{
	FClaudeToolResult Result;
	Result.ToolUseId = ToolUseId;
	Result.bSuccess = true;
	Result.Result = Message;
	return Result;
}

/**
 * Create an error result
 */
inline FClaudeToolResult MakeErrorResult(const FString& ToolUseId, const FString& ErrorMessage)
{
	FClaudeToolResult Result;
	Result.ToolUseId = ToolUseId;
	Result.bSuccess = false;
	Result.ErrorMessage = ErrorMessage;
	return Result;
}

// ============================================================================
// STRICT REGISTRATION MACROS - Force explicit Risk/Visibility
// ============================================================================
// 
// These macros REQUIRE Risk to be set. Use for new tools.
// 
// Usage:
//   RIFTBORN_TOOL_STRICT_BEGIN("spawn_actor", "Spawn an actor", Mutation, Public)
//       RIFTBORN_PARAM_STRING_REQ("class_name", "Actor class")
//   RIFTBORN_TOOL_STRICT_END(Tool_SpawnActor)

/**
 * Begin strict tool definition - REQUIRES Risk and Visibility
 * This eliminates silent defaults that cause governance drift.
 */
#define RIFTBORN_TOOL_STRICT_BEGIN(ToolName, Description, RiskLevel, Vis) \
	{ \
		FClaudeTool _Tool; \
		_Tool.Name = FString(ToolName); \
		_Tool.Description = FString(Description); \
		_Tool.Risk = EToolRisk::RiskLevel; \
		_Tool.Visibility = EToolVisibility::Vis; \
		/* Compile-time documentation of governance intent */

/**
 * End strict tool definition
 */
#define RIFTBORN_TOOL_STRICT_END(HandlerFunc) \
		Registry.RegisterTool(_Tool, FOnExecuteTool::CreateStatic(&FRiftbornBuiltinTools::HandlerFunc)); \
	}

/**
 * End strict tool definition for modular tools
 */
#define RIFTBORN_TOOL_STRICT_END_MODULE(FullHandler) \
		Registry.RegisterTool(_Tool, FOnExecuteTool::CreateStatic(&FullHandler)); \
	}

