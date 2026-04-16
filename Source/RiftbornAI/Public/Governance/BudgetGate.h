// Copyright 2026 RiftbornAI. All Rights Reserved.
//
// BudgetGate — Phase 1 of the judgment layer.
//
// Turns post-hoc scorers (MaxActorCount, BoundedPosition, etc.) into
// pre-execution gates. Runs BEFORE any tool dispatches. If a constraint
// fails, the tool never executes.
//
// Wiring: RiftbornExecuteGovernedTool() calls FBudgetGate::Get().Evaluate(Call)
//         at the very top of the function — before any dispatch path.
//
// Constraints are loaded from Config/tiers/budget_constraints.json at module
// startup (FRiftbornAIModule::StartupModule).
//
// NOTE: This is header-only (all inline). Splitting into .cpp files would
// require a full rebuild — live coding can only recompile existing .cpp files.
// When the plugin next does a full UBT rebuild, this should be refactored
// into Private/Governance/BudgetGate{,_Evaluators,_Loader}.cpp for compile
// speed and file discipline.

#pragma once

#include "CoreMinimal.h"
#include "ClaudeToolUse.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "HAL/CriticalSection.h"
#include "HAL/PlatformMisc.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include <atomic>

#if WITH_EDITOR
#include "Editor.h"
#include "Engine/Light.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#endif

/** What kind of budget is being enforced. */
enum class EBudgetConstraintKind : uint8
{
    Unknown = 0,
    MaxActorCount,
    MaxLightCount,
    BoundedPosition,
    ScaleFloor,
    SafePathsOnly,
    PropertyAllowlist,
    MaxFoliageInstancesPerCall
};

/** One typed constraint. Only the fields relevant to Kind are meaningful. */
struct FBudgetConstraint
{
    EBudgetConstraintKind Kind = EBudgetConstraintKind::Unknown;

    int32 IntLimit = 0;
    double MinX = 0.0, MaxX = 0.0;
    double MinY = 0.0, MaxY = 0.0;
    double MinZ = 0.0, MaxZ = 0.0;
    double FloatLimit = 0.0;
    FString StringFilter;
    TArray<FString> AllowedStrings;

    FString Describe() const
    {
        switch (Kind)
        {
            case EBudgetConstraintKind::MaxActorCount:
                return FString::Printf(TEXT("MaxActorCount limit=%d filter='%s'"), IntLimit, *StringFilter);
            case EBudgetConstraintKind::MaxLightCount:
                return FString::Printf(TEXT("MaxLightCount limit=%d"), IntLimit);
            case EBudgetConstraintKind::BoundedPosition:
                return FString::Printf(
                    TEXT("BoundedPosition x[%.0f,%.0f] y[%.0f,%.0f] z[%.0f,%.0f]"),
                    MinX, MaxX, MinY, MaxY, MinZ, MaxZ);
            case EBudgetConstraintKind::ScaleFloor:
                return FString::Printf(TEXT("ScaleFloor >=%.3f"), FloatLimit);
            case EBudgetConstraintKind::SafePathsOnly:
                return FString::Printf(TEXT("SafePathsOnly prefixes=%d"), AllowedStrings.Num());
            case EBudgetConstraintKind::PropertyAllowlist:
                return FString::Printf(TEXT("PropertyAllowlist names=%d"), AllowedStrings.Num());
            case EBudgetConstraintKind::MaxFoliageInstancesPerCall:
                return FString::Printf(TEXT("MaxFoliageInstancesPerCall limit=%d"), IntLimit);
            default:
                return TEXT("Unknown");
        }
    }
};

/** Result of a BudgetGate evaluation. */
struct FBudgetEvaluation
{
    bool bPass = true;
    FString Reason;
    EBudgetConstraintKind ViolatedKind = EBudgetConstraintKind::Unknown;
    FString ViolatedToolName;

    static FBudgetEvaluation Pass() { return FBudgetEvaluation(); }

    static FBudgetEvaluation Reject(const FString& R, EBudgetConstraintKind K, const FString& Tool)
    {
        FBudgetEvaluation E;
        E.bPass = false;
        E.Reason = R;
        E.ViolatedKind = K;
        E.ViolatedToolName = Tool;
        return E;
    }
};

/**
 * Singleton pre-execution budget gate.
 *
 * Thread-safe. Scene-dependent checks (MaxActorCount, MaxLightCount) short-circuit
 * to Pass() when not on the game thread — they are advisory off the game thread.
 */
