
#include "FDOverlay2DViewportClient.h"

#include "BaseBehaviors/ClickDragBehavior.h"
#include "BaseBehaviors/MouseWheelBehavior.h"
#include "ContextObjectStore.h"
#include "EditorModeManager.h"
#include "EdModeInteractiveToolsContext.h"
#include "Drawing/MeshDebugDrawing.h"
#include "FrameTypes.h"
//#include "FDOverlayEditorMode.h"
#include "UVEditorUXSettings.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Engine/Canvas.h"
#include "Math/Box.h"
#include "MathUtil.h"
#include "CameraController.h"



namespace FFDOverlay2DViewportClientLocals {

	enum class ETextAnchorPosition : uint8
	{
		None = 0,
		VerticalTop = 1 << 0,
		VerticalMiddle = 1 << 1,
		VerticalBottom = 1 << 2,
		HorizontalLeft = 1 << 3,
		HorizontalMiddle = 1 << 4,
		HorizontalRight = 1 << 5,
		TopLeftCorner = VerticalTop | HorizontalLeft,
		TopRightCorner = VerticalTop | HorizontalRight,
		BottomLeftCorner = VerticalBottom | HorizontalLeft,
		BottomRightCorner = VerticalBottom | HorizontalRight,
		TopCenter = VerticalTop | HorizontalMiddle,
		BottomCenter = VerticalBottom | HorizontalMiddle,
		LeftCenter = VerticalMiddle | HorizontalLeft,
		RightCenter = VerticalMiddle | HorizontalRight,
		Center = VerticalMiddle | HorizontalMiddle,
	};
	ENUM_CLASS_FLAGS(ETextAnchorPosition);

	struct FTextPadding
	{
		float TopPadding = 0.0;
		float BottomPadding = 0.0;
		float LeftPadding = 0.0;
		float RightPadding = 0.0;
	};

	FText ConvertToExponentialNotation(double InValue, double MinExponent = 0.0)
	{
		double Exponent = FMath::LogX(10, InValue < 0 ? -InValue : InValue);
		Exponent = FMath::Floor(Exponent);
		if (FMath::IsFinite(Exponent) && FMath::Abs(Exponent) > MinExponent)
		{
			double Divisor = FMath::Pow(10.0, Exponent);
			double Base = InValue / Divisor;
			return FText::Format(FTextFormat::FromString("{0}E{1}"), FText::AsNumber(Base), FText::AsNumber(Exponent));
		}
		else
		{
			return FText::AsNumber(InValue);
		}
	}

	FVector2D ComputeAnchoredPosition(const FVector2D& InPosition, const FText& InText, const UFont* InFont, UCanvas& Canvas, ETextAnchorPosition InAnchorPosition, const FTextPadding& InPadding, bool bClampInsideScreenBounds) {
		FVector2D OutPosition = InPosition;

		FVector2D TextSize(0, 0), TextShift(0, 0);
		Canvas.TextSize(InFont, InText.ToString(), TextSize[0], TextSize[1]);

		FBox2D TextBoundingBox(FVector2D(0, 0), TextSize);
		FBox2D ScreenBoundingBox(FVector2D(0, 0), FVector2D(Canvas.SizeX, Canvas.SizeY));

		if ((InAnchorPosition & ETextAnchorPosition::VerticalTop) != ETextAnchorPosition::None)
		{
			TextShift[1] = InPadding.TopPadding; // We are anchored to the top left hand corner by default
		}
		else if ((InAnchorPosition & ETextAnchorPosition::VerticalMiddle) != ETextAnchorPosition::None)
		{
			TextShift[1] = -TextSize[1] / 2.0;
		}
		else if ((InAnchorPosition & ETextAnchorPosition::VerticalBottom) != ETextAnchorPosition::None)
		{
			TextShift[1] = -TextSize[1] - InPadding.BottomPadding;
		}

		if ((InAnchorPosition & ETextAnchorPosition::HorizontalLeft) != ETextAnchorPosition::None)
		{
			TextShift[0] = InPadding.LeftPadding; // We are anchored to the top left hand corner by default
		}
		else if ((InAnchorPosition & ETextAnchorPosition::HorizontalMiddle) != ETextAnchorPosition::None)
		{
			TextShift[0] = -TextSize[0] / 2.0;
		}
		else if ((InAnchorPosition & ETextAnchorPosition::HorizontalRight) != ETextAnchorPosition::None)
		{
			TextShift[0] = -TextSize[0] - InPadding.RightPadding;
		}

		TextBoundingBox = TextBoundingBox.ShiftBy(InPosition + TextShift);

		if (bClampInsideScreenBounds && !ScreenBoundingBox.IsInside(TextBoundingBox)) {
			FVector2D TextCenter, TextExtents;
			TextBoundingBox.GetCenterAndExtents(TextCenter, TextExtents);
			FBox2D ScreenInsetBoundingBox(ScreenBoundingBox);
			ScreenInsetBoundingBox = ScreenInsetBoundingBox.ExpandBy(TextExtents * -1.0);
			FVector2D MovedCenter = ScreenInsetBoundingBox.GetClosestPointTo(TextCenter);
			TextBoundingBox = TextBoundingBox.MoveTo(MovedCenter);
		}

		return TextBoundingBox.Min;
	}

