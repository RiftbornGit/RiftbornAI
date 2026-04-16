// CapabilityAdapter.h
// Tool Equivalence Contract: Canonical schemas for semantic tool substitution
//
// Without this layer, "alternative tool" is roulette.
// With this layer, tool substitution is deterministic and auditable.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

// Forward declarations for types used before definition
enum class EToolSelectionReason : uint8;
struct FToolTrustSnapshot;
struct FArgTypeRecord;

/**
 * Canonical actor reference.
 * Normalizes the mess of label/actor_label/actor_name across tools.
 */
struct RIFTBORNAI_API FCanonicalActorRef
{
	/** Reference kind: "label", "guid", or "path" */
	FString Kind = TEXT("label");

	/** The actual reference value */
	FString Value;

	/** Is this reference valid? */
	bool IsValid() const 
	{ 
		if (Value.IsEmpty()) return false;
		return Kind == TEXT("label") || Kind == TEXT("guid") || Kind == TEXT("path"); 
	}
	
	/** Is this a GUID reference? */
	bool IsGuid() const { return Kind == TEXT("guid"); }
	
	/** Is this a path reference? */
	bool IsPath() const { return Kind == TEXT("path"); }

	static FCanonicalActorRef FromLabel(const FString& Label)
	{
		FCanonicalActorRef Ref;
		Ref.Kind = TEXT("label");
		Ref.Value = Label;
		return Ref;
	}
	
	static FCanonicalActorRef FromGuid(const FGuid& Guid)
	{
		FCanonicalActorRef Ref;
		Ref.Kind = TEXT("guid");
		Ref.Value = Guid.ToString();
		return Ref;
	}
	
	static FCanonicalActorRef FromPath(const FString& Path)
	{
		FCanonicalActorRef Ref;
		Ref.Kind = TEXT("path");
		Ref.Value = Path;
		return Ref;
	}
	
	/** Try to parse value as GUID (returns false if invalid) */
	bool TryGetGuid(FGuid& OutGuid) const
	{
		if (Kind != TEXT("guid")) return false;
		return FGuid::Parse(Value, OutGuid);
	}

	TSharedPtr<FJsonObject> ToJson() const
	{
		TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
		Json->SetStringField(TEXT("kind"), Kind);
		Json->SetStringField(TEXT("value"), Value);
		return Json;
	}

	static FCanonicalActorRef FromJson(const TSharedPtr<FJsonObject>& Json)
	{
		FCanonicalActorRef Ref;
		if (Json)
		{
			Ref.Kind = Json->GetStringField(TEXT("kind"));
			Ref.Value = Json->GetStringField(TEXT("value"));
		}
		return Ref;
	}
};

/**
 * Canonical 3D vector for location/scale.
 */
struct RIFTBORNAI_API FCanonicalVector3
{
	float X = 0.0f;
	float Y = 0.0f;
	float Z = 0.0f;

	FCanonicalVector3() = default;
	FCanonicalVector3(float InX, float InY, float InZ) : X(InX), Y(InY), Z(InZ) {}
	explicit FCanonicalVector3(const FVector& V) : X(V.X), Y(V.Y), Z(V.Z) {}

	FVector ToFVector() const { return FVector(X, Y, Z); }

	bool IsDefault(float DefaultX = 0.0f, float DefaultY = 0.0f, float DefaultZ = 0.0f) const
	{
		return FMath::IsNearlyEqual(X, DefaultX) &&
		       FMath::IsNearlyEqual(Y, DefaultY) &&
		       FMath::IsNearlyEqual(Z, DefaultZ);
	}

	TSharedPtr<FJsonObject> ToJson() const
	{
		TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
		Json->SetNumberField(TEXT("x"), X);
		Json->SetNumberField(TEXT("y"), Y);
		Json->SetNumberField(TEXT("z"), Z);
		return Json;
	}

	static FCanonicalVector3 FromJson(const TSharedPtr<FJsonObject>& Json, float DefX = 0.f, float DefY = 0.f, float DefZ = 0.f)
	{
		FCanonicalVector3 V;
		if (Json)
		{
			V.X = Json->HasField(TEXT("x")) ? Json->GetNumberField(TEXT("x")) : DefX;
			V.Y = Json->HasField(TEXT("y")) ? Json->GetNumberField(TEXT("y")) : DefY;
			V.Z = Json->HasField(TEXT("z")) ? Json->GetNumberField(TEXT("z")) : DefZ;
		}
		return V;
	}
};

/**
 * Canonical rotation (Euler angles).
 */
