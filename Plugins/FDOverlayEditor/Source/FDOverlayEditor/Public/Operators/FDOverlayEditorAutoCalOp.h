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
	Point,
	MultiLine,
	MultiPoint
};
UENUM()
enum class EVRICType : uint8
{
	PLD,
	PPD
};
USTRUCT()
struct FMultiLineData
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere)
	FVector3f LineOrigin;
	UPROPERTY(EditAnywhere)
	FVector3f LineDirection = FVector3f(0, 0, 1);
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "360"))
	float UVOffset;
	UPROPERTY(EditAnywhere)
	UCurveFloat* UVCurve = nullptr;
};
USTRUCT()
struct FMultiPointData
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, DisplayName = "PointOrigin")
	FVector3f LineOrigin;
	UPROPERTY(EditAnywhere, DisplayName = "UVDirection")
	FVector3f LineDirection = FVector3f(0, 0, 1);
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "360"))
	float UVOffset;
	UPROPERTY(EditAnywhere)
	UCurveFloat* UVCurve = nullptr;
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
	UPROPERTY(EditAnywhere, Category = "AutoCalMode")
	EFDOverlayEditorAutoCalType LayoutType = EFDOverlayEditorAutoCalType::Line;

	UPROPERTY(EditAnywhere, Category = "SingleMode", meta = (EditCondition = "LayoutType == EFDOverlayEditorAutoCalType::Line"))
	FVector3f LineOrigin;
	UPROPERTY(EditAnywhere, Category = "SingleMode", meta = (EditCondition = "LayoutType == EFDOverlayEditorAutoCalType::Line"))
	FVector3f LineDirection = FVector3f(0, 0, 1);

	UPROPERTY(EditAnywhere, Category = "SingleMode", meta = (EditCondition = "LayoutType == EFDOverlayEditorAutoCalType::Point"))
	FVector3f PointOrigin;
	UPROPERTY(EditAnywhere, Category = "SingleMode", meta = (EditCondition = "LayoutType == EFDOverlayEditorAutoCalType::Point"))
	FVector3f UVDirection = FVector3f(0, 0, 1);

	UPROPERTY(EditAnywhere, Category = "SingleMode", meta = (ClampMin = "0", ClampMax = "360", EditCondition = "LayoutType == EFDOverlayEditorAutoCalType::Point || LayoutType == EFDOverlayEditorAutoCalType::Line"))
	float UVOffset;
	UPROPERTY(EditAnywhere, Category = "SingleMode", meta = (EditCondition = "LayoutType == EFDOverlayEditorAutoCalType::Point || LayoutType == EFDOverlayEditorAutoCalType::Line"))
	UCurveFloat* UVCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "MultiMode", meta = (EditCondition = "LayoutType == EFDOverlayEditorAutoCalType::MultiLine"))
	TArray<FMultiLineData> MultiLineData;
	UPROPERTY(EditAnywhere, Category = "MultiMode", meta = (EditCondition = "LayoutType == EFDOverlayEditorAutoCalType::MultiPoint"))
	TArray<FMultiPointData> MultiPointData;

	UPROPERTY(EditAnywhere, Category = "Settings")
	int XYSize = 256;
	UPROPERTY(EditAnywhere, Category = "Settings")
	FString Name = TEXT("OutputName");
	UPROPERTY(EditAnywhere, Category = "Settings")
	FString AssetPathFormat = TEXT("{AssetFolder}/{AssetName}_Texture2D_{OutputName}_{MIDNum}");
	
	UPROPERTY(EditAnywhere, Category = "Settings", meta = (EditCondition = "LayoutType == EFDOverlayEditorAutoCalType::MultiLine || LayoutType == EFDOverlayEditorAutoCalType::MultiPoint"))
	EVRICType VRICType = EVRICType::PLD;
	/**
	 * 0 ~ 1. 
	 * Vector relative interference coefficient. 
	 * This Name looks good, hahaha!!
	 */
	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ClampMin = "0", ClampMax = "1", EditCondition = "VRICType == EVRICType::PLD && (LayoutType == EFDOverlayEditorAutoCalType::MultiLine || LayoutType == EFDOverlayEditorAutoCalType::MultiPoint)"))
	float VRIC = 0;
};

