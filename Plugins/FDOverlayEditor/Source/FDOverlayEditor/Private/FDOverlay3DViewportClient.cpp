

#include "FDOverlay3DViewportClient.h"

#include "BaseBehaviors/ClickDragBehavior.h"
#include "BaseBehaviors/MouseWheelBehavior.h"
#include "ContextObjectStore.h"
#include "EditorModeManager.h"
#include "EdModeInteractiveToolsContext.h"
#include "Drawing/MeshDebugDrawing.h"
#include "FrameTypes.h"
//#include "FDOverlayEditorMode.h"
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
void FFDOverlay3DViewportClient::FocusCameraOnSelection()
{
	if (ViewportButtonsAPI)
	{
		ViewportButtonsAPI->InitiateFocusCameraOnSelection();
	}
}