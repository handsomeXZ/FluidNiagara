// Copyright HandsomeCheese. All Rights Reserved.
#include "FDOverlay2DViewportClient.h"

#include "ContextObjectStore.h"
#include "EditorModeManager.h"
#include "FrameTypes.h"
//#include "FDOverlayEditorMode.h"
#include "UVEditorUXSettings.h"
#include "CameraController.h"

FFDOverlay2DViewportClient::FFDOverlay2DViewportClient(FEditorModeTools* InModeTools,
	FPreviewScene* InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewportWidget,
	UFDOverlayViewportButtonsAPI* ViewportButtonsAPIIn)
	: FEditorViewportClient(InModeTools, InPreviewScene, InEditorViewportWidget)
	, ViewportButtonsAPI(ViewportButtonsAPIIn)
{
	ShowWidget(false);

	// Don't draw the little XYZ drawing in the corner.
	bDrawAxes = false;

	// We want our near clip plane to be quite close so that we can zoom in further.
	OverrideNearClipPlane(KINDA_SMALL_NUMBER);

	// Set up viewport manipulation behaviors:
	FEditorCameraController* CameraControllerPtr = GetCameraController();
	CameraController->GetConfig().MovementAccelerationRate = 0.0;
	CameraController->GetConfig().RotationAccelerationRate = 0.0;
	CameraController->GetConfig().FOVAccelerationRate = 0.0;

}
bool FFDOverlay2DViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	// We'll support disabling input like our base class, even if it does not end up being used.
	return true;

}
void FFDOverlay2DViewportClient::SetWidgetMode(UE::Widget::EWidgetMode NewMode)
{

}

bool FFDOverlay2DViewportClient::CanSetWidgetMode(UE::Widget::EWidgetMode NewMode) const
{
	return false;
}

UE::Widget::EWidgetMode FFDOverlay2DViewportClient::GetWidgetMode() const
{
	return UE::Widget::EWidgetMode::WM_None;
}

bool FFDOverlay2DViewportClient::GetDisplayMode(EFDOverlay2DViewportClientDisplayMode ModeIn)
{
	return ModeIn == DisplayMode;
}

void FFDOverlay2DViewportClient::ToggleDisplayMode(EFDOverlay2DViewportClientDisplayMode ModeIn)
{
	DisplayMode = ModeIn;
	OnSwitchMaterialIDModeDelegate.Broadcast((uint8)ModeIn);
}

bool FFDOverlay2DViewportClient::ShouldOrbitCamera() const
{
	return false; 
}

void FFDOverlay2DViewportClient::ExecuteOnMaterialIDAdd()
{
	OnMaterialIDAddDelegate.Broadcast();
}

void FFDOverlay2DViewportClient::ExecuteOnMaterialIDSub()
{
	OnMaterialIDSubDelegate.Broadcast();
}