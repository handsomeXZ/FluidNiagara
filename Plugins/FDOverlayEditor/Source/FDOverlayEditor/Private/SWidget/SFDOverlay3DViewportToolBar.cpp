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
			SAssignNew(MainBoxPtr, SHorizontalBox)	// SAssignNew �� SNew ����������ҵ���ȸ
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
			MakeToolBar(InArgs._Extenders)
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

TSharedRef<SWidget> SFDOverlay3DViewportToolBar::MakeToolBar(const TSharedPtr<FExtender> ExtendersIn)
{
	// ��ťͨ�� SFDOverlay3DViewport::BindCommands() �е���������ӵ�ʵ�ʹ��ܣ�
	// �������� SFDOverlay3DViewport::MakeViewportToolbar() �й�����

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

#undef LOCTEXT_NAMESPACE