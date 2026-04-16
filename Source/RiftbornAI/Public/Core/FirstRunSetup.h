// Copyright RiftbornAI. All Rights Reserved.
// FirstRunSetup - Zero-config onboarding: auto-detect providers, auto-pull models
// Gap #5: First-run friction elimination

#pragma once

#include "CoreMinimal.h"

/**
 * Environment check result
 */
struct RIFTBORNAI_API FEnvironmentStatus
{
	/** Is Ollama installed and running? */
	bool bOllamaAvailable = false;
	
	/** Ollama version string */
	FString OllamaVersion;
	
	/** Available Ollama models */
	TArray<FString> OllamaModels;
	
	/** Does the user have at least one usable model? */
	bool bHasUsableModel = false;
	
	/** Recommended model to pull (if none available) */
	FString RecommendedModel;
	
	/** Does the user have any cloud API keys configured? */
	bool bHasCloudApiKey = false;
	
	/** Which cloud providers have keys? */
	TArray<FString> ConfiguredCloudProviders;
	
	/** Is Python available? */
	bool bPythonAvailable = false;
	
	/** Python version */
	FString PythonVersion;
	
	/** Are Bridge dependencies installed? */
	bool bBridgeDepsInstalled = false;

	/** Is the Claude Code CLI installed and on PATH? */
	bool bClaudeCodeAvailable = false;

	/** Reported version of `claude --version`, or empty if unavailable. */
	FString ClaudeCodeVersion;

	/** Absolute path to the detected `claude` binary, or empty if unavailable. */
	FString ClaudeCodePath;

	/** Overall readiness: can the copilot work right now? */
	bool bReady = false;
	
	/** Human-readable summary of what's missing */
	FString Summary;
	
	/** Human-readable steps to fix missing dependencies */
	TArray<FString> FixSteps;
};

/**
 * First-Run Setup & Environment Validator
 * 
 * Runs on plugin startup to ensure everything is configured.
 * Designed to eliminate the "install Ollama, pull model, setup Python, configure..."
 * friction that kills adoption.
 * 
 * Flow:
 *   1. Check if Ollama is installed → if not, prompt to install
 *   2. Check if a usable model exists → if not, auto-pull recommended model
 *   3. Check if cloud API keys are set → if yes, configure as provider
 *   4. Check Python + Bridge → if missing, offer guided setup
 *   5. Set everything up with zero manual config required
 */
class RIFTBORNAI_API FFirstRunSetup
{
public:
	static FFirstRunSetup& Get();
	
	/**
	 * Run the full environment check.
	 * This is fast (~100ms if Ollama is running, ~2s if checking models).
	 */
	FEnvironmentStatus CheckEnvironment();
	
	/**
	 * Auto-configure based on environment.
	 * Sets RiftbornSettings to match what's available.
	 * Returns true if at least one provider is usable.
	 */
	bool AutoConfigure();
	
	/**
	 * Request Ollama to pull a model (async).
	 * 
	 * @param ModelName Model to pull (e.g., "qwen2.5-coder:7b")
	 * @param OnComplete Callback when done (bool success, FString message)
	 */
	void PullOllamaModel(
		const FString& ModelName,
		TFunction<void(bool bSuccess, const FString& Message)> OnComplete = nullptr);
	
	/**
	 * Check if Ollama is installed (binary exists on PATH or common locations).
	 */
	bool IsOllamaInstalled() const;
	
	/**
	 * Try to start Ollama serve if it's installed but not running.
	 */
	bool TryStartOllama();
	
	/**
	 * Has setup already completed this session?
	 */
	bool HasCompletedSetup() const { return bSetupCompleted; }
	
	/**
	 * Get the last environment status (from CheckEnvironment).
	 */
	const FEnvironmentStatus& GetLastStatus() const { return LastStatus; }
	
	/**
	 * Run on editor startup (called from module startup).
	 * Performs auto-configuration silently.
	 */
	void RunStartupCheck();

private:
	FFirstRunSetup() = default;
	
	/** Check Ollama availability and models */
	void CheckOllama(FEnvironmentStatus& Status);
	
	/** Check cloud API keys */
	void CheckCloudProviders(FEnvironmentStatus& Status);
	
	/** Check Python environment */
	void CheckPython(FEnvironmentStatus& Status);

	/** Check Claude Code CLI presence (subscription-seat backend). */
	void CheckClaudeCode(FEnvironmentStatus& Status);
	
	/** Pick the best default model from available options */
	FString PickBestModel(const TArray<FString>& AvailableModels) const;
	
	/** Model preference order (best first) */
	static TArray<FString> GetModelPreferenceOrder();
	
	bool bSetupCompleted = false;
	FEnvironmentStatus LastStatus;
};
