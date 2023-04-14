// Copyright HandsomeCheese. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "EditorViewportClient.h"
#include "InputBehaviorSet.h"
//#include "UVEditor2DViewportBehaviorTargets.h" // FUVEditor2DScrollBehaviorTarget, FUVEditor2DMouseWheelZoomBehaviorTarget
#include "Context/FDOverlayViewportButtonsAPI.h" // UFDOverlayViewportButtonsAPI::ESelectionMode

class SWidget;
class UFDOverlayLive2DViewportAPI;

enum EFDOverlay2DViewportClientDisplayMode : uint8 {
	Compact = 0,
	Iterable = 1,
	Exploded = 2,
};

DECLARE_MULTICAST_DELEGATE(FOnMaterialIDAdd)
DECLARE_MULTICAST_DELEGATE(FOnMaterialIDSub)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSwitchMaterialIDMode, uint8)
/**
 * Client used to display a 2D view of the UV's
 */
class FDOVERLAYEDITOR_API FFDOverlay2DViewportClient : public FEditorViewportClient/*, public IInputBehaviorSource*/
{
public:
	FFDOverlay2DViewportClient(FEditorModeTools* InModeTools, FPreviewScene* InPreviewScene = nullptr,
		const TWeakPtr<SEditorViewport>& InEditorViewportWidget = nullptr, UFDOverlayViewportButtonsAPI* ViewportButtonsAPI = nullptr);

	virtual ~FFDOverlay2DViewportClient() {}

	// FEditorViewportClient
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;

	bool AreWidgetButtonsEnabled() const;

	virtual bool ShouldOrbitCamera() const override;
	bool CanSetWidgetMode(UE::Widget::EWidgetMode NewMode) const override;
	void SetWidgetMode(UE::Widget::EWidgetMode NewMode) override;
	UE::Widget::EWidgetMode GetWidgetMode() const override;	
public:
	bool GetDisplayMode(EFDOverlay2DViewportClientDisplayMode Mode);
	
	void ToggleDisplayMode(EFDOverlay2DViewportClientDisplayMode Mode);
	FOnSwitchMaterialIDMode& OnSwitchMaterialIDMode() { return OnSwitchMaterialIDModeDelegate; }

	FOnMaterialIDAdd& OnMaterialIDAdd(){ return OnMaterialIDAddDelegate; }
	void ExecuteOnMaterialIDAdd();
	FOnMaterialIDSub& OnMaterialIDSub() { return OnMaterialIDSubDelegate; }
	void ExecuteOnMaterialIDSub();

private:
	EFDOverlay2DViewportClientDisplayMode DisplayMode = Compact;
	// These get added in AddReferencedObjects for memory management
	UFDOverlayViewportButtonsAPI* ViewportButtonsAPI;
	
	FOnSwitchMaterialIDMode OnSwitchMaterialIDModeDelegate;
	FOnMaterialIDAdd OnMaterialIDAddDelegate;
	FOnMaterialIDSub OnMaterialIDSubDelegate;
	
};