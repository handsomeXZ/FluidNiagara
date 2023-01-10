#pragma once

#include "CoreMinimal.h"
#include "Util/ProgressCancel.h"
#include "ModelingOperators.h"
#include "InteractiveTool.h"

#include "FDOverlayEditorAutoCalOp.generated.h"

UENUM()
enum class EFDOverlayEditorAutoCalType
{
	Line,
	Point
};

/**
 * FDOverlay AutoCal Settings
 */
UCLASS()
class FDOVERLAYEDITOR_API UFDOverlayEditorAutoCalProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	/** Type of AutoCal applied to calculate */
	UPROPERTY(EditAnywhere, Category = "FDOverlay AutoCal")
	EFDOverlayEditorAutoCalType LayoutType = EFDOverlayEditorAutoCalType::Line;

	UPROPERTY(EditAnywhere, Category = "FDOverlay AutoCal")
	FString Name = TEXT("OutputName");

	UPROPERTY(EditAnywhere, Category = "FDOverlay AutoCal"/*, meta = (EditCondition = "bGenerateFrames")*/)
	FString AssetPathFormat = TEXT("{AssetFolder}/{AssetName}_Texture2D_{OutputName}_{MaterialID}");

};