	FCanvasTextItem CreateTextAnchored(const FVector2D& InPosition, const FText& InText, const UFont* InFont,
		const FLinearColor& InColor, UCanvas& Canvas, ETextAnchorPosition InAnchorPosition, const FTextPadding& InPadding, bool bClampInsideScreenBounds = true)
	{
		FVector2D OutPosition = ComputeAnchoredPosition(InPosition, InText, InFont, Canvas, InAnchorPosition, InPadding, bClampInsideScreenBounds);
		return FCanvasTextItem(OutPosition, InText, InFont, InColor);
	}

	bool ConvertUVToPixel(const FVector2D& UVIn, FVector2D& PixelOut, const FSceneView& View)
	{
		FVector TestWorld = FUVEditorUXSettings::UVToVertPosition(FUVEditorUXSettings::ExternalUVToInternalUV((FVector2f)UVIn));
		FVector4 TestProjectedHomogenous = View.WorldToScreen(TestWorld);
		bool bValid = View.ScreenToPixel(TestProjectedHomogenous, PixelOut);
		return bValid;
	}

	void ConvertPixelToUV(const FVector2D& PixelIn, double RelDepthIn, FVector2D& UVOut, const FSceneView& View)
	{
		FVector4 ScreenPoint = View.PixelToScreen(static_cast<float>(PixelIn.X), static_cast<float>(PixelIn.Y), static_cast<float>(RelDepthIn));
		FVector4 WorldPointHomogenous = View.ViewMatrices.GetInvViewProjectionMatrix().TransformFVector4(ScreenPoint);
		FVector WorldPoint(WorldPointHomogenous.X / WorldPointHomogenous.W,
			WorldPointHomogenous.Y / WorldPointHomogenous.W,
			WorldPointHomogenous.Z / WorldPointHomogenous.W);
		UVOut = (FVector2D)FUVEditorUXSettings::VertPositionToUV(WorldPoint);
		UVOut = (FVector2D)FUVEditorUXSettings::InternalUVToExternalUV((FVector2f)UVOut);
	}

};


