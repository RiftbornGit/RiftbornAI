// Copyright RiftbornAI. All Rights Reserved.
// BlueprintFixService - Auto-suggests fixes for Blueprint compile errors

#pragma once

#include "CoreMinimal.h"
#include "BlueprintPatch.h"
#include "EditorContextService.h"

class UBlueprint;
class UEdGraphNode;

/**
 * Compile error with detailed context for LLM fix generation
 */
struct RIFTBORNAI_API FBlueprintCompileError
{
	/** The error message from compiler */
	FString Message;
	
	/** Severity level */
	EMessageSeverity::Type Severity = EMessageSeverity::Error;
	
	/** The node that caused the error (if identifiable) */
	TWeakObjectPtr<UEdGraphNode> SourceNode;
	
	/** Node name for context */
	FString NodeName;
	
	/** Node type (K2Node_CallFunction, etc.) */
	FString NodeType;
	
	/** Graph containing the error */
	FString GraphName;
	
	/** Pin involved in error (if any) */
	FString PinName;
	
	/** Blueprint path */
	FString BlueprintPath;
	
	/** Line number in message log (for jumping to error) */
	int32 LineNumber = -1;
	
	/** Convert to JSON for LLM prompt */
	TSharedPtr<FJsonObject> ToJsonObject() const;
};

/**
 * Singleton service that monitors Blueprint compile errors and auto-suggests fixes.
 * 
 * Flow:
 * 1. Blueprint compile fails
 * 2. Service captures error context
 * 3. Optionally auto-triggers fix suggestion
 * 4. User accepts fix (Tab) or rejects (Esc)
 * 5. Fix applied → auto-recompile
 */
class RIFTBORNAI_API FBlueprintFixService
{
public:
	static FBlueprintFixService& Get();
	
	// === Initialization ===
	
	/** Start monitoring Blueprint compiles */
	void Initialize();
	
	/** Stop monitoring */
	void Shutdown();
	
	// === Error Capture ===
	
	/** Parse compile errors from a Blueprint */
	TArray<FBlueprintCompileError> CaptureCompileErrors(UBlueprint* Blueprint);
	
	/** Get last compile errors (cached) */
	const TArray<FBlueprintCompileError>& GetLastErrors() const { return LastCompileErrors; }
	
	/** Check if there are fixable errors */
	bool HasFixableErrors() const { return LastCompileErrors.Num() > 0; }
	
	// === Fix Suggestion ===
	
	/** Request a fix suggestion for the first/next error */
	void RequestFixSuggestion();
	
	/** Request fixes for all errors (batch mode) */
	void RequestFixAll();
	
	/** Cancel in-flight fix request */
	void CancelFixRequest();
	
	/** Is a fix request in flight? */
	bool IsRequestInFlight() const { return bRequestInFlight; }
	
	// === Fix Application ===
	
	/** Accept and apply the current suggested fix */
	void ApplyCurrentFix();
	
	/** Accept and apply all suggested fixes */
	void ApplyAllFixes();
	
	/** Reject/dismiss current fix suggestion */
	void RejectCurrentFix();
	
	/** Recompile after fix applied */
	void RecompileBlueprint();
	
	// === Configuration ===
	
	/** Enable/disable auto-fix popup on compile error */
	void SetAutoFixEnabled(bool bEnabled) { bAutoFixEnabled = bEnabled; }
	bool IsAutoFixEnabled() const { return bAutoFixEnabled; }
	
	/** Set max errors to process in Fix All */
	void SetMaxErrorsToFix(int32 Max) { MaxErrorsToFix = Max; }
	
	// === Delegates ===
	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnCompileErrorsDetected, const TArray<FBlueprintCompileError>&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnFixSuggested, const FBlueprintPatch&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnFixApplied, bool /* bSuccess */);
	
	FOnCompileErrorsDetected OnCompileErrorsDetected;
	FOnFixSuggested OnFixSuggested;
	FOnFixApplied OnFixApplied;

private:
	FBlueprintFixService();
	~FBlueprintFixService();
	
	// Compile event handlers
	void OnBlueprintPreCompile(UBlueprint* Blueprint);
	void OnBlueprintCompiled(UBlueprint* Blueprint);
	void OnAnyBlueprintCompiled();  // No-arg delegate handler
	
	// Build LLM prompt for fix suggestion
	FString BuildFixPrompt(const FBlueprintCompileError& Error, const FBlueprintGraphContext& Context);
	
	// Process LLM response
	void ProcessFixResponse(bool bSuccess, const FString& Response);
	
	// Navigate to error node in editor
	void NavigateToError(const FBlueprintCompileError& Error);
	
	// Highlight error node
	void HighlightErrorNode(const FBlueprintCompileError& Error);
	
	// State
	bool bInitialized = false;
	bool bAutoFixEnabled = true;
	bool bRequestInFlight = false;
	bool bCapturingErrors = false;  // Re-entrancy guard for CaptureCompileErrors
	int32 MaxErrorsToFix = 10;
	int32 CurrentErrorIndex = 0;
	
	// Cached errors from last compile
	TArray<FBlueprintCompileError> LastCompileErrors;
	
	// Current fix being suggested
	FBlueprintPatch CurrentFixPatch;
	
	// All fixes for Fix All mode
	TArray<FBlueprintPatch> AllFixPatches;
	
	// Blueprint being fixed
	TWeakObjectPtr<UBlueprint> TargetBlueprintWeak;
	
	// Last pre-compiled Blueprint (for OnAnyBlueprintCompiled)
	TWeakObjectPtr<UBlueprint> LastPreCompiledBlueprintWeak;
	
	// Delegate handles for cleanup
	FDelegateHandle PreCompileHandle;
	FDelegateHandle CompiledHandle;
	
	// LLM client
	TSharedPtr<class FOllamaClient> LlmClient;
	
	// Request tracking
	uint32 CurrentRequestId = 0;
};
