// CodexStyle.h — STUB (CodexBrain removed, provides default UE editor colors)
#pragma once
#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateTypes.h"

class RIFTBORNAI_API FRiftbornCodexStyle
{
public:
    static void Initialize() {}
    static void Shutdown() {}

    // Color stubs — return default UE editor tones
    static FSlateColor Background()      { return FSlateColor(FLinearColor(0.02f, 0.02f, 0.03f)); }
    static FSlateColor InputBackground() { return FSlateColor(FLinearColor(0.05f, 0.05f, 0.07f)); }
    static FSlateColor TextBright()      { return FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f)); }
    static FSlateColor TextMuted()       { return FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)); }
    static FSlateColor Accent()          { return FSlateColor(FLinearColor(0.2f, 0.6f, 1.0f)); }
    static FSlateColor Panel()           { return FSlateColor(FLinearColor(0.03f, 0.03f, 0.04f)); }
    static FSlateColor Header()          { return FSlateColor(FLinearColor(0.04f, 0.04f, 0.06f)); }

    // Font stubs
    static FSlateFontInfo BodyFont(float Size = 10.0f)
    {
        return FCoreStyle::GetDefaultFontStyle("Regular", Size);
    }
    static FSlateFontInfo HeadingFont(float Size = 13.0f)
    {
        return FCoreStyle::GetDefaultFontStyle("Bold", Size);
    }
    static FSlateFontInfo MonoFont(float Size = 9.0f)
    {
        return FCoreStyle::GetDefaultFontStyle("Mono", Size);
    }

    static FSlateColor Positive()  { return FSlateColor(FLinearColor(0.2f, 0.8f, 0.3f)); }
    static FSlateColor Warning()   { return FSlateColor(FLinearColor(1.0f, 0.7f, 0.2f)); }
    static FSlateColor Negative()  { return FSlateColor(FLinearColor(0.9f, 0.2f, 0.2f)); }
};
