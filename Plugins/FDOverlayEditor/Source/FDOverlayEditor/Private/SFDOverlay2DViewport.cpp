#include "SFDOverlay2DViewport.h"

#include "FDOverlay2DViewportClient.h"

#include "SViewportToolBar.h"


void SFDOverlay2DViewport::BindCommands()
{
	SAssetEditorViewport::BindCommands();
}

void SFDOverlay2DViewport::AddOverlayWidget(TSharedRef<SWidget> OverlaidWidget)
{
	ViewportOverlay->AddSlot()
		[
			OverlaidWidget
		];
}


void SFDOverlay2DViewport::RemoveOverlayWidget(TSharedRef<SWidget> OverlaidWidget)
{
	ViewportOverlay->RemoveSlot(OverlaidWidget);
}

TSharedPtr<SWidget> SFDOverlay2DViewport::MakeViewportToolbar()
{
	return SNew(SViewportToolBar);// SUVEditor2DViewportToolBar
		//.CommandList(CommandList)
		//.Viewport2DClient(StaticCastSharedPtr<FFDOverlay2DViewportClient>(Client));
}

bool SFDOverlay2DViewport::IsWidgetModeActive(UE::Widget::EWidgetMode Mode) const
{
	return static_cast<FFDOverlay2DViewportClient*>(Client.Get())->AreWidgetButtonsEnabled()
		&& Client->GetWidgetMode() == Mode;
}