class FBudgetGate
{
public:
    static FBudgetGate& Get()
    {
        static FBudgetGate Instance;
        return Instance;
    }

    void Initialize()
    {
        FScopeLock ScopeLock(&Lock);
        if (bInitialized) return;

        const FString EnvFlag = FPlatformMisc::GetEnvironmentVariable(TEXT("RIFTBORN_BUDGET_GATE_DISABLED"));
        if (EnvFlag == TEXT("1") || EnvFlag.Equals(TEXT("true"), ESearchCase::IgnoreCase))
        {
            bEnabled = false;
            UE_LOG(LogTemp, Warning,
                TEXT("[BudgetGate] DISABLED via RIFTBORN_BUDGET_GATE_DISABLED=%s"), *EnvFlag);
        }

        const FString PluginCfg = FPaths::ProjectPluginsDir()
            / TEXT("RiftbornAI/Config/tiers/budget_constraints.json");
        const FString ProjectCfg = FPaths::ProjectConfigDir()
            / TEXT("RiftbornAI/budget_constraints.json");

        FString PathUsed;
        if (FPaths::FileExists(PluginCfg)) PathUsed = PluginCfg;
        else if (FPaths::FileExists(ProjectCfg)) PathUsed = ProjectCfg;

        if (PathUsed.IsEmpty() || !LoadConstraintsFile(PathUsed))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[BudgetGate] no constraints file found (checked %s, %s) — gate is no-op"),
                *PluginCfg, *ProjectCfg);
        }
        else
        {
            UE_LOG(LogTemp, Log,
                TEXT("[BudgetGate] loaded constraints for %d tools from %s"),
                ToolConstraints.Num(), *PathUsed);
        }

        bInitialized = true;
    }

    bool IsEnabled() const { return bEnabled; }

    bool HasConstraints(const FString& ToolName) const
    {
        FScopeLock ScopeLock(&Lock);
        const TArray<FBudgetConstraint>* Found = ToolConstraints.Find(ToolName);
        return Found && Found->Num() > 0;
    }

    void RegisterConstraint(const FString& ToolName, const FBudgetConstraint& Constraint)
    {
        FScopeLock ScopeLock(&Lock);
        ToolConstraints.FindOrAdd(ToolName).Add(Constraint);
    }

    void Reset()
    {
        FScopeLock ScopeLock(&Lock);
        ToolConstraints.Empty();
        AllowedCount.store(0, std::memory_order_relaxed);
        BlockedCount.store(0, std::memory_order_relaxed);
        bInitialized = false;
        bEnabled = true;
    }

    void GetStatistics(int64& OutAllowed, int64& OutBlocked) const
    {
        OutAllowed = AllowedCount.load(std::memory_order_relaxed);
        OutBlocked = BlockedCount.load(std::memory_order_relaxed);
    }

    FBudgetEvaluation Evaluate(const FClaudeToolCall& Call) const
    {
        if (!bEnabled)
        {
            AllowedCount.fetch_add(1, std::memory_order_relaxed);
            return FBudgetEvaluation::Pass();
        }

        TArray<FBudgetConstraint> Constraints;
        {
            FScopeLock ScopeLock(&Lock);
            const TArray<FBudgetConstraint>* Found = ToolConstraints.Find(Call.ToolName);
            if (!Found || Found->Num() == 0)
            {
                AllowedCount.fetch_add(1, std::memory_order_relaxed);
                return FBudgetEvaluation::Pass();
            }
            Constraints = *Found;
        }

        for (const FBudgetConstraint& C : Constraints)
        {
            const FBudgetEvaluation E = EvaluateConstraint(C, Call);
            if (!E.bPass)
            {
                BlockedCount.fetch_add(1, std::memory_order_relaxed);
                UE_LOG(LogTemp, Warning,
                    TEXT("[BudgetGate] REJECT tool=%s constraint=%s reason=%s"),
                    *Call.ToolName, *C.Describe(), *E.Reason);
                return E;
            }
        }

        AllowedCount.fetch_add(1, std::memory_order_relaxed);
        return FBudgetEvaluation::Pass();
    }

