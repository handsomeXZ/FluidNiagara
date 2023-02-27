#include "SWidget/SFDOverlay2DViewportToolBar.h"

#include "FDOverlayEditorModeCommands.h"
#include "Tools/FDOverlayStyle.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "SEditorViewportToolBarMenu.h"
#include "SEditorViewportViewMenu.h"
#include "SWidget/SFDOverlay2DViewport.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SFDOverlay2DViewportToolBar"

void SFDOverlay2DViewportToolBar::Construct(const FArguments& InArgs, TSharedPtr<class SFDOverlay2DViewport> FDOverlay2DViewportIn)
{
	FDOverlay2DViewportPtr = FDOverlay2DViewportIn;
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
		];

	MainBoxPtr->AddSlot()
		.Padding(ToolbarSlotPadding)
		.HAlign(HAlign_Left)
		[
			MakeToolBar(InArgs._Extenders)
		];


	SViewportToolBar::Construct(SViewportToolBar::FArguments());
}

TSharedRef<SWidget> SFDOverlay2DViewportToolBar::MakeToolBar(const TSharedPtr<FExtender> ExtendersIn)
{
	// 按钮通过 SFDOverlay2DViewport::BindCommands() 中的命令绑定连接到实际功能，
	// 工具栏在 SFDOverlay2DViewport::MakeViewportToolbar() 中构建。

	FSlimHorizontalToolBarBuilder ToolbarBuilder(CommandList, FMultiBoxCustomization::None, ExtendersIn);

	// Use a custom style !!!!!!!!!!!
	FName ToolBarStyle = "EditorViewportToolBar";
	ToolbarBuilder.SetStyle(&FAppStyle::Get(), ToolBarStyle);
	ToolbarBuilder.SetLabelVisibility(EVisibility::Collapsed);

	ToolbarBuilder.BeginSection("MaterialID_DisplayStyle");
	{
		ToolbarBuilder.BeginBlockGroup();

		// Compact
		static FName DisplayStyleName1 = FName(TEXT("Compact"));
		ToolbarBuilder.AddToolBarButton(FFDOverlayEditorModeCommands::Get().Compact, NAME_None, TAttribute<FText>(), TAttribute<FText>(),
			TAttribute<FSlateIcon>(FSlateIcon(FFDOverlayStyle::Get().GetStyleSetName(), "FDOverlay.Compact")), DisplayStyleName1);

		// Iterable
		static FName DisplayStyleName2 = FName(TEXT("Iterable"));
		ToolbarBuilder.AddToolBarButton(FFDOverlayEditorModeCommands::Get().Iterable, NAME_None, TAttribute<FText>(), TAttribute<FText>(),
			TAttribute<FSlateIcon>(FSlateIcon(FFDOverlayStyle::Get().GetStyleSetName(), "FDOverlay.Iterable")), DisplayStyleName2);

		// Exploded
		static FName DisplayStyleName3 = FName(TEXT("Exploded"));
		ToolbarBuilder.AddToolBarButton(FFDOverlayEditorModeCommands::Get().Exploded, NAME_None, TAttribute<FText>(), TAttribute<FText>(),
			TAttribute<FSlateIcon>(FSlateIcon(FFDOverlayStyle::Get().GetStyleSetName(), "FDOverlay.Exploded")), DisplayStyleName3);


		ToolbarBuilder.EndBlockGroup();

	}

	ToolbarBuilder.EndSection();

	return ToolbarBuilder.MakeWidget();


}



#undef LOCTEXT_NAMESPACE