struct RIFTBORNAI_API FCanonicalRotation
{
	float Pitch = 0.0f;
	float Yaw = 0.0f;
	float Roll = 0.0f;

	bool IsDefault() const
	{
		return FMath::IsNearlyZero(Pitch) && FMath::IsNearlyZero(Yaw) && FMath::IsNearlyZero(Roll);
	}

	FRotator ToFRotator() const { return FRotator(Pitch, Yaw, Roll); }

	TSharedPtr<FJsonObject> ToJson() const
	{
		TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
		Json->SetNumberField(TEXT("pitch"), Pitch);
		Json->SetNumberField(TEXT("yaw"), Yaw);
		Json->SetNumberField(TEXT("roll"), Roll);
		return Json;
	}

	static FCanonicalRotation FromJson(const TSharedPtr<FJsonObject>& Json)
	{
		FCanonicalRotation R;
		if (Json)
		{
			R.Pitch = Json->HasField(TEXT("pitch")) ? Json->GetNumberField(TEXT("pitch")) : 0.0f;
			R.Yaw = Json->HasField(TEXT("yaw")) ? Json->GetNumberField(TEXT("yaw")) : 0.0f;
			R.Roll = Json->HasField(TEXT("roll")) ? Json->GetNumberField(TEXT("roll")) : 0.0f;
		}
		return R;
	}
};

/**
 * Canonical transform (location + rotation + scale).
 */
struct RIFTBORNAI_API FCanonicalTransform
{
	FCanonicalVector3 Location;
	FCanonicalRotation Rotation;
	FCanonicalVector3 Scale = FCanonicalVector3(1.0f, 1.0f, 1.0f);

	bool HasNonDefaultRotation() const { return !Rotation.IsDefault(); }
	bool HasNonDefaultScale() const { return !Scale.IsDefault(1.0f, 1.0f, 1.0f); }

	TSharedPtr<FJsonObject> ToJson() const
	{
		TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
		Json->SetObjectField(TEXT("location"), Location.ToJson());
		Json->SetObjectField(TEXT("rotation"), Rotation.ToJson());
		Json->SetObjectField(TEXT("scale"), Scale.ToJson());
		return Json;
	}

	static FCanonicalTransform FromJson(const TSharedPtr<FJsonObject>& Json)
	{
		FCanonicalTransform T;
		if (Json)
		{
			T.Location = FCanonicalVector3::FromJson(Json->GetObjectField(TEXT("location")));
			T.Rotation = FCanonicalRotation::FromJson(Json->GetObjectField(TEXT("rotation")));
			T.Scale = FCanonicalVector3::FromJson(Json->GetObjectField(TEXT("scale")), 1.0f, 1.0f, 1.0f);
		}
		return T;
	}
};

/**
 * Capability: Actor.SetTransform
 * Canonical args for moving/transforming actors.
 * Covers both move_actor and set_actor_transform tools.
 */
struct RIFTBORNAI_API FCapability_ActorSetTransform
{
	static constexpr const TCHAR* CapabilityId = TEXT("Actor.SetTransform");

	FCanonicalActorRef ActorRef;
	FCanonicalTransform Transform;
	FString Space = TEXT("world");  // "world" or "local"
	bool bSweep = false;
	bool bTeleport = false;

	bool IsValid() const { return ActorRef.IsValid(); }

	TSharedPtr<FJsonObject> ToJson() const
	{
		TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
		Json->SetObjectField(TEXT("actor_ref"), ActorRef.ToJson());
		Json->SetObjectField(TEXT("transform"), Transform.ToJson());
		Json->SetStringField(TEXT("space"), Space);
		Json->SetBoolField(TEXT("sweep"), bSweep);
		Json->SetBoolField(TEXT("teleport"), bTeleport);
		return Json;
	}

	static FCapability_ActorSetTransform FromJson(const TSharedPtr<FJsonObject>& Json)
	{
		FCapability_ActorSetTransform Cap;
		if (Json)
		{
			Cap.ActorRef = FCanonicalActorRef::FromJson(Json->GetObjectField(TEXT("actor_ref")));
			Cap.Transform = FCanonicalTransform::FromJson(Json->GetObjectField(TEXT("transform")));
			Cap.Space = Json->HasField(TEXT("space")) ? Json->GetStringField(TEXT("space")) : TEXT("world");
			Cap.bSweep = Json->HasField(TEXT("sweep")) ? Json->GetBoolField(TEXT("sweep")) : false;
			Cap.bTeleport = Json->HasField(TEXT("teleport")) ? Json->GetBoolField(TEXT("teleport")) : false;
		}
		return Cap;
	}
};

