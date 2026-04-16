// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "RiftbornGovernanceTypes.generated.h"

/**
 * Policy decision for tool execution
 * Represents the governance layer's verdict on whether a tool may run
 */
UENUM(BlueprintType)
enum class ERiftbornPolicyDecision : uint8
{
	/** Tool execution allowed without restriction */
	Allowed UMETA(DisplayName = "Allowed"),

	/** Tool requires explicit user confirmation before execution */
	NeedsConfirmation UMETA(DisplayName = "Needs Confirmation"),

	/** Tool execution forbidden by policy (profile, trust, etc.) */
	Forbidden UMETA(DisplayName = "Forbidden"),

	/** Tool arguments failed validation before execution */
	FailedValidation UMETA(DisplayName = "Failed Validation")
};

/**
 * Undo strategy for tool operations
 * Tools must declare their undo contract at registration time
 * 
 * IMPORTANT: Until proper undo implementation exists, all tools should
 * declare NoUndo (honest) or Snapshot (if they support editor undo).
 * InverseOperation requires actual inverse implementation.
 */
UENUM(BlueprintType)
enum class EToolUndoStrategy : uint8
{
	/** Tool cannot be undone - honest declaration (DEFAULT) */
	NoUndo UMETA(DisplayName = "No Undo"),
	
	/** Tool uses inverse operation (e.g., delete undoes create) */
	InverseOperation UMETA(DisplayName = "Inverse Operation"),
	
	/** Tool uses Unreal transaction/snapshot system */
	Snapshot UMETA(DisplayName = "Snapshot"),
	
	/** Undo strategy not yet determined - treated as NoUndo */
	NotDeclared UMETA(DisplayName = "Not Declared")
};

/**
 * Reversibility classification for tool operations
 * Used by UI to display undo capability honestly
 */
UENUM(BlueprintType)
enum class ERiftbornReversibility : uint8
{
	/** Operation can be undone (undo token available or no-op) */
	Reversible UMETA(DisplayName = "Reversible"),

	/** Operation cannot be undone */
	NotReversible UMETA(DisplayName = "Not Reversible"),

	/** Reversibility not determined */
	Unknown UMETA(DisplayName = "Unknown")
};

/**
 * Helper functions for governance enum serialization
 * Uses lowercase snake_case for API stability (JSON wire format)
 */
namespace RiftbornGovernance
{
	/** Convert EToolRisk to lowercase snake_case string */
	inline FString ToolRiskToString(uint8 Risk)
	{
		// EToolRisk values: Safe=0, Mutation=1, Dangerous=2, Destructive=3, Unknown=4
		switch (Risk)
		{
			case 0: return TEXT("safe");
			case 1: return TEXT("mutation");
			case 2: return TEXT("dangerous");
			case 3: return TEXT("destructive");
			default: return TEXT("unknown");
		}
	}

	/** Convert ERiftbornPolicyDecision to lowercase snake_case string */
	inline FString PolicyDecisionToString(ERiftbornPolicyDecision Decision)
	{
		switch (Decision)
		{
			case ERiftbornPolicyDecision::Allowed: return TEXT("allowed");
			case ERiftbornPolicyDecision::NeedsConfirmation: return TEXT("needs_confirmation");
			case ERiftbornPolicyDecision::Forbidden: return TEXT("forbidden");
			case ERiftbornPolicyDecision::FailedValidation: return TEXT("failed_validation");
			default: return TEXT("unknown");
		}
	}

	/** Convert ERiftbornReversibility to lowercase snake_case string */
	inline FString ReversibilityToString(ERiftbornReversibility Reversibility)
	{
		switch (Reversibility)
		{
			case ERiftbornReversibility::Reversible: return TEXT("reversible");
			case ERiftbornReversibility::NotReversible: return TEXT("not_reversible");
			default: return TEXT("unknown");
		}
	}

