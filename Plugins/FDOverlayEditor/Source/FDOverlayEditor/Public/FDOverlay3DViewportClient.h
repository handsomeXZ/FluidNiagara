

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

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnToggleOverlayChannel, EFDOverlay3DViewportClientDisplayMode, bool)

/**
 * Viewport client for the 3d live preview in the UV editor. Currently same as editor viewport
 * client but doesn't allow editor gizmos/widgets, and alters orbit camera control.
 */
class FDOVERLAYEDITOR_API FFDOverlay3DViewportClient : public FEditorViewportClient
{
public:

	FFDOverlay3DViewportClient(FEditorModeTools* InModeTools, FPreviewScene* InPreviewScene = nullptr,
		const TWeakPtr<SEditorViewport>& InEditorViewportWidget = nullptr, UFDOverlayViewportButtonsAPI* ViewportButtonsAPI = nullptr);

	virtual ~FFDOverlay3DViewportClient() {}


	bool CanSetWidgetMode(UE::Widget::EWidgetMode NewMode) const override {	return true; }
	void SetWidgetMode(UE::Widget::EWidgetMode NewMode) override;
	UE::Widget::EWidgetMode GetWidgetMode() const override { return UE::Widget::EWidgetMode::WM_None; }
	void FocusCameraOnSelection();
	FOnToggleOverlayChannel& OnToggleOverlayChannel() { return OnToggleOverlayChannelDelegate; }
public:
	bool GetDisplayMode(EFDOverlay3DViewportClientDisplayMode Mode);
	void ToggleDisplayMode(EFDOverlay3DViewportClientDisplayMode Mode);

protected:

	bool DisplayModeX = true;
	bool DisplayModeY = true;
	bool DisplayModeZ = true;
	bool DisplayModeW = true;

	UFDOverlayViewportButtonsAPI* ViewportButtonsAPI;
	FOnToggleOverlayChannel OnToggleOverlayChannelDelegate;
};