/**
 * Result of tool arg adaptation.
 */
struct RIFTBORNAI_API FToolArgAdaptResult
{
	bool bSuccess = false;
	TSharedPtr<FJsonObject> ToolArgs;
	TArray<FString> DroppedFields;  // Fields that couldn't be represented
	FString ErrorMessage;

	static FToolArgAdaptResult Success(TSharedPtr<FJsonObject> Args, TArray<FString> Dropped = {})
	{
		FToolArgAdaptResult R;
		R.bSuccess = true;
		R.ToolArgs = Args;
		R.DroppedFields = MoveTemp(Dropped);
		return R;
	}

	static FToolArgAdaptResult Failure(const FString& Error)
	{
		FToolArgAdaptResult R;
		R.bSuccess = false;
		R.ErrorMessage = Error;
		return R;
	}
};

/**
 * Capability Adapter Registry
 * Owns tool equivalence contracts and performs deterministic arg translation.
 */
class RIFTBORNAI_API FCapabilityAdapter
{
public:
	static FCapabilityAdapter& Get();

	// =========================================================================
	// Capability Registration
	// =========================================================================

	/** Get tools that implement a capability */
	TArray<FString> GetToolsForCapability(const FString& CapabilityId) const;

	/** Get the capability a tool implements (if any) */
	FString GetCapabilityForTool(const FString& ToolName) const;

	/** Check if a tool implements a capability */
	bool ToolImplementsCapability(const FString& ToolName, const FString& CapabilityId) const;

	// =========================================================================
	// Canonical ↔ Tool Arg Translation
	// =========================================================================

	/**
	 * Convert legacy tool args to canonical form.
	 * Use when receiving old-style requests that specify tool name + args.
	 */
	TSharedPtr<FJsonObject> ToCanonicalArgs(
		const FString& ToolName,
		const TSharedPtr<FJsonObject>& ToolArgs,
		FString& OutCapabilityId) const;

	/**
	 * Convert canonical args to tool-specific args.
	 * This is the core adapter function.
	 */
	FToolArgAdaptResult FromCanonicalArgs(
		const FString& CapabilityId,
		const TSharedPtr<FJsonObject>& CanonicalArgs,
		const FString& TargetToolName) const;

	// =========================================================================
	// Actor.SetTransform Specific Adapters
	// =========================================================================

	/** Adapt Actor.SetTransform canonical args to move_actor tool args */
	FToolArgAdaptResult AdaptToMoveActor(const FCapability_ActorSetTransform& Canonical) const;

	/** Adapt Actor.SetTransform canonical args to set_actor_transform tool args */
	FToolArgAdaptResult AdaptToSetActorTransform(const FCapability_ActorSetTransform& Canonical) const;

	/** Convert move_actor tool args to canonical Actor.SetTransform */
	FCapability_ActorSetTransform MoveActorToCanonical(const TSharedPtr<FJsonObject>& ToolArgs) const;

	/** Convert set_actor_transform tool args to canonical Actor.SetTransform */
	FCapability_ActorSetTransform SetActorTransformToCanonical(const TSharedPtr<FJsonObject>& ToolArgs) const;

	// =========================================================================
	// Validation
	// =========================================================================

	/** Validate tool args against tool schema before execution */
	bool ValidateToolArgs(const FString& ToolName, const TSharedPtr<FJsonObject>& ToolArgs, FString& OutError) const;

	/**
	 * Strict type validation for canonical args.
	 * Rejects: string-as-number, NaN, Inf, absurd values (|x| > 1e9).
	 * Populates InputArgTypes for audit trail.
	 */
	bool ValidateCanonicalTypes(
		const TSharedPtr<FJsonObject>& Args,
		TArray<FArgTypeRecord>& OutArgTypes,
		FString& OutError) const;

	// =========================================================================
	// Tool Selection Policy
	// =========================================================================

	/**
	 * Tool selection result with full audit trail.
	 */
	struct FToolSelectionResult
	{
		FString SelectedTool;
		TArray<FToolTrustSnapshot> CandidateTools;
		TArray<EToolSelectionReason> SelectionReasons;
		bool bLossySelection = false;  // True if selected tool drops fields
		bool bLossyAllowed = false;    // True if lossy was only option
	};

