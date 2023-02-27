
#pragma once

#include "CoreMinimal.h"

#include "SAssetEditorViewport.h"


/**
 * Viewport used for live preview in FDOverlay editor. Has a custom toolbar overlay at the top.
 */
class FDOVERLAYEDITOR_API SFDOverlay3DViewport : public SAssetEditorViewport
{
public:
	// These allow the toolkit to add an accept/cancel overlay when needed. PopulateViewportOverlays
	// is not helpful here because that gets called just once.
	virtual void AddOverlayWidget(TSharedRef<SWidget> OverlaidWidget);
	virtual void RemoveOverlayWidget(TSharedRef<SWidget> OverlaidWidget);

	// SAssetEditorViewport
	virtual void BindCommands() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
};