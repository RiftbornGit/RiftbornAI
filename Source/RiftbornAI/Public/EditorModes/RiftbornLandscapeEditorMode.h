#pragma once

#include "CoreMinimal.h"
#include "Tools/UEdMode.h"
#include "RiftbornLandscapeEditorMode.generated.h"

UCLASS()
class RIFTBORNAI_API URiftbornLandscapeEditorMode : public UEdMode
{
	GENERATED_BODY()

public:
	static const FEditorModeID EM_RiftbornLandscape;

	virtual void Enter() override;
	virtual void Exit() override;
	virtual void CreateToolkit() override;
};
