// Copyright HandsomeCheese. All Rights Reserved.
#include "SWidget/SFDOverlay3DViewportToolBar.h"

#include "FDOverlayEditorModeCommands.h"
#include "Tools/FDOverlayStyle.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "SEditorViewportToolBarMenu.h"
#include "SEditorViewportViewMenu.h"
#include "SWidget/SFDOverlay3DViewport.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SFDOverlay3DViewportToolBar"

void SFDOverlay3DViewportToolBar::Construct(const FArguments& InArgs, TSharedPtr<class SFDOverlay3DViewport> FDOverlay3DViewportIn)
{
	FDOverlay3DViewportPtr = FDOverlay3DViewportIn;
	CommandList = InArgs._CommandList;

	const FMargin ToolbarSlotPadding(4.0f, 1.0f);
	TSharedPtr<SHorizontalBox> MainBoxPtr;


	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::Get().GetBrush("EditorViewportToolBar.Background"))
		.Cursor(EMouseCursor::Default)
		[
			SAssignNew(MainBoxPtr, SHorizontalBox)	// SAssignNew 和 SNew 的区别，详见我的语雀
		]
	] ;
	
	MainBoxPtr->AddSlot()
		.Padding(ToolbarSlotPadding)
		.HAlign(HAlign_Left)
		[
			MakeDisplayToolBar(InArgs._Extenders)
		];

	MainBoxPtr->AddSlot()
		.Padding(ToolbarSlotPadding)
		.HAlign(HAlign_Center)
		[
			MakeCenterToolBar(InArgs._Extenders)
		];

	MainBoxPtr->AddSlot()
		.Padding(ToolbarSlotPadding)
		.HAlign(HAlign_Right)
		[
			MakeRightToolBar(InArgs._Extenders)
		];

	SViewportToolBar::Construct(SViewportToolBar::FArguments());
}

TSharedRef<SWidget> SFDOverlay3DViewportToolBar::MakeDisplayToolBar(const TSharedPtr<FExtender> ExtendersIn)
{
	TSharedRef<SEditorViewport> ViewportRef = StaticCastSharedPtr<SEditorViewport>(FDOverlay3DViewportPtr.Pin()).ToSharedRef();

	return SNew(SEditorViewportViewMenu, ViewportRef, SharedThis(this))
		.Cursor(EMouseCursor::Default)
		.MenuExtenders(ExtendersIn);
}

TSharedRef<SWidget> SFDOverlay3DViewportToolBar::MakeRightToolBar(const TSharedPtr<FExtender> ExtendersIn)
{
	// 按钮通过 SFDOverlay3DViewport::BindCommands() 中的命令绑定连接到实际功能，
	// 工具栏在 SFDOverlay3DViewport::MakeViewportToolbar() 中构建。

	FSlimHorizontalToolBarBuilder ToolbarBuilder(CommandList, FMultiBoxCustomization::None, ExtendersIn);

	// Use a custom style !!!!!!!!!!!
	FName ToolBarStyle = "EditorViewportToolBar";
	ToolbarBuilder.SetStyle(&FAppStyle::Get(), ToolBarStyle);
	ToolbarBuilder.SetLabelVisibility(EVisibility::Collapsed);

	ToolbarBuilder.BeginSection("XYZWChannel");
	{
		ToolbarBuilder.BeginBlockGroup();

		// XChannel
		static FName XChannelName = FName(TEXT("XChannel"));
		ToolbarBuilder.AddToolBarButton(FFDOverlayEditorModeCommands::Get().XChannel, NAME_None, TAttribute<FText>(), TAttribute<FText>(),
			TAttribute<FSlateIcon>(FSlateIcon(FFDOverlayStyle::Get().GetStyleSetName(), "FDOverlay.XChannel")), XChannelName);

		// YChannel
		static FName YChannelName = FName(TEXT("YChannel"));
		ToolbarBuilder.AddToolBarButton(FFDOverlayEditorModeCommands::Get().YChannel, NAME_None, TAttribute<FText>(), TAttribute<FText>(),
			TAttribute<FSlateIcon>(FSlateIcon(FFDOverlayStyle::Get().GetStyleSetName(), "FDOverlay.YChannel")), YChannelName);

		// ZChannel
		static FName ZChannelName = FName(TEXT("ZChannel"));
		ToolbarBuilder.AddToolBarButton(FFDOverlayEditorModeCommands::Get().ZChannel, NAME_None, TAttribute<FText>(), TAttribute<FText>(),
			TAttribute<FSlateIcon>(FSlateIcon(FFDOverlayStyle::Get().GetStyleSetName(), "FDOverlay.ZChannel")), ZChannelName);

		// WChannel
		static FName WChannelName = FName(TEXT("WChannel"));
		ToolbarBuilder.AddToolBarButton(FFDOverlayEditorModeCommands::Get().WChannel, NAME_None, TAttribute<FText>(), TAttribute<FText>(),
			TAttribute<FSlateIcon>(FSlateIcon(FFDOverlayStyle::Get().GetStyleSetName(), "FDOverlay.WChannel")), WChannelName);

		ToolbarBuilder.EndBlockGroup();

	}

	ToolbarBuilder.EndSection();

	return ToolbarBuilder.MakeWidget();


}

