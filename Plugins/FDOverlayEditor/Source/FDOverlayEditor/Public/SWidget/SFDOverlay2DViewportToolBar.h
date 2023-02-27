#pragma once

#include "CoreMinimal.h"

#include "SViewportToolBar.h"

class FExtender;
class FUICommandList;
class SFDOverlay2DViewport;

/**
 * Toolbar that shows up at the top of the 2d viewport
 */
class SFDOverlay2DViewportToolBar : public SViewportToolBar
{
public:
	SLATE_BEGIN_ARGS(SFDOverlay2DViewportToolBar) {}
	SLATE_ARGUMENT(TSharedPtr<FUICommandList>, CommandList)
		SLATE_ARGUMENT(TSharedPtr<FExtender>, Extenders)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, TSharedPtr<class SFDOverlay2DViewport> FDOverlay2DViewportIn);

private:
	TSharedRef<SWidget> MakeToolBar(const TSharedPtr<FExtender> ExtendersIn);
	/** The viewport that we are in */
	TWeakPtr<class SFDOverlay2DViewport> FDOverlay2DViewportPtr;

	TSharedPtr<FUICommandList> CommandList;
};
