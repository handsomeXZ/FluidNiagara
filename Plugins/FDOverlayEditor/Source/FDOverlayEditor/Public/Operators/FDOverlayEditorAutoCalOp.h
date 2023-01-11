#pragma once

#include "CoreMinimal.h"
#include "Util/ProgressCancel.h"
#include "ModelingOperators.h"
#include "InteractiveTool.h"

#include "FDOverlayEditorAutoCalOp.generated.h"



UENUM()
enum class EFDOverlayEditorAutoCalType : uint8
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

	UPROPERTY(EditAnywhere, Category = "FDOverlay AutoCal", meta = (EditCondition = "LayoutType == EFDOverlayEditorAutoCalType::Line"))
	FVector3f LineOrigin;
	UPROPERTY(EditAnywhere, Category = "FDOverlay AutoCal", meta = (EditCondition = "LayoutType == EFDOverlayEditorAutoCalType::Line"))
	FVector3f LineDirection;

	UPROPERTY(EditAnywhere, Category = "FDOverlay AutoCal", meta = (ClampMin = "0", ClampMax = "360"))
	float UVOffset;

	UPROPERTY(EditAnywhere, Category = "FDOverlay AutoCal")
	UCurveFloat* UVCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "FDOverlay AutoCal")
	int XYSize = 512;

	UPROPERTY(EditAnywhere, Category = "FDOverlay AutoCal")
	FString Name = TEXT("OutputName");

	UPROPERTY(EditAnywhere, Category = "FDOverlay AutoCal")
	FString AssetPathFormat = TEXT("{AssetFolder}/{AssetName}_Texture2D_{OutputName}_{MaterialID}");

};