FFDOverlay2DViewportClient::FFDOverlay2DViewportClient(FEditorModeTools* InModeTools,
	FPreviewScene* InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewportWidget,
	UFDOverlayViewportButtonsAPI* ViewportButtonsAPIIn,
	UFDOverlayLive2DViewportAPI* live2DViewportAPIIn)
	: FEditorViewportClient(InModeTools, InPreviewScene, InEditorViewportWidget)
	, ViewportButtonsAPI(ViewportButtonsAPIIn), Live2DViewportAPI(live2DViewportAPIIn)
{
	ShowWidget(false);

	// Don't draw the little XYZ drawing in the corner.
	bDrawAxes = false;

	// We want our near clip plane to be quite close so that we can zoom in further.
	OverrideNearClipPlane(KINDA_SMALL_NUMBER);

	// Set up viewport manipulation behaviors:

	//FEditorCameraController* CameraControllerPtr = GetCameraController();
	//CameraController->GetConfig().MovementAccelerationRate = 0.0;
	//CameraController->GetConfig().RotationAccelerationRate = 0.0;
	//CameraController->GetConfig().FOVAccelerationRate = 0.0;

	//BehaviorSet = NewObject<UInputBehaviorSet>();

	//// We'll have the priority of our viewport manipulation behaviors be lower (i.e. higher
	//// numerically) than both the gizmo default and the tool default.
	//static constexpr int DEFAULT_VIEWPORT_BEHAVIOR_PRIORITY = 150;

	//ScrollBehaviorTarget = MakeUnique<FUVEditor2DScrollBehaviorTarget>(this);
	//UClickDragInputBehavior* ScrollBehavior = NewObject<UClickDragInputBehavior>();
	//ScrollBehavior->Initialize(ScrollBehaviorTarget.Get());
	//ScrollBehavior->SetDefaultPriority(DEFAULT_VIEWPORT_BEHAVIOR_PRIORITY);
	//ScrollBehavior->SetUseRightMouseButton();
	//BehaviorSet->Add(ScrollBehavior);

	//ZoomBehaviorTarget = MakeUnique<FUVEditor2DMouseWheelZoomBehaviorTarget>(this);
	//ZoomBehaviorTarget->SetCameraFarPlaneWorldZ(FUVEditorUXSettings::CameraFarPlaneWorldZ);
	//ZoomBehaviorTarget->SetCameraNearPlaneProportionZ(FUVEditorUXSettings::CameraNearPlaneProportionZ);
	//ZoomBehaviorTarget->SetZoomLimits(0.001, 100000);
	//UMouseWheelInputBehavior* ZoomBehavior = NewObject<UMouseWheelInputBehavior>();
	//ZoomBehavior->Initialize(ZoomBehaviorTarget.Get());
	//ZoomBehavior->SetDefaultPriority(DEFAULT_VIEWPORT_BEHAVIOR_PRIORITY);
	//BehaviorSet->Add(ZoomBehavior);

	//ModeTools->GetInteractiveToolsContext()->InputRouter->RegisterSource(this);

	static const FName CanvasName(TEXT("UVEditor2DCanvas"));
	CanvasObject = NewObject<UCanvas>(GetTransientPackage(), CanvasName);

	//Live2DViewportAPI->OnDrawGridChange.AddLambda(
	//	[this](bool bDrawGridIn) {
	//		bDrawGrid = bDrawGridIn;
	//	});

	//Live2DViewportAPI->OnDrawRulersChange.AddLambda(
	//	[this](bool bDrawRulersIn) {
	//		bDrawGridRulers = bDrawRulersIn;
	//	});

}

bool FFDOverlay2DViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	// We'll support disabling input like our base class, even if it does not end up being used.
	if (bDisableInput)
	{
		return true;
	}

	// Our viewport manipulation is placed in the input router that ModeTools manages
	return ModeTools->InputKey(this, EventArgs.Viewport, EventArgs.Key, EventArgs.Event);

}

bool FFDOverlay2DViewportClient::ShouldOrbitCamera() const
{
	return false; // The UV Editor's 2D viewport should never orbit.
}

void FFDOverlay2DViewportClient::SetWidgetMode(UE::Widget::EWidgetMode NewMode)
{
	if (ViewportButtonsAPI)
	{
		switch (NewMode)
		{
		case UE::Widget::EWidgetMode::WM_None:
			ViewportButtonsAPI->SetGizmoMode(UFDOverlayViewportButtonsAPI::EGizmoMode::Select);
			break;
		case UE::Widget::EWidgetMode::WM_Translate:
			ViewportButtonsAPI->SetGizmoMode(UFDOverlayViewportButtonsAPI::EGizmoMode::Transform);
			break;
		default:
			// Do nothing
			break;
		}
	}
}

bool FFDOverlay2DViewportClient::AreWidgetButtonsEnabled() const
{
	return ViewportButtonsAPI && ViewportButtonsAPI->AreGizmoButtonsEnabled();
}

void FFDOverlay2DViewportClient::SetLocationGridSnapEnabled(bool bEnabled)
{
	ViewportButtonsAPI->ToggleSnapEnabled(UFDOverlayViewportButtonsAPI::ESnapTypeFlag::Location);
}