private:
    FBudgetGate() = default;

    // ----- JSON loader --------------------------------------------------------

    static EBudgetConstraintKind ParseKind(const FString& S)
    {
        if (S.Equals(TEXT("MaxActorCount"), ESearchCase::IgnoreCase)) return EBudgetConstraintKind::MaxActorCount;
        if (S.Equals(TEXT("MaxLightCount"), ESearchCase::IgnoreCase)) return EBudgetConstraintKind::MaxLightCount;
        if (S.Equals(TEXT("BoundedPosition"), ESearchCase::IgnoreCase)) return EBudgetConstraintKind::BoundedPosition;
        if (S.Equals(TEXT("ScaleFloor"), ESearchCase::IgnoreCase)) return EBudgetConstraintKind::ScaleFloor;
        if (S.Equals(TEXT("SafePathsOnly"), ESearchCase::IgnoreCase)) return EBudgetConstraintKind::SafePathsOnly;
        if (S.Equals(TEXT("PropertyAllowlist"), ESearchCase::IgnoreCase)) return EBudgetConstraintKind::PropertyAllowlist;
        if (S.Equals(TEXT("MaxFoliageInstancesPerCall"), ESearchCase::IgnoreCase)) return EBudgetConstraintKind::MaxFoliageInstancesPerCall;
        return EBudgetConstraintKind::Unknown;
    }

    static void ReadStringArray(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Key, TArray<FString>& Out)
    {
        const TArray<TSharedPtr<FJsonValue>>* Arr = nullptr;
        if (!Obj->TryGetArrayField(Key, Arr) || !Arr) return;
        Out.Reserve(Arr->Num());
        for (const TSharedPtr<FJsonValue>& V : *Arr)
        {
            FString S;
            if (V.IsValid() && V->TryGetString(S) && !S.IsEmpty()) Out.Add(S);
        }
    }

    static bool ParseConstraint(const TSharedPtr<FJsonObject>& Obj, FBudgetConstraint& Out)
    {
        FString KindStr;
        if (!Obj->TryGetStringField(TEXT("kind"), KindStr)) return false;
        Out.Kind = ParseKind(KindStr);
        if (Out.Kind == EBudgetConstraintKind::Unknown)
        {
            UE_LOG(LogTemp, Warning, TEXT("[BudgetGate] unknown constraint kind '%s'"), *KindStr);
            return false;
        }

        int32 IntVal = 0;
        if (Obj->TryGetNumberField(TEXT("limit"), IntVal)) Out.IntLimit = IntVal;

        double DVal = 0.0;
        if (Obj->TryGetNumberField(TEXT("min_x"), DVal)) Out.MinX = DVal;
        if (Obj->TryGetNumberField(TEXT("max_x"), DVal)) Out.MaxX = DVal;
        if (Obj->TryGetNumberField(TEXT("min_y"), DVal)) Out.MinY = DVal;
        if (Obj->TryGetNumberField(TEXT("max_y"), DVal)) Out.MaxY = DVal;
        if (Obj->TryGetNumberField(TEXT("min_z"), DVal)) Out.MinZ = DVal;
        if (Obj->TryGetNumberField(TEXT("max_z"), DVal)) Out.MaxZ = DVal;
        if (Obj->TryGetNumberField(TEXT("min_scale"), DVal)) Out.FloatLimit = DVal;

        Obj->TryGetStringField(TEXT("class_filter"), Out.StringFilter);

        ReadStringArray(Obj, TEXT("allowed_prefixes"), Out.AllowedStrings);
        if (Out.AllowedStrings.Num() == 0)
        {
            ReadStringArray(Obj, TEXT("allowed_properties"), Out.AllowedStrings);
        }
        return true;
    }

    bool LoadConstraintsFile(const FString& FilePath)
    {
        FString Content;
        if (!FFileHelper::LoadFileToString(Content, *FilePath)) return false;

        TSharedPtr<FJsonObject> Root;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
        if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("[BudgetGate] could not parse %s"), *FilePath);
            return false;
        }

        const TSharedPtr<FJsonObject>* ToolsObjPtr = nullptr;
        if (!Root->TryGetObjectField(TEXT("tools"), ToolsObjPtr) || !ToolsObjPtr)
        {
            UE_LOG(LogTemp, Error, TEXT("[BudgetGate] %s missing 'tools' field"), *FilePath);
            return false;
        }

        int32 TotalConstraints = 0;
        int32 ParseFailures = 0;

        for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*ToolsObjPtr)->Values)
        {
            const FString& ToolName = Pair.Key;

            const TSharedPtr<FJsonObject>* ToolEntryPtr = nullptr;
            if (!Pair.Value.IsValid() || !Pair.Value->TryGetObject(ToolEntryPtr) || !ToolEntryPtr) continue;

            const TArray<TSharedPtr<FJsonValue>>* ConstraintsArr = nullptr;
            if (!(*ToolEntryPtr)->TryGetArrayField(TEXT("constraints"), ConstraintsArr) || !ConstraintsArr) continue;

            TArray<FBudgetConstraint> Parsed;
            for (const TSharedPtr<FJsonValue>& V : *ConstraintsArr)
            {
                if (!V.IsValid()) continue;
                const TSharedPtr<FJsonObject>* CObj = nullptr;
                if (!V->TryGetObject(CObj) || !CObj) continue;

                FBudgetConstraint C;
                if (ParseConstraint(*CObj, C))
                {
                    Parsed.Add(C);
                    ++TotalConstraints;
                }
                else
                {
                    ++ParseFailures;
                }
            }

            if (Parsed.Num() > 0) ToolConstraints.Add(ToolName, MoveTemp(Parsed));
        }

        UE_LOG(LogTemp, Log,
            TEXT("[BudgetGate] parsed %d constraints for %d tools (%d failures) from %s"),
            TotalConstraints, ToolConstraints.Num(), ParseFailures, *FilePath);

        return TotalConstraints > 0;
    }

    // ----- argument accessors -------------------------------------------------

    static FString GetArg(const FClaudeToolCall& Call, const TCHAR* Key)
    {
        const FString* Found = Call.Arguments.Find(Key);
        return Found ? *Found : FString();
    }

    static double GetArgAsDouble(const FClaudeToolCall& Call, const TCHAR* Key, double Default = 0.0)
    {
        const FString* Found = Call.Arguments.Find(Key);
        if (!Found || Found->IsEmpty()) return Default;
        return FCString::Atod(**Found);
    }

    static int32 GetArgAsInt(const FClaudeToolCall& Call, const TCHAR* Key, int32 Default = 0)
    {
        const FString* Found = Call.Arguments.Find(Key);
        if (!Found || Found->IsEmpty()) return Default;
        return FCString::Atoi(**Found);
    }

    static void BuildPropertyAliasCandidates(const FString& RequestedName, TArray<FString>& OutCandidates)
    {
        const FString Trimmed = RequestedName.TrimStartAndEnd();
        if (Trimmed.IsEmpty())
        {
            return;
        }

        OutCandidates.AddUnique(Trimmed);

        if (Trimmed.Len() > 1 && Trimmed[0] == TCHAR('b') && FChar::IsUpper(Trimmed[1]))
        {
            OutCandidates.AddUnique(Trimmed.Mid(1));
        }
        else if (FChar::IsUpper(Trimmed[0]))
        {
            OutCandidates.AddUnique(TEXT("b") + Trimmed);
        }
    }

    static bool MatchesAllowedPropertyName(const FString& RequestedName, const FString& AllowedName)
    {
        TArray<FString> Candidates;
        BuildPropertyAliasCandidates(RequestedName, Candidates);
        for (const FString& Candidate : Candidates)
        {
            if (Candidate.Equals(AllowedName, ESearchCase::IgnoreCase))
            {
                return true;
            }
        }
        return false;
    }

