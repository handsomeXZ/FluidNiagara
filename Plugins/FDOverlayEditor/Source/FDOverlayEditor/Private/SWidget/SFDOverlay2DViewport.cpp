// Copyright HandsomeCheese. All Rights Reserved.
#include "SWidget/SFDOverlay2DViewport.h"

#include "FDOverlay2DViewportClient.h"
#include "SWidget/SFDOverlay2DViewportToolBar.h"
#include "SWidget/SFDOverlay2DViewportSwicthOverlay.h"
//#include "SViewportToolBar.h"
#include "FDOverlayEditorModeCommands.h"
#include "Tools/FDOverlayStyle.h"

void SFDOverlay2DViewport::BindCommands()
{
	SAssetEditorViewport::BindCommands();

	const FFDOverlayEditorModeCommands& CommandInfos = FFDOverlayEditorModeCommands::Get();

	CommandList->MapAction(
		CommandInfos.Compact,
		FExecuteAction::CreateLambda([this]() {
			StaticCastSharedPtr<FFDOverlay2DViewportClient>(Client)->ToggleDisplayMode(EFDOverlay2DViewportClientDisplayMode::Compact);
			}),
		FCanExecuteAction::CreateLambda([this]() { return true; }),
		FIsActionChecked::CreateLambda([this]() { return StaticCastSharedPtr<FFDOverlay2DViewportClient>(Client)->GetDisplayMode(EFDOverlay2DViewportClientDisplayMode::Compact); })
	);
	CommandList->MapAction(
		CommandInfos.Iterable,
		FExecuteAction::CreateLambda([this]() {
			StaticCastSharedPtr<FFDOverlay2DViewportClient>(Client)->ToggleDisplayMode(EFDOverlay2DViewportClientDisplayMode::Iterable);
			}),
		FCanExecuteAction::CreateLambda([this]() { return true; }),
		FIsActionChecked::CreateLambda([this]() { return StaticCastSharedPtr<FFDOverlay2DViewportClient>(Client)->GetDisplayMode(EFDOverlay2DViewportClientDisplayMode::Iterable); })
	);
	CommandList->MapAction(
		CommandInfos.Exploded,
		FExecuteAction::CreateLambda([this]() {
			//StaticCastSharedPtr<FFDOverlay2DViewportClient>(Client)->ToggleDisplayMode(EFDOverlay2DViewportClientDisplayMode::Exploded);
			}),
		FCanExecuteAction::CreateLambda([this]() { return true; }),
		FIsActionChecked::CreateLambda([this]() { return StaticCastSharedPtr<FFDOverlay2DViewportClient>(Client)->GetDisplayMode(EFDOverlay2DViewportClientDisplayMode::Exploded); })
	);

	
	AddOverlayWidget(SNew(SFDSwitchOverlayBox)
		.IsEnabled_Lambda([this]() { return true; })
		.Visibility_Lambda([this]() { return StaticCastSharedPtr<FFDOverlay2DViewportClient>(Client)->GetDisplayMode(EFDOverlay2DViewportClientDisplayMode::Iterable) ? EVisibility::Visible : EVisibility::Collapsed; })
		.Icon_Lambda([this]() { return FFDOverlayStyle::Get().GetOptionalBrush("FDOverlay.Compact"); })
		.OnSwitchOverlayAdd_Lambda([this]() { StaticCastSharedPtr<FFDOverlay2DViewportClient>(Client)->ExecuteOnMaterialIDAdd(); UE_LOG(LogTemp, Warning, TEXT("Success")); return FReply::Handled(); })
		.OnSwitchOverlaySub_Lambda([this]() { StaticCastSharedPtr<FFDOverlay2DViewportClient>(Client)->ExecuteOnMaterialIDSub(); UE_LOG(LogTemp, Warning, TEXT("Success")); return FReply::Handled(); })
	);


}

void SFDOverlay2DViewport::AddOverlayWidget(TSharedRef<SWidget> OverlaidWidget)
{
	ViewportOverlay->AddSlot()
		[
			OverlaidWidget	// 由 FFDOverlayEditorModeToolkit 创建，借助 FFDOverlayAssetEditorToolkit 传入
		];
}


void SFDOverlay2DViewport::RemoveOverlayWidget(TSharedRef<SWidget> OverlaidWidget)
{
	ViewportOverlay->RemoveSlot(OverlaidWidget);
}

TSharedPtr<SWidget> SFDOverlay2DViewport::MakeViewportToolbar()
{
	/*return SNew(SViewportToolBar);*/
	return SNew(SFDOverlay2DViewportToolBar, SharedThis(this))
		.CommandList(CommandList);

}

bool SFDOverlay2DViewport::IsWidgetModeActive(UE::Widget::EWidgetMode Mode) const
{
	return Client->GetWidgetMode() == Mode;
}