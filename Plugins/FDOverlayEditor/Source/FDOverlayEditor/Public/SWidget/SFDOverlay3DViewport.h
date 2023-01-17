
#pragma once

#include "CoreMinimal.h"

#include "SAssetEditorViewport.h"


/**
 * Viewport used for live preview in FDOverlay editor. Has a custom toolbar overlay at the top.
 */
class FDOVERLAYEDITOR_API SFDOverlay3DViewport : public SAssetEditorViewport
{
public:

	// SAssetEditorViewport
	virtual void BindCommands() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
};