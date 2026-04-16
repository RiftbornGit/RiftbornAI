// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

/**
 * RiftbornAI Logging System
 * 
 * Provides a dedicated log category and convenience macros for logging
 * throughout the RiftbornAI plugin. All logs appear under LogRiftbornAI
 * category in the Output Log.
 * 
 * Macros:
 *   RIFTBORN_LOG(Verbosity, Format, ...)  - Generic log with verbosity
 *   RIFTBORN_LOG_ERROR(Format, ...)       - Error level (red)
 *   RIFTBORN_LOG_WARNING(Format, ...)     - Warning level (yellow) 
 *   RIFTBORN_LOG_INFO(Format, ...)        - Normal info level
 *   RIFTBORN_LOG_VERBOSE(Format, ...)     - Verbose level (hidden by default)
 * 
 * Example:
 *   RIFTBORN_LOG_INFO(TEXT("Tool executed: %s"), *ToolName);
 *   RIFTBORN_LOG_ERROR(TEXT("Failed to load asset: %s"), *AssetPath);
 */

// Custom log category for RiftbornAI
RIFTBORNAI_API DECLARE_LOG_CATEGORY_EXTERN(LogRiftbornAI, Log, All);

// Backward compatibility alias - some files use LogRiftborn
#define LogRiftborn LogRiftbornAI

// Convenience macros
#define RIFTBORN_LOG(Verbosity, Format, ...) UE_LOG(LogRiftbornAI, Verbosity, Format, ##__VA_ARGS__)
#define RIFTBORN_LOG_ERROR(Format, ...) UE_LOG(LogRiftbornAI, Error, Format, ##__VA_ARGS__)
#define RIFTBORN_LOG_WARNING(Format, ...) UE_LOG(LogRiftbornAI, Warning, Format, ##__VA_ARGS__)
#define RIFTBORN_LOG_INFO(Format, ...) UE_LOG(LogRiftbornAI, Log, Format, ##__VA_ARGS__)
#define RIFTBORN_LOG_VERBOSE(Format, ...) UE_LOG(LogRiftbornAI, Verbose, Format, ##__VA_ARGS__)
