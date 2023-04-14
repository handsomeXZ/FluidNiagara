// Copyright HandsomeCheese. All Rights Reserved.

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
	case EFDOverlay3DViewportClientDisplayMode::X:
		return DisplayModeX;
	case EFDOverlay3DViewportClientDisplayMode::Y:
		return DisplayModeY;
	case EFDOverlay3DViewportClientDisplayMode::Z:
		return DisplayModeZ;
	case EFDOverlay3DViewportClientDisplayMode::W:
		return DisplayModeW;
	default:
		return false;
	}
}

void FFDOverlay3DViewportClient::ToggleDisplayMode(EFDOverlay3DViewportClientDisplayMode ModeIn)
{
	switch (ModeIn)
	{
	case EFDOverlay3DViewportClientDisplayMode::X:
		DisplayModeX = !DisplayModeX;
		OnToggleOverlayChannelDelegate.Broadcast((uint8)EFDOverlay3DViewportClientDisplayMode::X, DisplayModeX);
		break;
	case EFDOverlay3DViewportClientDisplayMode::Y:
		DisplayModeY = !DisplayModeY;
		OnToggleOverlayChannelDelegate.Broadcast((uint8)EFDOverlay3DViewportClientDisplayMode::Y, DisplayModeY);
		break;
	case EFDOverlay3DViewportClientDisplayMode::Z:
		DisplayModeZ = !DisplayModeZ;
		OnToggleOverlayChannelDelegate.Broadcast((uint8)EFDOverlay3DViewportClientDisplayMode::Z, DisplayModeZ);
		break;
	case EFDOverlay3DViewportClientDisplayMode::W:
		DisplayModeW = !DisplayModeW;
		OnToggleOverlayChannelDelegate.Broadcast((uint8)EFDOverlay3DViewportClientDisplayMode::W, DisplayModeW);
		break;
	}

}

bool FFDOverlay3DViewportClient::GetRenderMode(EFDOverlay3DViewportClientRenderMode ModeIn)
{
	if (ModeIn == RenderMode)
	{
		return true;
	}
	return false;
}

void FFDOverlay3DViewportClient::ToggleRenderMode(EFDOverlay3DViewportClientRenderMode ModeIn)
{
	switch (ModeIn)
	{
	case EFDOverlay3DViewportClientRenderMode::DefaultLight:
		RenderMode = EFDOverlay3DViewportClientRenderMode::DefaultLight;
		OnToggleOverlayRenderDelegate.Broadcast((uint8)EFDOverlay3DViewportClientRenderMode::DefaultLight);
		break;
	case EFDOverlay3DViewportClientRenderMode::Emissive:
		RenderMode = EFDOverlay3DViewportClientRenderMode::Emissive;
		OnToggleOverlayRenderDelegate.Broadcast((uint8)EFDOverlay3DViewportClientRenderMode::Emissive);
		break;
	case EFDOverlay3DViewportClientRenderMode::Translucency:
		RenderMode = EFDOverlay3DViewportClientRenderMode::Translucency;
		OnToggleOverlayRenderDelegate.Broadcast((uint8)EFDOverlay3DViewportClientRenderMode::Translucency);
		break;
	case EFDOverlay3DViewportClientRenderMode::Transition:
		RenderMode = EFDOverlay3DViewportClientRenderMode::Transition;
		OnToggleOverlayRenderDelegate.Broadcast((uint8)EFDOverlay3DViewportClientRenderMode::Transition);
		break;
	}
}