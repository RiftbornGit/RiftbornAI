// BlueprintTypeUtils.h
// Shared utility for Blueprint type conversions

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphPin.h"

/**
 * Utility class for Blueprint type conversions.
 * Centralizes string-to-FEdGraphPinType mapping used across the codebase.
 */
class RIFTBORNAI_API FBlueprintTypeUtils
{
public:
    /**
     * Convert a type name string to an FEdGraphPinType.
     *
     * Supported types:
     * - Primitives: bool, int, int32, int64, float, double, byte
     * - Strings: string, FString, name, FName, text, FText
     * - Structs: vector, FVector, rotator, FRotator, transform, FTransform,
     *            color, FLinearColor, vector2d, FVector2D
     * - Object: object (defaults to UObject*)
     *
     * @param TypeName The type name (case-insensitive)
     * @param OutPinType The resulting pin type
     * @return True if the type was recognized, false otherwise
     */
    static bool StringToPinType(const FString& TypeName, FEdGraphPinType& OutPinType);

    /**
     * Convert an FEdGraphPinType back to a readable string.
     * Useful for logging and debugging.
     *
     * @param PinType The pin type to convert
     * @return A human-readable type name
     */
    static FString PinTypeToString(const FEdGraphPinType& PinType);

    /**
     * Check if a type name is a recognized Blueprint type.
     *
     * @param TypeName The type name to check
     * @return True if the type is supported
     */
    static bool IsValidTypeName(const FString& TypeName);

    /**
     * Get a list of all supported type names.
     *
     * @return Array of supported type name strings
     */
    static TArray<FString> GetSupportedTypeNames();
};
