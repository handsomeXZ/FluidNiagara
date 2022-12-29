

#pragma once

#include "CoreMinimal.h"

#include "SAssetEditorViewport.h"



class FDASSISTOR_API SFDAssistor2DViewport : public SAssetEditorViewport
{
public:

	// These allow the toolkit to add an accept/cancel overlay when needed. PopulateViewportOverlays
	// is not helpful here because that gets called just once.
	virtual void AddOverlayWidget(TSharedRef<SWidget> OverlaidWidget);
	virtual void RemoveOverlayWidget(TSharedRef<SWidget> OverlaidWidget);

	// SEditorViewport
	virtual void BindCommands() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	bool IsWidgetModeActive(UE::Widget::EWidgetMode Mode) const override;
};
