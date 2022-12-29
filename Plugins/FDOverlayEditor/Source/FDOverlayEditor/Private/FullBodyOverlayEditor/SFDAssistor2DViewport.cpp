#include "FullBodyOverlayEditor/SFDAssistor2DViewport.h"

#include "FullBodyOverlayEditor/FDAssistor2DViewportClient.h"

#include "SViewportToolBar.h"


void SFDAssistor2DViewport::BindCommands()
{
	SAssetEditorViewport::BindCommands();
}

void SFDAssistor2DViewport::AddOverlayWidget(TSharedRef<SWidget> OverlaidWidget)
{
	ViewportOverlay->AddSlot()
		[
			OverlaidWidget
		];
}


void SFDAssistor2DViewport::RemoveOverlayWidget(TSharedRef<SWidget> OverlaidWidget)
{
	ViewportOverlay->RemoveSlot(OverlaidWidget);
}

TSharedPtr<SWidget> SFDAssistor2DViewport::MakeViewportToolbar()
{
	return SNew(SViewportToolBar);// SUVEditor2DViewportToolBar
		//.CommandList(CommandList)
		//.Viewport2DClient(StaticCastSharedPtr<FFDAssistor2DViewportClient>(Client));
}

bool SFDAssistor2DViewport::IsWidgetModeActive(UE::Widget::EWidgetMode Mode) const
{
	return static_cast<FFDAssistor2DViewportClient*>(Client.Get())->AreWidgetButtonsEnabled()
		&& Client->GetWidgetMode() == Mode;
}