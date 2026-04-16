// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CppCodeGenerator.generated.h"

// ============================================================================
// Enums
// ============================================================================

/** C++ class type to generate */
UENUM(BlueprintType)
enum class ECppClassType : uint8
{
    Actor,              // AActor subclass
    Character,          // ACharacter subclass
    Pawn,               // APawn subclass
    Component,          // UActorComponent subclass
    SceneComponent,     // USceneComponent subclass
    Object,             // UObject subclass
    Subsystem,          // UGameInstanceSubsystem, UWorldSubsystem, etc.
    Interface,          // UInterface
    Struct,             // USTRUCT
    GameMode,           // AGameModeBase subclass
    PlayerController,   // APlayerController subclass
    AIController,       // AAIController subclass
    Widget,             // UUserWidget subclass
    AnimInstance,       // UAnimInstance subclass
    GameplayAbility     // UGameplayAbility subclass
};

/** C++ code generation template type */
UENUM(BlueprintType)
enum class ECppTemplateType : uint8
{
    Minimal,            // Bare minimum scaffolding
    Standard,           // Standard Unreal patterns
    Advanced,           // With common helper functions
    Networked,          // With replication support
    Custom              // User-defined template
};

/** Property replication mode */
UENUM(BlueprintType)
enum class EReplicationMode : uint8
{
    None,
    Replicated,
    ReplicatedUsing,    // With RepNotify
    ReplicatedCondition // With COND_OwnerOnly, etc.
};

// ============================================================================
// Structs
// ============================================================================

/** C++ property specification */
USTRUCT(BlueprintType)
struct FCppPropertySpec
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Name;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Type = TEXT("float");
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString DefaultValue;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bBlueprintReadOnly = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bEditAnywhere = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bVisibleAnywhere = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EReplicationMode ReplicationMode = EReplicationMode::None;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Category = TEXT("Default");
};

/** C++ function parameter specification */
USTRUCT(BlueprintType)
struct FCppFunctionParamSpec
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Name;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Type;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString DefaultValue;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bIsConst = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bIsReference = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bIsPointer = false;
};

/** C++ function specification */
USTRUCT(BlueprintType)
struct FCppFunctionSpec
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Name;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ReturnType = TEXT("void");
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FCppFunctionParamSpec> Parameters;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bBlueprintCallable = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bBlueprintPure = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bConst = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bVirtual = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bOverride = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Category = TEXT("Default");
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString BodyImplementation; // Optional manual implementation
};

/** C++ class specification */
USTRUCT(BlueprintType)
struct FCppClassSpec
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ClassName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    ECppClassType ClassType = ECppClassType::Actor;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ParentClass; // Optional custom parent
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ModuleName; // Defaults to FApp::GetProjectName() at runtime when empty
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FCppPropertySpec> Properties;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FCppFunctionSpec> Functions;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FString> IncludeFiles;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bBlueprintable = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bBlueprintType = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bAbstract = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    ECppTemplateType TemplateType = ECppTemplateType::Standard;
};

/** Generated C++ code files */
USTRUCT(BlueprintType)
struct FCppGeneratedCode
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString HeaderContent;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString SourceContent;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString HeaderFilePath;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString SourceFilePath;
};

// ============================================================================
// Knowledge Base
// ============================================================================

/**
 * C++ code generation knowledge base loaded from Config/CppClassTemplates.json.
 * Stores the virtual overrides, default parent class, and required include path
 * auto-applied per ECppClassType when designing a class from a description.
 */
USTRUCT()
struct RIFTBORNAI_API FCppKnowledgeBase
{
    GENERATED_BODY()

    static FCppKnowledgeBase& Get();

    /** Get common function templates (virtual overrides) for a class type. */
    TArray<FCppFunctionSpec> GetCommonFunctions(ECppClassType ClassType) const;

    /** Get default parent class name for a class type. */
    FString GetDefaultParentClass(ECppClassType ClassType) const;

    /** Get required includes for a class type. */
    TArray<FString> GetRequiredIncludes(ECppClassType ClassType) const;

private:
    void Initialize();
    bool bInitialized = false;

    TMap<ECppClassType, TArray<FCppFunctionSpec>> CommonFunctions;
    TMap<ECppClassType, FString> ParentClasses;
    TMap<ECppClassType, TArray<FString>> Includes;
};

// ============================================================================
// C++ Code Generator
// ============================================================================

/** Main C++ code generation system - natural language to C++ classes */
USTRUCT()
struct RIFTBORNAI_API FCppCodeGenerator
{
    GENERATED_BODY()
    
    // ========================================================================
    // Natural Language Parsing
    // ========================================================================
    
    /** Design C++ class from natural language description */
    static FCppClassSpec DesignClassFromDescription(const FString& Description, ECppClassType ClassType = ECppClassType::Actor, const FString& ModuleName = TEXT("MyProject"));
    
    /** Parse class type from description */
    static ECppClassType ParseClassType(const FString& Text);
    
    /** Extract properties from description */
    static TArray<FCppPropertySpec> ExtractProperties(const FString& Text);
    
    /** Extract functions from description */
    static TArray<FCppFunctionSpec> ExtractFunctions(const FString& Text);
    
    /** Parse property type from text */
    static FString ParsePropertyType(const FString& Text);
    
    // ========================================================================
    // Code Generation
    // ========================================================================
    
    /** Generate complete C++ class (header + source) */
    static FCppGeneratedCode GenerateClass(const FCppClassSpec& Spec);
    
    /** Generate and write C++ class to disk, optionally triggering compilation */
    struct FWriteResult
    {
        bool bSuccess = false;
        TArray<FString> WrittenFiles;
        FString ErrorMessage;
        bool bCompileSucceeded = false;
        float CompileTimeSeconds = 0.0f;
        TArray<FString> CompileErrors;
    };
    static FWriteResult GenerateAndWriteClass(const FCppClassSpec& Spec, bool bAutoCompile = true);
    
    /** Generate header file content */
    static FString GenerateHeader(const FCppClassSpec& InSpec);
    
    /** Generate source file content */
    static FString GenerateSource(const FCppClassSpec& Spec);
    
    /** Generate constructor implementation */
    static FString GenerateConstructor(const FCppClassSpec& Spec);
    
    /** Generate function implementation */
    static FString GenerateFunctionImplementation(const FCppClassSpec& Spec, const FCppFunctionSpec& Function);
    
    /** Generate property declarations */
    static FString GeneratePropertyDeclarations(const TArray<FCppPropertySpec>& Properties);
    
    /** Generate function declarations */
    static FString GenerateFunctionDeclarations(const TArray<FCppFunctionSpec>& Functions);
    
    /** Generate replication setup */
    static FString GenerateReplicationSetup(const FCppClassSpec& Spec);
    
    // ========================================================================
    // Utility
    // ========================================================================
    
    /** Validate class spec */
    static bool ValidateClassSpec(const FCppClassSpec& Spec, TArray<FString>& OutErrors);
    
    /** Generate file paths for class */
    static void GenerateFilePaths(const FCppClassSpec& Spec, FString& OutHeaderPath, FString& OutSourcePath);
    
    /** Format C++ identifier (PascalCase, etc.) */
    static FString FormatIdentifier(const FString& Input, bool bIsFunctionName = false);
    
    /** Get UPROPERTY string for property */
    static FString GetUPropertyString(const FCppPropertySpec& Property);
    
    /** Get UFUNCTION string for function */
    static FString GetUFunctionString(const FCppFunctionSpec& Function);
};
