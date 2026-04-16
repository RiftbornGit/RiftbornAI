// Copyright 2026 RiftbornAI. All Rights Reserved.
//
// ArtDirectionBounds — Phase 6 of the judgment layer.
//
// Compiles art-direction intent into measurable numeric rules.
// GameDesignPresets.json continues to hold descriptive strings; this file
// carries the bounds a critic can actually test against (light intensity,
// fog density, roughness/metallic bands, saturation, bloom).
//
// Loaded from Config/tiers/art_direction_bounds.json at module startup.
// Existing audit_material_consistency and scene-quality tools can read
// these ranges via FArtDirectionBounds::Get() instead of hardcoded constants.
// Splits later; header-only for live coding.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "HAL/CriticalSection.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

/** A single inclusive numeric range. */
struct FArtRange
{
    double Min = 0.0;
    double Max = 0.0;

    bool IsSet() const { return Max > Min; }
    bool Contains(double V) const { return V >= Min && V <= Max; }
};

/**
 * Typed bounds for one named style (e.g. "grounded_realism"). Fields left at
 * default {0, 0} mean "no bound configured" and should be skipped by critics.
 */
struct FArtDirectionStyle
{
    FString Description;
    FArtRange LightIntensityLumens;
    FArtRange DirectionalLightIntensityLux;
    FArtRange SkyLightIntensity;
    FArtRange FogDensity;
    FArtRange ExposureCompensation;
    FArtRange RoughnessTypical;
    FArtRange MetallicTypical;
    FArtRange SaturationPostProcess;
    FArtRange BloomIntensity;
};

/** Outcome of a bounds check for a single metric + value. */
struct FArtViolation
{
    FString StyleName;
    FString MetricName;
    double ActualValue = 0.0;
    double BoundMin = 0.0;
    double BoundMax = 0.0;
    FString Detail;
};

class FArtDirectionBounds
{
public:
    static FArtDirectionBounds& Get()
    {
        static FArtDirectionBounds Instance;
        return Instance;
    }

    /** Load from Config/tiers/art_direction_bounds.json. Idempotent. */
    void Initialize()
    {
        FScopeLock ScopeLock(&Lock);
        if (bInitialized) return;

        const FString PluginCfg = FPaths::ProjectPluginsDir()
            / TEXT("RiftbornAI/Config/tiers/art_direction_bounds.json");
        const FString ProjectCfg = FPaths::ProjectConfigDir()
            / TEXT("RiftbornAI/art_direction_bounds.json");

        FString PathUsed;
        if (FPaths::FileExists(PluginCfg)) PathUsed = PluginCfg;
        else if (FPaths::FileExists(ProjectCfg)) PathUsed = ProjectCfg;

        if (!PathUsed.IsEmpty() && LoadFile(PathUsed))
        {
            UE_LOG(LogTemp, Log,
                TEXT("[ArtDirectionBounds] loaded %d styles from %s"),
                Styles.Num(), *PathUsed);
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[ArtDirectionBounds] no bounds file found (checked %s, %s)"),
                *PluginCfg, *ProjectCfg);
        }

