#pragma once

#include "CoreMinimal.h"

#include "SViewportToolBar.h"

class FExtender;
class FUICommandList;
class SFDOverlay3DViewport;

/**
 * Toolbar that shows up at the top of the 3d viewport
 */
class SFDOverlay3DViewportToolBar : public SViewportToolBar
{
public:
	SLATE_BEGIN_ARGS(SFDOverlay3DViewportToolBar) {}
		SLATE_ARGUMENT(TSharedPtr<FUICommandList>, CommandList)
		SLATE_ARGUMENT(TSharedPtr<FExtender>, Extenders)
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, TSharedPtr<class SFDOverlay3DViewport> FDOverlay3DViewportIn);

private:
	TSharedRef<SWidget> MakeDisplayToolBar(const TSharedPtr<FExtender> ExtendersIn);
	TSharedRef<SWidget> MakeToolBar(const TSharedPtr<FExtender> ExtendersIn);

	/** The viewport that we are in */
	TWeakPtr<class SFDOverlay3DViewport> FDOverlay3DViewportPtr;

	TSharedPtr<FUICommandList> CommandList;
};
