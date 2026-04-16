// Copyright RiftbornAI. All Rights Reserved.
//
// CopilotModelRegistry — single source of truth for the curated model set
// the in-editor copilot exposes in its dropdown. Replaces the substring
// `.Contains("claude")` / `.Contains("gpt")` checks that were sprinkled
// across CopilotPanel_*.cpp and would silently mis-route a fifth provider.
//
// Discovered models from FModelDiscovery still override registry context
// limits when present — the registry is the fallback, not the gate.

#pragma once

#include "CoreMinimal.h"

/** A single curated copilot model entry. All fields are required — empty
 *  strings here mean the entry's behaviour drifts at runtime, which is
 *  exactly what the registry exists to prevent. */
struct RIFTBORNAI_API FCopilotModelEntry
{
	/** Canonical model ID stored in CurrentModelId / persisted to disk. */
	const TCHAR* ModelId = nullptr;

	/** Human-readable label shown in the dropdown row. */
	const TCHAR* DisplayLabel = nullptr;

	/** Provider name accepted by FAIProviderFactory::CreateProvider
	 *  (e.g. "Claude", "ClaudeCode", "OpenAI", "Ollama"). */
	const TCHAR* ProviderName = nullptr;

	/** Model ID actually sent to provider->SetModel. For aliases like
	 *  `claude-code:opus-4-6` the underlying API model is `claude-opus-4-6`.
	 *  Empty string => same as ModelId. */
	const TCHAR* UnderlyingModelId = nullptr;

	/** Token context window. Fallback only — FModelDiscovery wins when its
	 *  cache has a real value for this ID. */
	int32 ContextLimit = 8192;

	/** True if this is a cloud / subscription-billed model. Local Ollama
	 *  models are false. Used by the dropdown's "fall back to local model
	 *  when no cloud key is configured" path. */
	bool bIsCloud = false;

	/** True if the provider must pass IAIProvider::IsConfigured() before
	 *  this model can be selected (cloud API keys, Claude Code CLI). False
	 *  for local Ollama which gates on endpoint reachability separately. */
	bool bRequiresAuth = false;
};

namespace RiftbornCopilotModels
{
	/** Static curated registry. Order is the dropdown order. */
	RIFTBORNAI_API TArrayView<const FCopilotModelEntry> GetRegistry();

	/** Lookup by canonical model ID. Returns nullptr if not in the registry
	 *  (e.g. an Ollama-discovered local model, an unknown model from a
	 *  persisted session). Callers must handle the nullptr case. */
	RIFTBORNAI_API const FCopilotModelEntry* FindByModelId(const FString& ModelId);
}