	/**
	 * Select best tool for a capability given current trust state.
	 * Considers: trust penalties, expressiveness match, dropped fields.
	 * Returns full audit trail for proof bundle.
	 */
	FToolSelectionResult SelectBestToolWithAudit(
		const FString& CapabilityId,
		const TSharedPtr<FJsonObject>& CanonicalArgs,
		const TArray<FString>& ExplicitlyBlockedTools) const;

	/**
	 * Legacy wrapper - returns just the tool name.
	 */
	FString SelectBestTool(
		const FString& CapabilityId,
		const TSharedPtr<FJsonObject>& CanonicalArgs,
		const TArray<FString>& BlockedTools) const;

private:
	FCapabilityAdapter();

	// Capability → Tools mapping
	TMap<FString, TArray<FString>> CapabilityToTools;

	// Tool → Capability mapping (reverse lookup)
	TMap<FString, FString> ToolToCapability;

	void RegisterCapabilities();
};

// =========================================================================
// Proof Bundle Extension for Adapter Transparency
// =========================================================================

/**
 * Selection reason codes - why a tool was chosen or rejected.
 */
enum class EToolSelectionReason : uint8
{
	Preferred,              // First choice, no issues
	BlockedByTrust,         // Rejected due to trust penalty
	MoreExpressive,         // Chosen because it preserves more fields
	DroppedFieldsMinimized, // Chosen to minimize data loss
	OnlyOption,             // Only available tool for capability
	LossyButAllowed,        // Lossy but acceptable (no better option)
	RejectedLossy,          // Rejected because would drop required fields
};

/**
 * Trust snapshot for a candidate tool at selection time.
 */
struct RIFTBORNAI_API FToolTrustSnapshot
{
	FString ToolName;
	float Penalty = 0.0f;
	bool bBlocked = false;
	float EffectiveSuccessRate = 1.0f;
	TArray<EToolSelectionReason> ReasonCodes;

	TSharedPtr<FJsonObject> ToJson() const
	{
		TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
		Json->SetStringField(TEXT("tool_name"), ToolName);
		Json->SetNumberField(TEXT("penalty"), Penalty);
		Json->SetBoolField(TEXT("blocked"), bBlocked);
		Json->SetNumberField(TEXT("effective_success_rate"), EffectiveSuccessRate);

		TArray<TSharedPtr<FJsonValue>> ReasonsArray;
		for (EToolSelectionReason Reason : ReasonCodes)
		{
			FString ReasonStr;
			switch (Reason)
			{
				case EToolSelectionReason::Preferred: ReasonStr = TEXT("Preferred"); break;
				case EToolSelectionReason::BlockedByTrust: ReasonStr = TEXT("BlockedByTrust"); break;
				case EToolSelectionReason::MoreExpressive: ReasonStr = TEXT("MoreExpressive"); break;
				case EToolSelectionReason::DroppedFieldsMinimized: ReasonStr = TEXT("DroppedFieldsMinimized"); break;
				case EToolSelectionReason::OnlyOption: ReasonStr = TEXT("OnlyOption"); break;
				case EToolSelectionReason::LossyButAllowed: ReasonStr = TEXT("LossyButAllowed"); break;
				case EToolSelectionReason::RejectedLossy: ReasonStr = TEXT("RejectedLossy"); break;
			}
			ReasonsArray.Add(MakeShared<FJsonValueString>(ReasonStr));
		}
		Json->SetArrayField(TEXT("reason_codes"), ReasonsArray);

		return Json;
	}
};

/**
 * Input argument type record for coercion auditing.
 */
struct RIFTBORNAI_API FArgTypeRecord
{
	FString Key;
	FString OriginalType;  // "number", "string", "bool", "null"
	FString CoercedType;   // What it became after parsing
	bool bCoercionOccurred = false;
	bool bValidationFailed = false;
	FString ValidationError;

	TSharedPtr<FJsonObject> ToJson() const
	{
		TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
		Json->SetStringField(TEXT("key"), Key);
		Json->SetStringField(TEXT("original_type"), OriginalType);
		Json->SetStringField(TEXT("coerced_type"), CoercedType);
		Json->SetBoolField(TEXT("coercion_occurred"), bCoercionOccurred);
		if (bValidationFailed)
		{
			Json->SetBoolField(TEXT("validation_failed"), true);
			Json->SetStringField(TEXT("validation_error"), ValidationError);
		}
		return Json;
	}
};

/**
 * Adapter proof record - included in step proof bundle.
 *
 * INVARIANT: This record must contain enough information to:
 * 1. Reproduce the exact tool selection decision
 * 2. Audit why alternatives were rejected
 * 3. Detect silent type coercion
 * 4. Verify lossy adaptation was intentional
 */
