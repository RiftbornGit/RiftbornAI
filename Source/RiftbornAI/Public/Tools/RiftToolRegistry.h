// RiftToolRegistry.h
// Bridges user-authored .riftool files into FClaudeToolRegistry so the agent
// can invoke them. Scanned at startup and on each save from the builder panel.

#pragma once

#include "CoreMinimal.h"

struct FRiftTool;

class RIFTBORNAI_API FRiftToolRegistry
{
public:
	/** Unregister previously-registered Rift tools, then re-register from disk.
	 *  Returns the number of tools registered. Safe to call repeatedly. */
	static int32 ScanAndRegister();

	/** Register a single tool (also persists the name so a later re-scan
	 *  unregisters it cleanly). Returns false on schema-parse or name collision. */
	static bool RegisterOne(const FRiftTool& Tool, FString* OutError = nullptr);

	/** Unregister by name — used when a tool is deleted from the UI. */
	static void UnregisterOne(const FString& ToolName);

	/** Count of user-authored Rift tools currently registered. */
	static int32 GetRegisteredCount();

	/** Synchronous execute-by-name for the Test pane.
	 *  ArgsJson is an object like {"radius": 500, "seed": 1}; values are flattened
	 *  to strings (this matches FClaudeToolCall::Arguments). */
	static bool InvokeByName(const FString& ToolName,
	                         const FString& ArgsJson,
	                         FString& OutResult,
	                         FString& OutError);
};