        bInitialized = true;
    }

    bool HasStyle(const FString& Name) const
    {
        FScopeLock ScopeLock(&Lock);
        return Styles.Contains(Name);
    }

    /** Fetch a named style. Returns empty FArtDirectionStyle if not found. */
    FArtDirectionStyle GetStyle(const FString& Name) const
    {
        FScopeLock ScopeLock(&Lock);
        const FArtDirectionStyle* Found = Styles.Find(Name);
        return Found ? *Found : FArtDirectionStyle();
    }

    /** List configured style names. */
    TArray<FString> ListStyleNames() const
    {
        FScopeLock ScopeLock(&Lock);
        TArray<FString> Out;
        Styles.GetKeys(Out);
        Out.Sort();
        return Out;
    }

    /**
     * Check a set of scalar metrics against a style's bounds. Each metric
     * name must match one of the fields below. Returns empty array on pass.
     *
     * Supported metric keys:
     *   "light_intensity_lumens"
     *   "directional_light_intensity_lux"
     *   "sky_light_intensity"
     *   "fog_density"
     *   "exposure_compensation"
     *   "roughness"
     *   "metallic"
     *   "saturation_post_process"
     *   "bloom_intensity"
     */
    TArray<FArtViolation> CheckMetrics(const FString& StyleName,
                                       const TMap<FString, double>& Metrics) const
    {
        TArray<FArtViolation> Out;
        FArtDirectionStyle S;
        {
            FScopeLock ScopeLock(&Lock);
            const FArtDirectionStyle* Found = Styles.Find(StyleName);
            if (!Found)
            {
                return Out;
            }
            S = *Found;
        }

        auto Check = [&](const TCHAR* MetricKey, const FArtRange& Range)
        {
            if (!Range.IsSet()) return;
            const double* V = Metrics.Find(FString(MetricKey));
            if (!V) return;
            if (!Range.Contains(*V))
            {
                FArtViolation Vi;
                Vi.StyleName = StyleName;
                Vi.MetricName = MetricKey;
                Vi.ActualValue = *V;
                Vi.BoundMin = Range.Min;
                Vi.BoundMax = Range.Max;
                Vi.Detail = FString::Printf(
                    TEXT("%.3f outside [%.3f,%.3f] for style '%s'"),
                    *V, Range.Min, Range.Max, *StyleName);
                Out.Add(Vi);
            }
        };

        Check(TEXT("light_intensity_lumens"), S.LightIntensityLumens);
        Check(TEXT("directional_light_intensity_lux"), S.DirectionalLightIntensityLux);
        Check(TEXT("sky_light_intensity"), S.SkyLightIntensity);
        Check(TEXT("fog_density"), S.FogDensity);
        Check(TEXT("exposure_compensation"), S.ExposureCompensation);
        Check(TEXT("roughness"), S.RoughnessTypical);
        Check(TEXT("metallic"), S.MetallicTypical);
        Check(TEXT("saturation_post_process"), S.SaturationPostProcess);
        Check(TEXT("bloom_intensity"), S.BloomIntensity);

        return Out;
    }

    /** Clear styles (testing). */
    void Reset()
    {
        FScopeLock ScopeLock(&Lock);
        Styles.Empty();
        bInitialized = false;
    }

private:
    FArtDirectionBounds() = default;

    static void ReadRange(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Key, FArtRange& Out)
    {
        const TSharedPtr<FJsonObject>* RangeObj = nullptr;
        if (!Obj->TryGetObjectField(Key, RangeObj) || !RangeObj) return;
        (*RangeObj)->TryGetNumberField(TEXT("min"), Out.Min);
        (*RangeObj)->TryGetNumberField(TEXT("max"), Out.Max);
    }

    bool LoadFile(const FString& FilePath)
    {
        FString Content;
        if (!FFileHelper::LoadFileToString(Content, *FilePath)) return false;

        TSharedPtr<FJsonObject> Root;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
        if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("[ArtDirectionBounds] could not parse %s"), *FilePath);
            return false;
        }

        const TSharedPtr<FJsonObject>* StylesObj = nullptr;
        if (!Root->TryGetObjectField(TEXT("styles"), StylesObj) || !StylesObj)
        {
            UE_LOG(LogTemp, Error, TEXT("[ArtDirectionBounds] %s missing 'styles'"), *FilePath);
            return false;
        }

        int32 Loaded = 0;
        for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*StylesObj)->Values)
        {
            const TSharedPtr<FJsonObject>* StyleObj = nullptr;
            if (!Pair.Value.IsValid() || !Pair.Value->TryGetObject(StyleObj) || !StyleObj) continue;

            FArtDirectionStyle S;
            (*StyleObj)->TryGetStringField(TEXT("description"), S.Description);
            ReadRange(*StyleObj, TEXT("light_intensity_lumens"),         S.LightIntensityLumens);
            ReadRange(*StyleObj, TEXT("directional_light_intensity_lux"),S.DirectionalLightIntensityLux);
            ReadRange(*StyleObj, TEXT("sky_light_intensity"),            S.SkyLightIntensity);
            ReadRange(*StyleObj, TEXT("fog_density"),                    S.FogDensity);
            ReadRange(*StyleObj, TEXT("exposure_compensation"),          S.ExposureCompensation);
            ReadRange(*StyleObj, TEXT("roughness_typical"),              S.RoughnessTypical);
            ReadRange(*StyleObj, TEXT("metallic_typical"),               S.MetallicTypical);
            ReadRange(*StyleObj, TEXT("saturation_post_process"),        S.SaturationPostProcess);
            ReadRange(*StyleObj, TEXT("bloom_intensity"),                S.BloomIntensity);

            Styles.Add(Pair.Key, S);
            ++Loaded;
        }

        return Loaded > 0;
    }

    mutable FCriticalSection Lock;
    TMap<FString, FArtDirectionStyle> Styles;
    bool bInitialized = false;
};