struct RIFTBORNAI_API FAdapterProof
{
	// === Core identity ===
	FString CapabilityId;
	FString SourceTool;
	FString SelectedTool;
	FString AdapterVersion = TEXT("1.1.0");

	// === Canonical representation ===
	TSharedPtr<FJsonObject> CanonicalArgs;
	TSharedPtr<FJsonObject> ToolArgs;

	// === Selection audit trail ===
	TArray<FToolTrustSnapshot> CandidateTools;  // All tools considered, with trust state
	TArray<EToolSelectionReason> SelectionReasons;  // Why SelectedTool was chosen

	// === Data integrity ===
	TArray<FArgTypeRecord> InputArgTypes;  // Type of each input arg before coercion
	TArray<FString> DroppedFields;  // Fields that couldn't be represented
	bool bLossyAdaptation = false;  // True if any semantic data was lost
	bool bLossyAllowed = false;  // True if lossy was intentional (no better option)

	// === Validation state ===
	bool bInputValidationPassed = true;
	bool bOutputValidationPassed = true;
	FString ValidationError;

	TSharedPtr<FJsonObject> ToJson() const
	{
		TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();

		// Core identity
		Json->SetStringField(TEXT("capability_id"), CapabilityId);
		Json->SetStringField(TEXT("source_tool"), SourceTool);
		Json->SetStringField(TEXT("selected_tool"), SelectedTool);
		Json->SetStringField(TEXT("adapter_version"), AdapterVersion);

		// Canonical representation
		if (CanonicalArgs.IsValid())
		{
			Json->SetObjectField(TEXT("canonical_args"), CanonicalArgs);
		}
		if (ToolArgs.IsValid())
		{
			Json->SetObjectField(TEXT("tool_args"), ToolArgs);
		}

		// Selection audit trail
		TArray<TSharedPtr<FJsonValue>> CandidatesArray;
		for (const FToolTrustSnapshot& Candidate : CandidateTools)
		{
			CandidatesArray.Add(MakeShared<FJsonValueObject>(Candidate.ToJson()));
		}
		Json->SetArrayField(TEXT("candidate_tools"), CandidatesArray);

		TArray<TSharedPtr<FJsonValue>> ReasonsArray;
		for (EToolSelectionReason Reason : SelectionReasons)
		{
			FString ReasonStr;
			switch (Reason)
			{
				case EToolSelectionReason::Preferred: ReasonStr = TEXT("Preferred"); break;
				case EToolSelectionReason::BlockedByTrust: ReasonStr = TEXT("BlockedByTrust"); break;
				case EToolSelectionReason::MoreExpressive: ReasonStr = TEXT("MoreExpressive"); break;
				case EToolSelectionReason::DroppedFieldsMinimized: ReasonStr = TEXT("DroppedFieldsMinimized"); break;
				case EToolSelectionReason::OnlyOption: ReasonStr = TEXT("OnlyOption"); break;
				case EToolSelectionReason::LossyButAllowed: ReasonStr = TEXT("LossyButAllowed"); break;
				case EToolSelectionReason::RejectedLossy: ReasonStr = TEXT("RejectedLossy"); break;
			}
			ReasonsArray.Add(MakeShared<FJsonValueString>(ReasonStr));
		}
		Json->SetArrayField(TEXT("selection_reasons"), ReasonsArray);

		// Data integrity
		TArray<TSharedPtr<FJsonValue>> ArgTypesArray;
		for (const FArgTypeRecord& ArgType : InputArgTypes)
		{
			ArgTypesArray.Add(MakeShared<FJsonValueObject>(ArgType.ToJson()));
		}
		Json->SetArrayField(TEXT("input_arg_types"), ArgTypesArray);

		TArray<TSharedPtr<FJsonValue>> DroppedArray;
		for (const FString& Field : DroppedFields)
		{
			DroppedArray.Add(MakeShared<FJsonValueString>(Field));
		}
		Json->SetArrayField(TEXT("dropped_fields"), DroppedArray);

		Json->SetBoolField(TEXT("lossy_adaptation"), bLossyAdaptation);
		Json->SetBoolField(TEXT("lossy_allowed"), bLossyAllowed);

		// Validation state
		Json->SetBoolField(TEXT("input_validation_passed"), bInputValidationPassed);
		Json->SetBoolField(TEXT("output_validation_passed"), bOutputValidationPassed);
		if (!ValidationError.IsEmpty())
		{
			Json->SetStringField(TEXT("validation_error"), ValidationError);
		}

		return Json;
	}
};
