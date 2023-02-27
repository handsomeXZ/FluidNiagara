

#include "FDOverlay3DViewportClient.h"

#include "BaseBehaviors/ClickDragBehavior.h"
#include "BaseBehaviors/MouseWheelBehavior.h"
#include "ContextObjectStore.h"
#include "EditorModeManager.h"
#include "EdModeInteractiveToolsContext.h"
#include "Drawing/MeshDebugDrawing.h"
#include "FrameTypes.h"
#include "Templates\SharedPointer.h"

#include "Context/FDOverlayViewportButtonsAPI.h"


FFDOverlay3DViewportClient::FFDOverlay3DViewportClient(FEditorModeTools* InModeTools,
	FPreviewScene* InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewportWidget,
	UFDOverlayViewportButtonsAPI* ViewportButtonsAPIIn)
	: FEditorViewportClient(InModeTools, InPreviewScene, InEditorViewportWidget),
	ViewportButtonsAPI(ViewportButtonsAPIIn)
{
	// We want our near clip plane to be quite close so that we can zoom in further.
	OverrideNearClipPlane(KINDA_SMALL_NUMBER);
}

FFDOverlay3DViewportClient::~FFDOverlay3DViewportClient()
{
	OnToggleOverlayChannelDelegate.RemoveAll(this);
}


void FFDOverlay3DViewportClient::FocusCameraOnSelection()
{
	if (ViewportButtonsAPI)
	{
		ViewportButtonsAPI->InitiateFocusCameraOnSelection();
	}
}
void FFDOverlay3DViewportClient::SetWidgetMode(UE::Widget::EWidgetMode NewMode)
{
	
}

bool FFDOverlay3DViewportClient::GetDisplayMode(EFDOverlay3DViewportClientDisplayMode ModeIn)
{
	switch (ModeIn)
	{
		case X:
			return DisplayModeX;
		case Y:
			return DisplayModeY;
		case Z:
			return DisplayModeZ;
		case W:
			return DisplayModeW;
		default:
			return false;
	}
}

void FFDOverlay3DViewportClient::ToggleDisplayMode(EFDOverlay3DViewportClientDisplayMode ModeIn)
{
	switch (ModeIn)
	{
	case X:
		DisplayModeX = !DisplayModeX;
		OnToggleOverlayChannelDelegate.Broadcast((uint8)EFDOverlay3DViewportClientDisplayMode::X, DisplayModeX);
		break;
	case Y:
		DisplayModeY = !DisplayModeY;
		OnToggleOverlayChannelDelegate.Broadcast((uint8)EFDOverlay3DViewportClientDisplayMode::Y, DisplayModeY);
		break;
	case Z:
		DisplayModeZ = !DisplayModeZ;
		OnToggleOverlayChannelDelegate.Broadcast((uint8)EFDOverlay3DViewportClientDisplayMode::Z, DisplayModeZ);
		break;
	case W:
		DisplayModeW = !DisplayModeW;
		OnToggleOverlayChannelDelegate.Broadcast((uint8)EFDOverlay3DViewportClientDisplayMode::W, DisplayModeW);
		break;
	}

}