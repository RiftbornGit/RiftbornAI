// Copyright RiftbornAI. All Rights Reserved.
// Deep Code Indexer - Parses C++ and Blueprints for semantic understanding

#pragma once

#include "CoreMinimal.h"

/**
 * Indexed C++ class information
 */
struct RIFTBORNAI_API FIndexedCppClass
{
	FString ClassName;
	FString FilePath;
	FString ParentClass;
	FString ModuleName;
	
	TArray<FString> Properties;      // UPROPERTY names
	TArray<FString> Functions;       // UFUNCTION names
	TArray<FString> Components;      // Component types
	TArray<FString> Interfaces;      // Implemented interfaces
	TArray<FString> Tags;            // Semantic tags
	
	FString BriefDescription;        // From comments
	bool bIsAbstract = false;
	bool bIsBlueprintable = false;
	bool bIsActor = false;
	bool bIsComponent = false;
	bool bIsGameMode = false;
	bool bIsCharacter = false;
	bool bIsPawn = false;
};

/**
 * Indexed function/method information
 */
struct RIFTBORNAI_API FIndexedFunction
{
	FString FunctionName;
	FString OwnerClass;
	FString ReturnType;
	TArray<TPair<FString, FString>> Parameters;  // Name, Type
	
	FString BriefDescription;
	bool bIsBlueprint = false;
	bool bIsEvent = false;
	bool bIsRPC = false;
	bool bIsConst = false;
};

/**
 * Indexed property information
 */
struct RIFTBORNAI_API FIndexedProperty
{
	FString PropertyName;
	FString OwnerClass;
	FString PropertyType;
	FString Category;
	FString DefaultValue;
	
	bool bIsEditable = false;
	bool bIsBlueprintVisible = false;
	bool bIsReplicated = false;
	
	TArray<FString> NaturalNames;  // "speed" for MaxWalkSpeed
};

/**
 * Deep Code Indexer
 * Parses project source code to build semantic understanding.
 */
class RIFTBORNAI_API FDeepCodeIndexer
{
public:
	static FDeepCodeIndexer& Get();
	
	/** Build index from project source */
	void BuildIndex(const FString& ProjectPath);
	
	/** Check if indexed */
	bool IsIndexed() const { return bIsIndexed; }
	
	/** Refresh specific file */
	void RefreshFile(const FString& FilePath);
	
	// =========================================================================
	// QUERIES
	// =========================================================================
	
	/** Find class by name (fuzzy) */
	const FIndexedCppClass* FindClass(const FString& Query) const;
	
	/** Find all classes of type */
	TArray<const FIndexedCppClass*> FindClassesOfType(const FString& BaseClass) const;
	
	/** Find function by name */
	const FIndexedFunction* FindFunction(const FString& Query, const FString& ClassHint = TEXT("")) const;
	
	/** Find property by natural name */
	const FIndexedProperty* FindProperty(const FString& NaturalName, const FString& ClassHint = TEXT("")) const;
	
	/** Get all properties of a class */
	TArray<FIndexedProperty> GetClassProperties(const FString& ClassName) const;
	
	/** Search code for pattern */
	TArray<FString> SearchCode(const FString& Query, int32 MaxResults = 10) const;
	
	// =========================================================================
	// CONTEXT FOR AI
	// =========================================================================
	
	/** Get context for AI about a class */
	FString GetClassContext(const FString& ClassName) const;
	
	/** Get relevant context for a query */
	FString GetRelevantCodeContext(const FString& UserQuery) const;
	
	/** Get compact summary */
	FString GetProjectCodeSummary() const;

private:
	FDeepCodeIndexer();
	
	void ParseCppFile(const FString& FilePath);
	void ParseHeaderFile(const FString& FilePath);
	void ExtractClassInfo(const FString& Content, const FString& FilePath);
	void ExtractFunctions(const FString& Content, const FString& ClassName);
	void ExtractProperties(const FString& Content, const FString& ClassName);
	void GenerateNaturalNames(FIndexedProperty& Property);
	
	TMap<FString, FIndexedCppClass> ClassIndex;
	TMap<FString, FIndexedFunction> FunctionIndex;
	TMap<FString, FIndexedProperty> PropertyIndex;
	TMap<FString, TArray<FString>> NaturalNameMap;  // "speed" -> [MaxWalkSpeed, MovementSpeed]
	
	bool bIsIndexed = false;
};
