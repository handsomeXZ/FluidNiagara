// Copyright HandsomeCheese. All Rights Reserved.
#include "SWidget/SFDOverlay3DViewport.h"

#include "SWidget/SFDOverlay3DViewportToolBar.h"
#include "FDOverlayEditorModeCommands.h"
#include "FDOverlay3DViewportClient.h"

#include "EditorViewportClient.h"

#define LOCTEXT_NAMESPACE "SFDOverlay3DViewport"

void SFDOverlay3DViewport::BindCommands()
{
	SAssetEditorViewport::BindCommands();

	const FFDOverlayEditorModeCommands& CommandInfos = FFDOverlayEditorModeCommands::Get();
	//============================================================
	CommandList->MapAction(
		CommandInfos.XChannel,
		FExecuteAction::CreateLambda([this]() {
			StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->ToggleDisplayMode(EFDOverlay3DViewportClientDisplayMode::X);
			}),
		FCanExecuteAction::CreateLambda([this]() { return true; }),
		FIsActionChecked::CreateLambda([this]() { return StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->GetDisplayMode(EFDOverlay3DViewportClientDisplayMode::X); })
	);
	CommandList->MapAction(
		CommandInfos.YChannel,
		FExecuteAction::CreateLambda([this]() {
			StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->ToggleDisplayMode(EFDOverlay3DViewportClientDisplayMode::Y);
			}),
		FCanExecuteAction::CreateLambda([this]() { return true; }),
		FIsActionChecked::CreateLambda([this]() { return StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->GetDisplayMode(EFDOverlay3DViewportClientDisplayMode::Y); })
	);
	CommandList->MapAction(
		CommandInfos.ZChannel,
		FExecuteAction::CreateLambda([this]() {
			StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->ToggleDisplayMode(EFDOverlay3DViewportClientDisplayMode::Z);
			}),
		FCanExecuteAction::CreateLambda([this]() { return true; }),
		FIsActionChecked::CreateLambda([this]() { return StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->GetDisplayMode(EFDOverlay3DViewportClientDisplayMode::Z); })
	);
	CommandList->MapAction(
		CommandInfos.WChannel,
		FExecuteAction::CreateLambda([this]() {
			StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->ToggleDisplayMode(EFDOverlay3DViewportClientDisplayMode::W);
			}),
		FCanExecuteAction::CreateLambda([this]() { return true; }),
		FIsActionChecked::CreateLambda([this]() { return StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->GetDisplayMode(EFDOverlay3DViewportClientDisplayMode::W); })
	);
	//============================================================
	CommandList->MapAction(
		CommandInfos.DefaultLight,
		FExecuteAction::CreateLambda([this]() {
			StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->ToggleRenderMode(EFDOverlay3DViewportClientRenderMode::DefaultLight);
			}),
		FCanExecuteAction::CreateLambda([this]() { return true; }),
		FIsActionChecked::CreateLambda([this]() { return StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->GetRenderMode(EFDOverlay3DViewportClientRenderMode::DefaultLight); })
	);
	CommandList->MapAction(
		CommandInfos.Emissive,
		FExecuteAction::CreateLambda([this]() {
			StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->ToggleRenderMode(EFDOverlay3DViewportClientRenderMode::Emissive);
			}),
		FCanExecuteAction::CreateLambda([this]() { return true; }),
		FIsActionChecked::CreateLambda([this]() { return StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->GetRenderMode(EFDOverlay3DViewportClientRenderMode::Emissive); })
	);
	CommandList->MapAction(
		CommandInfos.Translucency,
		FExecuteAction::CreateLambda([this]() {
			StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->ToggleRenderMode(EFDOverlay3DViewportClientRenderMode::Translucency);
			}),
		FCanExecuteAction::CreateLambda([this]() { return true; }),
		FIsActionChecked::CreateLambda([this]() { return StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->GetRenderMode(EFDOverlay3DViewportClientRenderMode::Translucency); })
	);
	CommandList->MapAction(
		CommandInfos.Transition,
		FExecuteAction::CreateLambda([this]() {
			StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->ToggleRenderMode(EFDOverlay3DViewportClientRenderMode::Transition);
			}),
		FCanExecuteAction::CreateLambda([this]() { return true; }),
		FIsActionChecked::CreateLambda([this]() { return StaticCastSharedPtr<FFDOverlay3DViewportClient>(Client)->GetRenderMode(EFDOverlay3DViewportClientRenderMode::Transition); })
	);
	//============================================================
}

TSharedPtr<SWidget> SFDOverlay3DViewport::MakeViewportToolbar()
{
	return SNew(SFDOverlay3DViewportToolBar, SharedThis(this))
		.CommandList(CommandList);
}

void SFDOverlay3DViewport::AddOverlayWidget(TSharedRef<SWidget> OverlaidWidget)
{
	ViewportOverlay->AddSlot()
		[
			OverlaidWidget	// 由 FFDOverlayEditorModeToolkit 创建，借助 FFDOverlayAssetEditorToolkit 传入
		];
}


void SFDOverlay3DViewport::RemoveOverlayWidget(TSharedRef<SWidget> OverlaidWidget)
{
	ViewportOverlay->RemoveSlot(OverlaidWidget);
}
#undef LOCTEXT_NAMESPACE