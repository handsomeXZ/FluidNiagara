// Copyright HandsomeCheese. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "EditorViewportClient.h"

class UFDOverlayViewportButtonsAPI;

enum EFDOverlay3DViewportClientDisplayMode : uint8 {
	X = 0,
	Y = 1,
	Z = 2,
	W = 3
};

enum EFDOverlay3DViewportClientRenderMode : uint8 {
	DefaultLight = 0,
	Emissive = 1,
	Translucency = 2,
	Transition = 3
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnToggleOverlayChannel, uint8, bool)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnToggleOverlayRender, uint8)

/**
 * Viewport client for the 3d live preview in the UV editor. Currently same as editor viewport
 * client but doesn't allow editor gizmos/widgets, and alters orbit camera control.
 */
class FDOVERLAYEDITOR_API FFDOverlay3DViewportClient : public FEditorViewportClient
{
public:

	FFDOverlay3DViewportClient(FEditorModeTools* InModeTools, FPreviewScene* InPreviewScene = nullptr,
		const TWeakPtr<SEditorViewport>& InEditorViewportWidget = nullptr, UFDOverlayViewportButtonsAPI* ViewportButtonsAPI = nullptr);

	virtual ~FFDOverlay3DViewportClient();


	bool CanSetWidgetMode(UE::Widget::EWidgetMode NewMode) const override {	return true; }
	void SetWidgetMode(UE::Widget::EWidgetMode NewMode) override;
	UE::Widget::EWidgetMode GetWidgetMode() const override { return UE::Widget::EWidgetMode::WM_None; }
	void FocusCameraOnSelection();
	FOnToggleOverlayChannel& OnToggleOverlayChannel() { return OnToggleOverlayChannelDelegate; }
	FOnToggleOverlayRender& OnToggleOverlayRender() { return OnToggleOverlayRenderDelegate; }
public:
	bool GetDisplayMode(EFDOverlay3DViewportClientDisplayMode Mode);
	void ToggleDisplayMode(EFDOverlay3DViewportClientDisplayMode Mode);

	bool GetRenderMode(EFDOverlay3DViewportClientRenderMode Mode);
	void ToggleRenderMode(EFDOverlay3DViewportClientRenderMode Mode);
private:

	bool DisplayModeX = true;
	bool DisplayModeY = true;
	bool DisplayModeZ = true;
	bool DisplayModeW = true;

	EFDOverlay3DViewportClientRenderMode RenderMode = DefaultLight;

	UFDOverlayViewportButtonsAPI* ViewportButtonsAPI;
	FOnToggleOverlayChannel OnToggleOverlayChannelDelegate;
	FOnToggleOverlayRender OnToggleOverlayRenderDelegate;
};