TSharedRef<SWidget> SFDOverlay3DViewportToolBar::MakeCenterToolBar(const TSharedPtr<FExtender> ExtendersIn)
{
	FSlimHorizontalToolBarBuilder ToolbarBuilder(CommandList, FMultiBoxCustomization::None, ExtendersIn);

	FName ToolBarStyle = "EditorViewportToolBar";
	ToolbarBuilder.SetStyle(&FAppStyle::Get(), ToolBarStyle);
	ToolbarBuilder.SetLabelVisibility(EVisibility::Collapsed);

	ToolbarBuilder.BeginSection("MeshRenderMenu");
	{
		ToolbarBuilder.BeginBlockGroup();

		// Default lighting 
		static FName DefaultLightName = FName(TEXT("DefaultLight"));
		ToolbarBuilder.AddToolBarButton(FFDOverlayEditorModeCommands::Get().DefaultLight, NAME_None, TAttribute<FText>(), TAttribute<FText>(),
			TAttribute<FSlateIcon>(FSlateIcon(FFDOverlayStyle::Get().GetStyleSetName(), "FDOverlay.DefaultLight")), DefaultLightName);

		// Emissive
		static FName EmissiveName = FName(TEXT("Emissive"));
		ToolbarBuilder.AddToolBarButton(FFDOverlayEditorModeCommands::Get().Emissive, NAME_None, TAttribute<FText>(), TAttribute<FText>(),
			TAttribute<FSlateIcon>(FSlateIcon(FFDOverlayStyle::Get().GetStyleSetName(), "FDOverlay.Emissive")), EmissiveName);

		// Default lighting 
		static FName TranslucencyName = FName(TEXT("Translucency"));
		ToolbarBuilder.AddToolBarButton(FFDOverlayEditorModeCommands::Get().Translucency, NAME_None, TAttribute<FText>(), TAttribute<FText>(),
			TAttribute<FSlateIcon>(FSlateIcon(FFDOverlayStyle::Get().GetStyleSetName(), "FDOverlay.Translucency")), TranslucencyName);

		static FName TransitionName = FName(TEXT("Transition"));
		ToolbarBuilder.AddToolBarButton(FFDOverlayEditorModeCommands::Get().Transition, NAME_None, TAttribute<FText>(), TAttribute<FText>(),
			TAttribute<FSlateIcon>(FSlateIcon(FFDOverlayStyle::Get().GetStyleSetName(), "FDOverlay.Transition")), TransitionName);

		ToolbarBuilder.EndBlockGroup();

	}

	ToolbarBuilder.EndSection();

	return ToolbarBuilder.MakeWidget();


}

#undef LOCTEXT_NAMESPACE