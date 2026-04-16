// TemporalAwarenessTools.h - Agent tools for time, weather, and seasonal understanding

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"

/**
 * Temporal Awareness Tools
 * 
 * Provides agent tools for understanding and controlling:
 * - Time of day (get/set dawn, morning, noon, dusk, night, etc.)
 * - Weather conditions (clear, rain, snow, fog, etc.)
 * - Seasons (spring, summer, fall, winter)
 * - Real-world time and timezones
 * - Lighting recommendations
 * 
 * These tools enable agents to:
 * 1. Query current temporal state
 * 2. Set specific time/weather/season for scene design
 * 3. Get lighting setup recommendations
 * 4. Generate weather forecasts
 */
class RIFTBORNAI_API FTemporalAwarenessTools
{
public:
    /** Register all temporal awareness tools with the agent registry */
    static void RegisterTools();
    
private:
    // Time tools
    static FClaudeToolResult Tool_GetCurrentTime(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetTime(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetTimeOfDay(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetTimeOfDay(const FClaudeToolCall& Call);
    
    // Weather tools
    static FClaudeToolResult Tool_GetWeather(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetWeather(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetForecast(const FClaudeToolCall& Call);
    
    // Season tools
    static FClaudeToolResult Tool_GetSeason(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetSeason(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SetBiome(const FClaudeToolCall& Call);
    
    // Full state
    static FClaudeToolResult Tool_GetTemporalState(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_GetLightingRecommendations(const FClaudeToolCall& Call);
    
    // Real-world time
    static FClaudeToolResult Tool_GetRealWorldTime(const FClaudeToolCall& Call);
    static FClaudeToolResult Tool_SyncToRealTime(const FClaudeToolCall& Call);
    
    // Helper to create success result from JSON string
    static FClaudeToolResult MakeSuccess(const FString& ToolUseId, const FString& JsonResult);
    static FClaudeToolResult MakeFailure(const FString& ToolUseId, const FString& ErrorMessage);
};