	/** Convert EToolUndoStrategy to lowercase snake_case string */
	inline FString UndoStrategyToString(EToolUndoStrategy Strategy)
	{
		switch (Strategy)
		{
			case EToolUndoStrategy::NoUndo: return TEXT("no_undo");
			case EToolUndoStrategy::InverseOperation: return TEXT("inverse_operation");
			case EToolUndoStrategy::Snapshot: return TEXT("snapshot");
			case EToolUndoStrategy::NotDeclared: return TEXT("not_declared");
			default: return TEXT("not_declared");
		}
	}

	/** Response schema version - increment when breaking changes occur */
	inline const TCHAR* GetResponseSchemaVersion()
	{
		return TEXT("v1");
	}

	/** Proof bundle schema version */
	inline const TCHAR* GetProofSchemaVersion()
	{
		return TEXT("v4");
	}
}

/**
 * Confirmation token for NeedsConfirmation policy decisions
 * Binds a blocked tool call to a single-use confirm action
 * 
 * SECURITY: Token is single-use and expires. Args hash prevents
 * "confirm one thing, execute another" attacks.
 */
USTRUCT()
struct FRiftbornConfirmationToken
{
	GENERATED_BODY()

	/** Unique token ID (cryptographically random) */
	UPROPERTY()
	FString Token;

	/** Tool name that was blocked */
	UPROPERTY()
	FString ToolName;

	/** SHA-256 hash of normalized arguments (prevents tampering) */
	UPROPERTY()
	FString ArgsHash;

	/** Original request ID for correlation */
	UPROPERTY()
	FString RequestId;

	/** Session/user identity (for RBAC) */
	UPROPERTY()
	FString SessionId;

	/** UTC timestamp when token expires */
	UPROPERTY()
	FDateTime ExpiresUtc = FDateTime();

	/** Required role to confirm (empty = any authenticated) */
	UPROPERTY()
	FString RequiredRole;

	/** Human-readable reason why confirmation is needed */
	UPROPERTY()
	FString Reason;

	/** Original tool call arguments (stored for execution after confirm) */
	UPROPERTY()
	TMap<FString, FString> OriginalArgs;

	/** Has this token been used? (single-use enforcement) */
	UPROPERTY()
	bool bUsed = false;

	/** Is the token valid and not expired? */
	bool IsValid() const
	{
		return !Token.IsEmpty() && !bUsed && FDateTime::UtcNow() < ExpiresUtc;
	}

	/** Generate a cryptographically random token */
	static FString GenerateToken()
	{
		// Use GUID for uniqueness + random bytes for unpredictability
		FGuid Guid = FGuid::NewGuid();
		return FString::Printf(TEXT("confirm_%s_%08X"), *Guid.ToString(EGuidFormats::Short), FMath::Rand());
	}

	/** Compute SHA-256 hash of arguments for tamper detection */
	static FString ComputeArgsHash(const TMap<FString, FString>& Args)
	{
		// Sort keys for deterministic hashing
		TArray<FString> Keys;
		Args.GetKeys(Keys);
		Keys.Sort();

		FString Normalized;
		for (const FString& Key : Keys)
		{
			Normalized += Key + TEXT("=") + Args[Key] + TEXT(";");
		}

		// Simple hash for now (could upgrade to actual SHA-256)
		uint32 Hash = GetTypeHash(Normalized);
		return FString::Printf(TEXT("%08X"), Hash);
	}

	/** Convert to JSON for API response. Secret token is omitted unless explicitly requested. */
	TSharedPtr<FJsonObject> ToJson(bool bIncludeToken = false) const
	{
		TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
		if (bIncludeToken)
		{
			Json->SetStringField(TEXT("token"), Token);
		}
		Json->SetStringField(TEXT("tool"), ToolName);
		Json->SetStringField(TEXT("args_hash"), ArgsHash);
		Json->SetStringField(TEXT("request_id"), RequestId);
		Json->SetStringField(TEXT("expires_utc"), ExpiresUtc.ToIso8601());
		Json->SetStringField(TEXT("required_role"), RequiredRole);
		Json->SetStringField(TEXT("reason"), Reason);
		return Json;
	}
};
