

#pragma once

#include "CoreMinimal.h"

#include "SAssetEditorViewport.h"

class SFDSwitchOverlayBox;

class FDOVERLAYEDITOR_API SFDOverlay2DViewport : public SAssetEditorViewport
{
public:

	virtual void AddOverlayWidget(TSharedRef<SWidget> OverlaidWidget);
	virtual void RemoveOverlayWidget(TSharedRef<SWidget> OverlaidWidget);

	// SEditorViewport
	virtual void BindCommands() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	bool IsWidgetModeActive(UE::Widget::EWidgetMode Mode) const override;
};