#if WITH_EDITOR
    static UWorld* GetEditorWorldSafe()
    {
        if (!GEditor) return nullptr;
        return GEditor->GetEditorWorldContext().World();
    }

    static int32 CountActors(UWorld* World, const FString& ClassFilter)
    {
        if (!World) return 0;
        int32 Count = 0;
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            AActor* A = *It;
            if (!A) continue;
            if (ClassFilter.IsEmpty())
            {
                ++Count;
                continue;
            }
            const FString ClassName = A->GetClass() ? A->GetClass()->GetName() : FString();
            if (ClassName.Contains(ClassFilter, ESearchCase::IgnoreCase))
            {
                ++Count;
            }
        }
        return Count;
    }

    static int32 CountLights(UWorld* World)
    {
        if (!World) return 0;
        int32 Count = 0;
        for (TActorIterator<ALight> It(World); It; ++It)
        {
            if (*It) ++Count;
        }
        return Count;
    }
#endif

    // ----- constraint evaluator ----------------------------------------------

    FBudgetEvaluation EvaluateConstraint(const FBudgetConstraint& C, const FClaudeToolCall& Call) const
    {
        switch (C.Kind)
        {
            case EBudgetConstraintKind::MaxActorCount:
            {
#if WITH_EDITOR
                if (!IsInGameThread()) return FBudgetEvaluation::Pass();
                UWorld* World = GetEditorWorldSafe();
                if (!World) return FBudgetEvaluation::Pass();
                const int32 Current = CountActors(World, C.StringFilter);
                if (Current >= C.IntLimit)
                {
                    const FString FilterNote = C.StringFilter.IsEmpty()
                        ? FString()
                        : FString::Printf(TEXT(" for filter '%s'"), *C.StringFilter);
                    return FBudgetEvaluation::Reject(
                        FString::Printf(
                            TEXT("Actor count (%d) at or over budget (%d)%s"),
                            Current, C.IntLimit, *FilterNote),
                        C.Kind, Call.ToolName);
                }
#endif
                return FBudgetEvaluation::Pass();
            }

            case EBudgetConstraintKind::MaxLightCount:
            {
#if WITH_EDITOR
                if (!IsInGameThread()) return FBudgetEvaluation::Pass();
                UWorld* World = GetEditorWorldSafe();
                if (!World) return FBudgetEvaluation::Pass();
                const int32 Current = CountLights(World);
                if (Current >= C.IntLimit)
                {
                    return FBudgetEvaluation::Reject(
                        FString::Printf(TEXT("Light count (%d) at or over budget (%d)"), Current, C.IntLimit),
                        C.Kind, Call.ToolName);
                }
#endif
                return FBudgetEvaluation::Pass();
            }

            case EBudgetConstraintKind::BoundedPosition:
            {
                const bool bHasX = Call.Arguments.Contains(TEXT("x"));
                const bool bHasY = Call.Arguments.Contains(TEXT("y"));
                const bool bHasZ = Call.Arguments.Contains(TEXT("z"));
                if (!bHasX && !bHasY && !bHasZ) return FBudgetEvaluation::Pass();

                if (bHasX)
                {
                    const double X = GetArgAsDouble(Call, TEXT("x"));
                    if (X < C.MinX || X > C.MaxX)
                    {
                        return FBudgetEvaluation::Reject(
                            FString::Printf(TEXT("X=%.1f outside bounds [%.0f,%.0f]"), X, C.MinX, C.MaxX),
                            C.Kind, Call.ToolName);
                    }
                }
                if (bHasY)
                {
                    const double Y = GetArgAsDouble(Call, TEXT("y"));
                    if (Y < C.MinY || Y > C.MaxY)
                    {
                        return FBudgetEvaluation::Reject(
                            FString::Printf(TEXT("Y=%.1f outside bounds [%.0f,%.0f]"), Y, C.MinY, C.MaxY),
                            C.Kind, Call.ToolName);
                    }
                }
                if (bHasZ)
                {
                    const double Z = GetArgAsDouble(Call, TEXT("z"));
                    if (Z < C.MinZ || Z > C.MaxZ)
                    {
                        return FBudgetEvaluation::Reject(
                            FString::Printf(TEXT("Z=%.1f outside bounds [%.0f,%.0f]"), Z, C.MinZ, C.MaxZ),
                            C.Kind, Call.ToolName);
                    }
                }
                return FBudgetEvaluation::Pass();
            }

            case EBudgetConstraintKind::ScaleFloor:
            {
                const bool bHasUniform = Call.Arguments.Contains(TEXT("scale"));
                const bool bHasPerAxis =
                    Call.Arguments.Contains(TEXT("scale_x")) ||
                    Call.Arguments.Contains(TEXT("scale_y")) ||
                    Call.Arguments.Contains(TEXT("scale_z"));
                if (!bHasUniform && !bHasPerAxis) return FBudgetEvaluation::Pass();

                const double Uniform = GetArgAsDouble(Call, TEXT("scale"), 1.0);
                const double SX = GetArgAsDouble(Call, TEXT("scale_x"), Uniform);
                const double SY = GetArgAsDouble(Call, TEXT("scale_y"), Uniform);
                const double SZ = GetArgAsDouble(Call, TEXT("scale_z"), Uniform);
                const double Min = FMath::Min3(SX, SY, SZ);
                if (Min < C.FloatLimit)
                {
                    return FBudgetEvaluation::Reject(
                        FString::Printf(TEXT("Scale %.3f below floor %.3f"), Min, C.FloatLimit),
                        C.Kind, Call.ToolName);
                }
                return FBudgetEvaluation::Pass();
            }

            case EBudgetConstraintKind::SafePathsOnly:
            {
                FString Path = GetArg(Call, TEXT("destination_path"));
                if (Path.IsEmpty()) Path = GetArg(Call, TEXT("destination"));
                if (Path.IsEmpty()) Path = GetArg(Call, TEXT("path"));
                if (Path.IsEmpty()) Path = GetArg(Call, TEXT("asset_path"));
                if (Path.IsEmpty()) return FBudgetEvaluation::Pass();

                // Slash-tolerant prefix match. Naïve StartsWith fails when the
                // configured prefix has a trailing slash and the tool argument
                // doesn't (e.g. allow="/Game/Maps/" vs path="/Game/Maps"). We
                // normalize by stripping the trailing slash and accepting both
                // exact match AND `path` followed by `/` (folder boundary).
                for (const FString& RawPrefix : C.AllowedStrings)
                {
                    FString Prefix = RawPrefix;
                    while (Prefix.EndsWith(TEXT("/"), ESearchCase::CaseSensitive))
                    {
                        Prefix.LeftChopInline(1, EAllowShrinking::No);
                    }

                    // Exact match (e.g. asking to operate on the folder itself).
                    if (Path.Equals(Prefix, ESearchCase::IgnoreCase)) return FBudgetEvaluation::Pass();

                    // Sub-path match (e.g. /Game/Maps/MyLevel under /Game/Maps).
                    // Adding the trailing slash to `Prefix` here prevents
                    // /Game/MapsExtra accidentally matching /Game/Maps.
                    const FString PrefixWithSlash = Prefix + TEXT("/");
                    if (Path.StartsWith(PrefixWithSlash, ESearchCase::IgnoreCase)) return FBudgetEvaluation::Pass();
                }

                const FString Prefixes = FString::Join(C.AllowedStrings, TEXT(", "));
                return FBudgetEvaluation::Reject(
                    FString::Printf(TEXT("Path '%s' not in safe-paths allowlist: [%s]"), *Path, *Prefixes),
                    C.Kind, Call.ToolName);
            }

            case EBudgetConstraintKind::PropertyAllowlist:
            {
                FString PropName = GetArg(Call, TEXT("property"));
                if (PropName.IsEmpty()) PropName = GetArg(Call, TEXT("property_name"));
                if (PropName.IsEmpty()) return FBudgetEvaluation::Pass();

                // Normalize common AActor short-names → underlying scene
                // component property names so the model can use either form.
                // SetActorScale3D/Location/Rotation map to the SceneComponent
                // RelativeXxx UPROPERTY internally; treat them as equivalent
                // for allowlisting purposes.
                FString NormalizedName = PropName;
                if (NormalizedName.Equals(TEXT("ActorScale3D"), ESearchCase::IgnoreCase)) NormalizedName = TEXT("RelativeScale3D");
                else if (NormalizedName.Equals(TEXT("ActorLocation"), ESearchCase::IgnoreCase)) NormalizedName = TEXT("RelativeLocation");
                else if (NormalizedName.Equals(TEXT("ActorRotation"), ESearchCase::IgnoreCase)) NormalizedName = TEXT("RelativeRotation");

                for (const FString& Allowed : C.AllowedStrings)
                {
                    if (MatchesAllowedPropertyName(PropName, Allowed)) return FBudgetEvaluation::Pass();
                    if (MatchesAllowedPropertyName(NormalizedName, Allowed)) return FBudgetEvaluation::Pass();
                }

                const FString Allowlist = FString::Join(C.AllowedStrings, TEXT(", "));
                return FBudgetEvaluation::Reject(
                    FString::Printf(TEXT("Property '%s' not in allowlist: [%s]"), *PropName, *Allowlist),
                    C.Kind, Call.ToolName);
            }

            case EBudgetConstraintKind::MaxFoliageInstancesPerCall:
            {
                const int32 Count = GetArgAsInt(Call, TEXT("count"), 1);
                if (Count > C.IntLimit)
                {
                    return FBudgetEvaluation::Reject(
                        FString::Printf(TEXT("Foliage count (%d) exceeds per-call limit (%d)"),
                            Count, C.IntLimit),
                        C.Kind, Call.ToolName);
                }
                return FBudgetEvaluation::Pass();
            }

            case EBudgetConstraintKind::Unknown:
            default:
                return FBudgetEvaluation::Pass();
        }
    }

    mutable FCriticalSection Lock;
    TMap<FString, TArray<FBudgetConstraint>> ToolConstraints;
    bool bInitialized = false;
    bool bEnabled = true;

    mutable std::atomic<int64> AllowedCount{0};
    mutable std::atomic<int64> BlockedCount{0};
};
