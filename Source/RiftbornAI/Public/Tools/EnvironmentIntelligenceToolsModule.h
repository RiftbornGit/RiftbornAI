// Copyright RiftbornAI. All Rights Reserved.
// Environment Intelligence Tools Module — scene analysis, acoustics, weathering, weather simulation.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Tools/ToolModuleBase.h"

/**
 * FEnvironmentIntelligenceToolsModule
 * 
 * Environment Intelligence Tools Module — scene analysis, acoustics, weathering, weather simulation.
 */
class RIFTBORNAI_API FEnvironmentIntelligenceToolsModule : public TToolModuleBase<FEnvironmentIntelligenceToolsModule>
{
public:
	static FString StaticModuleName() { return TEXT("EnvironmentIntelligenceTools"); }

	virtual void RegisterTools(FClaudeToolRegistry& Registry) override;

	// Tool 3: AI Scene Critic — analyze + auto-fix scene issues
	static FClaudeToolResult Tool_CritiqueScene(const FClaudeToolCall& Call);

	// Tool 5: Acoustic Space Analyzer — line-trace room geometry → auto-reverb
	static FClaudeToolResult Tool_AnalyzeAcoustics(const FClaudeToolCall& Call);

	// Tool 9: Weathering System — age surfaces procedurally
	static FClaudeToolResult Tool_ApplyWeathering(const FClaudeToolCall& Call);

	// Tool 10: Live Replay Sculptor — record PIE camera path for guided editing
	static FClaudeToolResult Tool_RecordReplay(const FClaudeToolCall& Call);

	// Tool 11: Dynamic Weather — rain/snow/puddles/wetness via MaterialParameterCollection
	static FClaudeToolResult Tool_SetWeather(const FClaudeToolCall& Call);
};
