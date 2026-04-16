// RiftToolModel.h
// Plain-data model for user-authored tools. Serializes to .riftool JSON files
// under Project/Saved/RiftbornAI/Tools/. Kept free of UObject machinery so
// the builder UI can load/save without a cooker dependency.

#pragma once

#include "CoreMinimal.h"

enum class ERiftToolHandlerKind : uint8
{
	Python,
	Blueprint,
	Composition
};

enum class ERiftToolBlastRadius : uint8
{
	ReadOnly,
	SceneMutating,
	Filesystem
};

struct RIFTBORNAI_API FRiftTool
{
	FString Name;
	FString Description;
	// JSON schema for parameters (stored as a string so the user can edit it raw).
	FString ParamsSchemaJson = TEXT("{\n  \"type\": \"object\",\n  \"properties\": {}\n}");
	ERiftToolHandlerKind HandlerKind = ERiftToolHandlerKind::Python;
	// For Python: the script body. For Blueprint: "/Game/Path.Asset::FunctionName".
	// For Composition: a JSON array of tool-call steps.
	FString HandlerBody;
	ERiftToolBlastRadius BlastRadius = ERiftToolBlastRadius::ReadOnly;

	// Whether the agent is allowed to pick this tool. Disabled tools remain
	// on disk but are unregistered from FClaudeToolRegistry, so toggling one
	// off is a fast way to isolate a broken tool without deleting it.
	bool    bEnabled = true;

	// --- Marketplace metadata (Phase 5). Populated on publish / install. ---
	// Author and version are authored locally; the rest is stamped by the
	// marketplace backend when a tool is published or purchased.
	FString Author;          // creator display name
	FString Version = TEXT("0.1.0");
	FString License = TEXT("MIT");
	int32   PriceCents = 0;  // 0 = free

	// Stamped by the server on publish/purchase. Empty for purely-local tools.
	FString ListingId;
	FString Signature;       // base64 ed25519 over canonical bundle JSON
	FString BundleSha256;
	// 'authored' = you wrote this tool; 'installed' = purchased/downloaded.
	// Drives the on-disk subdir and whether the UI shows a Publish or Buy path.
	FString Provenance = TEXT("authored");

	// Roundtrip helpers.
	FString ToJson() const;
	static bool FromJson(const FString& Json, FRiftTool& Out);

	// Filesystem helpers — all safe to call from the game thread.
	static FString ToolsDir();
	static FString FileNameFor(const FString& ToolName);  // sanitized, ".riftool" appended
	static TArray<FRiftTool> LoadAll();
	static bool Save(const FRiftTool& Tool, FString* OutError = nullptr);
	static bool Delete(const FString& ToolName);

	// Kind <-> string.
	static FString KindToString(ERiftToolHandlerKind K);
	static ERiftToolHandlerKind KindFromString(const FString& S);
	static FString BlastToString(ERiftToolBlastRadius B);
	static ERiftToolBlastRadius BlastFromString(const FString& S);
};
