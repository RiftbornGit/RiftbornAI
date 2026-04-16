// Copyright RiftbornAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Engine/StaticMesh.h"

/**
 * Model preview widget with 3D viewport and mesh statistics
 * Shows: Faces, Vertices, Triangle/Quad info, Accept/Reject buttons
 */
class SModelPreviewWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SModelPreviewWidget) {}
        SLATE_ARGUMENT(FString, ModelPath)
        SLATE_ARGUMENT(FString, RequestId)
        SLATE_EVENT(FSimpleDelegate, OnAccept)
        SLATE_EVENT(FSimpleDelegate, OnReject)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual ~SModelPreviewWidget();

    /** Update preview with new model */
    void SetModelPath(const FString& InModelPath);
    
    /** Get mesh statistics */
    struct FMeshStats
    {
        int32 Vertices = 0;
        int32 Triangles = 0;
        int32 Quads = 0;
        int32 Faces = 0;
        bool bHasUVs = false;
        bool bHasNormals = false;
        bool bHasTangents = false;
        int32 MaterialSlots = 0;
        FVector BoundingBoxSize = FVector::ZeroVector;
        float MemoryUsageMB = 0.0f;
    };

private:
    FString ModelPath;
    FString RequestId;
    FSimpleDelegate OnAcceptDelegate;
    FSimpleDelegate OnRejectDelegate;

    TSharedPtr<STextBlock> StatsText;
    TSharedPtr<SBorder> ViewportBorder;
    UStaticMesh* LoadedMesh = nullptr;
    FMeshStats CurrentStats;

    /** Load and analyze mesh */
    void LoadMeshFile();
    
    /** Analyze mesh and populate stats */
    FMeshStats AnalyzeMesh(UStaticMesh* Mesh);
    
    /** Parse OBJ file for statistics (fallback when mesh not loaded) */
    FMeshStats ParseOBJFile(const FString& FilePath);
    
    /** Parse FBX file metadata */
    FMeshStats ParseFBXFile(const FString& FilePath);
    
    /** Generate stats display text */
    FText GetStatsText() const;
    
    /** Button handlers */
    FReply OnAcceptClicked();
    FReply OnRejectClicked();
    FReply OnReimportClicked();
};
