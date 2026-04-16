// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RiftbornDataAsset.generated.h"

// ============================================================================
// DATA ASSET TYPES
// ============================================================================

/** Data table column type */
UENUM(BlueprintType)
enum class EDataTableColumnType : uint8
{
    String,
    Integer,
    Float,
    Boolean,
    Vector,
    Rotator,
    Name,
    Asset,
    Custom
};

/** Curve table type */
UENUM(BlueprintType)
enum class ECurveTableType : uint8
{
    SimpleCurve,        // Single float curve
    VectorCurve,        // 3D vector curve
    LinearColorCurve,   // RGBA color curve
    Custom
};

/** Data asset category */
UENUM(BlueprintType)
enum class EDataAssetCategory : uint8
{
    GameplayData,       // Abilities, items, etc.
    Configuration,      // Settings, balancing
    Progression,        // XP, leveling, unlocks
    Economy,           // Prices, rewards, loot
    Narrative,         // Dialogue, quests
    Custom
};

// ============================================================================
// DATA TABLE SPECS
// ============================================================================

/** Data table column specification */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FDataTableColumn
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ColumnName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EDataTableColumnType Type = EDataTableColumnType::String;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString DefaultValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Description;
};

/** Data table row specification */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FDataTableRow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString RowName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TMap<FString, FString> ColumnValues;
};

/** Complete data table specification */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FDataTableSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString StructName;  // For custom struct types

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FDataTableColumn> Columns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FDataTableRow> Rows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Description;
};

// ============================================================================
// CURVE TABLE SPECS
// ============================================================================

/** Curve key point */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FCurveKeyPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Time = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Value = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FVector VectorValue = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FLinearColor ColorValue = FLinearColor::White;
};

/** Curve specification */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FCurveSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString CurveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    ECurveTableType Type = ECurveTableType::SimpleCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FCurveKeyPoint> KeyPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Description;
};

/** Curve table specification */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FCurveTableSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FCurveSpec> Curves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Description;
};

// ============================================================================
// DATA ASSET SPECS
// ============================================================================

/** Data asset property specification */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FDataAssetProperty
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString PropertyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString PropertyType;  // int32, float, FString, TArray, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString DefaultValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bEditAnywhere = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Category = TEXT("Data");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Tooltip;
};

/** Custom data asset specification */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FDataAssetSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EDataAssetCategory Category = EDataAssetCategory::GameplayData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ParentClass = TEXT("UPrimaryDataAsset");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FDataAssetProperty> Properties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Description;
};

// ============================================================================
// KNOWLEDGE BASE
// ============================================================================

/**
 * Knowledge base for data assets - contains presets and templates
 */
class RIFTBORNAI_API FDataAssetKnowledgeBase
{
public:
    FDataAssetKnowledgeBase();
    static FDataAssetKnowledgeBase& Get();

    const FDataTableSpec* GetDataTablePreset(const FString& PresetName) const;
    TArray<FString> GetDataTablePresetNames() const;

    const FCurveTableSpec* GetCurveTablePreset(const FString& PresetName) const;
    TArray<FString> GetCurveTablePresetNames() const;

    const FDataAssetSpec* GetDataAssetPreset(const FString& PresetName) const;
    TArray<FString> GetDataAssetPresetNames() const;

private:
    TMap<FString, FDataTableSpec> DataTablePresets;
    TMap<FString, FCurveTableSpec> CurveTablePresets;
    TMap<FString, FDataAssetSpec> DataAssetPresets;
};

// ============================================================================
// DATA ASSET FACTORY SYSTEM
// ============================================================================

/**
 * Data asset factory - generates data tables, curves, and custom data assets
 */
struct RIFTBORNAI_API FDataAssetFactory
{
    // Design from descriptions
    static FDataTableSpec DesignDataTableFromDescription(const FString& Description);
    static FCurveTableSpec DesignCurveTableFromDescription(const FString& Description);
    static FDataAssetSpec DesignDataAssetFromDescription(const FString& Description);

    // Parsing helpers
    static EDataTableColumnType ParseColumnType(const FString& Text);
    static ECurveTableType ParseCurveType(const FString& Text);
    static EDataAssetCategory ParseCategory(const FString& Text);

    // Extraction
    static TArray<FString> ExtractColumnNames(const FString& Text);
    static TArray<FString> ExtractPropertyNames(const FString& Text);
    static int32 ExtractRowCount(const FString& Text);

    // CSV Import
    static FDataTableSpec ImportCSVToDataTable(const FString& CSVContent, const FString& TableName);
    static FString ParseCSVRow(const FString& Row, TArray<FString>& OutValues);

    // Code generation
    static FString GenerateDataTableSetupInstructions(const FDataTableSpec& Spec);
    static FString GenerateCurveTableSetupInstructions(const FCurveTableSpec& Spec);
    static FString GenerateDataAssetCode(const FDataAssetSpec& Spec);
    static FString GenerateStructCode(const FDataTableSpec& Spec);

    // JSON serialization
    static FString DataTableSpecToJson(const FDataTableSpec& Spec);
    static FDataTableSpec DataTableSpecFromJson(const FString& Json);
    static FString CurveTableSpecToJson(const FCurveTableSpec& Spec);
    static FCurveTableSpec CurveTableSpecFromJson(const FString& Json);

    // Utility
    static FString GetColumnTypeName(EDataTableColumnType Type);
    static FString GetCurveTypeName(ECurveTableType Type);
    static FString GetCategoryName(EDataAssetCategory Category);
};
