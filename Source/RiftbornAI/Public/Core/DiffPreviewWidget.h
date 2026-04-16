// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Structured diff hunk with positional metadata
 */
struct FDiffHunk
{
    FString Type;
    int32 OldStart = 0;
    int32 OldLines = 0;
    int32 NewStart = 0;
    int32 NewLines = 0;
    TArray<FString> OldText;
    TArray<FString> NewText;
};

/**
 * Diff entry for preview
 */
struct FDiffEntry
{
    FString FilePath;
    FString OldContent;
    FString NewContent;
    TArray<FString> AddedLines;
    TArray<FString> RemovedLines;
    TArray<FString> ModifiedLines;
    TArray<FDiffHunk> DiffHunks;
};

/**
 * Diff preview widget - Shows changes before applying
 * GitHub-style diff view with syntax highlighting
 */
class SDiffPreviewWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SDiffPreviewWidget)
        : _ShowHeader(true)
        , _ShowActions(true)
    {}
        SLATE_ARGUMENT(TArray<FDiffEntry>, Diffs)
        SLATE_ARGUMENT(bool, ShowHeader)
        SLATE_ARGUMENT(bool, ShowActions)
        SLATE_ARGUMENT(FString, TitleText)
        SLATE_EVENT(FSimpleDelegate, OnAccept)
        SLATE_EVENT(FSimpleDelegate, OnReject)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    TArray<FDiffEntry> Diffs;
    bool bShowHeader = true;
    bool bShowActions = true;
    FString TitleText;
    FSimpleDelegate OnAcceptDelegate;
    FSimpleDelegate OnRejectDelegate;
    
    TSharedRef<SWidget> BuildDiffView(const FDiffEntry& Diff);
    TSharedRef<SWidget> BuildFileDiffHeader(const FDiffEntry& Diff);
    TSharedRef<SWidget> BuildDiffLines(const FDiffEntry& Diff);
    
    FReply OnAcceptClicked();
    FReply OnRejectClicked();
    
    // Diff line styling
    FSlateColor GetLineColor(bool bAdded, bool bRemoved) const;
    FLinearColor GetLineBackground(bool bAdded, bool bRemoved) const;
};