bool FFDOverlay2DViewportClient::GetLocationGridSnapEnabled()
{
	return ViewportButtonsAPI->GetSnapEnabled(UFDOverlayViewportButtonsAPI::ESnapTypeFlag::Location);
}

void FFDOverlay2DViewportClient::SetLocationGridSnapValue(float SnapValue)
{
	ViewportButtonsAPI->SetSnapValue(UFDOverlayViewportButtonsAPI::ESnapTypeFlag::Location, SnapValue);
}

float FFDOverlay2DViewportClient::GetLocationGridSnapValue()
{
	return 	ViewportButtonsAPI->GetSnapValue(UFDOverlayViewportButtonsAPI::ESnapTypeFlag::Location);
}

void FFDOverlay2DViewportClient::SetRotationGridSnapEnabled(bool bEnabled)
{
	ViewportButtonsAPI->ToggleSnapEnabled(UFDOverlayViewportButtonsAPI::ESnapTypeFlag::Rotation);
}

bool FFDOverlay2DViewportClient::GetRotationGridSnapEnabled()
{
	return ViewportButtonsAPI->GetSnapEnabled(UFDOverlayViewportButtonsAPI::ESnapTypeFlag::Rotation);
}

void FFDOverlay2DViewportClient::SetRotationGridSnapValue(float SnapValue)
{
	ViewportButtonsAPI->SetSnapValue(UFDOverlayViewportButtonsAPI::ESnapTypeFlag::Rotation, SnapValue);
}

float FFDOverlay2DViewportClient::GetRotationGridSnapValue()
{
	return ViewportButtonsAPI->GetSnapValue(UFDOverlayViewportButtonsAPI::ESnapTypeFlag::Rotation);
}

void FFDOverlay2DViewportClient::SetScaleGridSnapEnabled(bool bEnabled)
{
	ViewportButtonsAPI->ToggleSnapEnabled(UFDOverlayViewportButtonsAPI::ESnapTypeFlag::Scale);
}

bool FFDOverlay2DViewportClient::GetScaleGridSnapEnabled()
{
	return ViewportButtonsAPI->GetSnapEnabled(UFDOverlayViewportButtonsAPI::ESnapTypeFlag::Scale);
}

void FFDOverlay2DViewportClient::SetScaleGridSnapValue(float SnapValue)
{
	ViewportButtonsAPI->SetSnapValue(UFDOverlayViewportButtonsAPI::ESnapTypeFlag::Scale, SnapValue);
}

float FFDOverlay2DViewportClient::GetScaleGridSnapValue()
{
	return ViewportButtonsAPI->GetSnapValue(UFDOverlayViewportButtonsAPI::ESnapTypeFlag::Scale);
}

bool FFDOverlay2DViewportClient::AreSelectionButtonsEnabled() const
{
	return ViewportButtonsAPI && ViewportButtonsAPI->AreSelectionButtonsEnabled();
}

bool FFDOverlay2DViewportClient::CanSetWidgetMode(UE::Widget::EWidgetMode NewMode) const
{
	if (!AreWidgetButtonsEnabled())
	{
		return false;
	}

	return NewMode == UE::Widget::EWidgetMode::WM_None
		|| NewMode == UE::Widget::EWidgetMode::WM_Translate;
}

UE::Widget::EWidgetMode FFDOverlay2DViewportClient::GetWidgetMode() const
{
	if (!AreWidgetButtonsEnabled())
	{
		return UE::Widget::EWidgetMode::WM_None;
	}

	switch (ViewportButtonsAPI->GetGizmoMode())
	{
	case UFDOverlayViewportButtonsAPI::EGizmoMode::Select:
		return UE::Widget::EWidgetMode::WM_None;
		break;
	case UFDOverlayViewportButtonsAPI::EGizmoMode::Transform:
		return UE::Widget::EWidgetMode::WM_Translate;
		break;
	default:
		return UE::Widget::EWidgetMode::WM_None;
		break;
	}
}

UFDOverlayViewportButtonsAPI::ESelectionMode FFDOverlay2DViewportClient::GetSelectionMode() const
{
	if (!AreSelectionButtonsEnabled())
	{
		return UFDOverlayViewportButtonsAPI::ESelectionMode::None;
	}

	return ViewportButtonsAPI->GetSelectionMode();
}