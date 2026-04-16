#pragma once

#include "CoreMinimal.h"
#include "InteractiveTool.h"
#include "InteractiveToolBuilder.h"
#include "RiftbornDrawPathTool.generated.h"

class URiftbornLandscapePath;

UCLASS(Transient)
class URiftbornDrawPathTool : public UInteractiveTool
{
	GENERATED_BODY()

public:
	void CommitPath();
	void ClearControlPoints();
	URiftbornLandscapePath* SaveToAsset(URiftbornLandscapePath* ExistingAsset) const;
};

UCLASS(Transient)
class URiftbornDrawPathToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};
