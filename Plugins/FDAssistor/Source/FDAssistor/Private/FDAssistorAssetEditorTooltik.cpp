#include "FDAssistorAssetEditorTooltik.h"

#include "FDAssistorAssetEditor.h"
#include "FDAssistorModeUILayer.h"

#define LOCTEXT_NAMESPACE "FDAssistorAssetEditorToolkit"


FFDAssistorAssetEditorToolkit::FFDAssistorAssetEditorToolkit(UAssetEditor* InOwningAssetEditor) :FBaseAssetToolkit(InOwningAssetEditor)
{
	check(Cast<UFDAssistorAssetEditor>(InOwningAssetEditor));
}

FFDAssistorAssetEditorToolkit::~FFDAssistorAssetEditorToolkit()
{
	
}

void FFDAssistorAssetEditorToolkit::PostInitAssetEditor() 
{
	// 我们在这里设置了ModeUILayer连接，因为InitAssetEditor对我们是关闭的。
	// 其他编辑器在其他地方执行此步骤，但这是我们的最佳位置。
	TSharedPtr<class IToolkitHost> PinnedToolkitHost = ToolkitHost.Pin();
	check(PinnedToolkitHost.IsValid());
	ModeUILayer = MakeShareable(new FFDAssistorEditorModeUILayer(PinnedToolkitHost.Get()));
	ModeUILayer->SetModeMenuCategory(FDAssistorEditorMenuCategory);

	// 
	TArray<TObjectPtr<UObject>> ObjectsToEdit;
	OwningAssetEditor->GetObjectsToEdit(ObjectsToEdit);

	TArray<FTransform> ObjectTransforms;
	ObjectTransforms.SetNum(ObjectsToEdit.Num());

	// 这个静态方法初始化了各种上下文，使得 Mode在Enter()的 ContextStore 中可用才能正常工作。
	UUVEditorMode::InitializeAssetEditorContexts(*EditorModeManager->GetInteractiveToolsContext()->ContextObjectStore,
		ObjectsToEdit, ObjectTransforms, *LivePreviewViewportClient, *LivePreviewEditorModeManager,
		*ViewportButtonsAPI, *FDAssistorLive2DViewportAPI);

	// 目前，除了设置所有的UI元素，工具箱还启动了FD编辑器模式，这是编辑器始终工作的模式(东西被打包到一个模式中，以便在必要时可以移动到另一个资产编辑器中)。
	// 我们需要激活UV模式来创建左侧的工具箱。
	check(EditorModeManager.IsValid());
	EditorModeManager->ActivateMode(UFDAssistorEditorMode::EM_FDAssistorModeId);
	UFDAssistorEditorMode* FDAssistoMode = Cast<UFDAssistorEditorMode>(
		EditorModeManager->GetActiveScriptableMode(UFDAssistorEditorMode::EM_FDAssistorModeId));
	check(FDAssistoMode);

	// 不管用户如何修改布局，我们将确保我们有一个2d视口，这将允许我们的模式接收tick。
	// 我们不再需要调用工具面板选项卡，因为这是由底层基础设施处理的。
	if (!TabManager->FindExistingLiveTab(ViewportTabID))
	{
		TabManager->TryInvokeTab(ViewportTabID);
	}

	// 添加"Apply Changes"按钮.实际上，几乎在任何时候这样做都是安全的，甚至在工具栏注册之前，但将大多数内容放入PostInitAssetEditor()更容易。
	// TODO:我们可以考虑将这些绑定到模式的操作放到一些模式操作列表中，然后让模式在进入/退出时将它们提供给拥有的资产编辑器。当/如果这变得更容易做的时候，重新审视。
	// ToolkitCommands: List of UI commands for this toolkit.   This should be filled in by the derived class!
	ToolkitCommands->MapAction(
		FFDAssistorEditorModeCommands::Get().ApplyChanges,
		FExecuteAction::CreateUObject(UVMode, &UFDAssistorEditorMode::ApplyChanges),
		FCanExecuteAction::CreateUObject(UVMode, &UFDAssistorEditorMode::CanApplyChanges));
	FName ParentToolbarName;
	const FName ToolBarName = GetToolMenuToolbarName(ParentToolbarName);
	UToolMenu* AssetToolbar = UToolMenus::Get()->ExtendMenu(ToolBarName);
	FToolMenuSection& Section = AssetToolbar->FindOrAddSection("Asset");
	Section.AddEntry(FToolMenuEntry::InitToolBarButton(FFDAssistorEditorModeCommands::Get().ApplyChanges));

	// Add the channel selection button.
	// TODO：这个没必要实现，不过是否可以把这个替换为 材质ID Selection button ？
	/*check(UVMode->GetToolkit().Pin());
	FUVEditorModeToolkit* UVModeToolkit = static_cast<FUVEditorModeToolkit*>(UVMode->GetToolkit().Pin().Get());
	Section.AddEntry(FToolMenuEntry::InitComboButton(
		"UVEditorChannelMenu",
		FUIAction(),
		FOnGetContent::CreateLambda([UVModeToolkit]()
			{
				return UVModeToolkit->CreateChannelMenu();
			}),
		LOCTEXT("UVEditorChannelMenu_Label", "Channels"),
				LOCTEXT("UVEditorChannelMenu_ToolTip", "Select the current UV Channel for each mesh"),
				FSlateIcon(FUVEditorStyle::Get().GetStyleSetName(), "UVEditor.ChannelSettings")
				));*/




	// Uniform adjustment
	auto SetCommonViewportClientOptions = [](FEditorViewportClient* Client)
	{
		// 通常，bIsRealtime标志是由是否是远程连接决定的，但我们的工具需要始终勾选。
		Client->SetRealtime(true);

		// 禁用动态模糊效果，导致我们的渲染 “fade”，因为东西是移动的
		Client->EngineShowFlags.SetTemporalAA(false);
		Client->EngineShowFlags.SetMotionBlur(false);

		// 禁用 gizmos 被遮挡部分的dithering 
		Client->EngineShowFlags.SetOpaqueCompositeEditorPrimitives(true);

		// 禁用硬件遮挡查询，这使得使用顶点着色器将材料拉向摄像机进行z排序变得更加困难，
		// 因为非半透明材料开始遮挡自己(一旦组件边界位于被置换的几何图形后面)。
		Client->EngineShowFlags.SetDisableOcclusionQueries(true);
	};

	// Adjust our main (2D) viewport:
	{
		SetCommonViewportClientOptions(ViewportClient.Get());

		// 不幸的是 Ortho 在渲染东西方面有太多问题，所以我们应该使用透视。
		ViewportClient->SetViewportType(ELevelViewportType::LVT_Perspective);

		// 在我们可以使用的材料方面，Lit 给了我们最多的选择。
		ViewportClient->SetViewMode(EViewModeIndex::VMI_Lit);

		// scale [0,1] to [0,ScaleFactor]
		// 我们设置我们的相机向下看，居中，足够远，能够看到边缘与90度FOV
		double ScaleFactor = 1;
		UFDAssistorEditorSubsystem* FDAssistorSubsystem = GEditor->GetEditorSubsystem<UFDAssistorEditorSubsystem>();
		if (FDAssistorSubsystem)
		{
			ScaleFactor = FUVEditorUXSettings::UVMeshScalingFactor;
		}
		ViewportClient->SetViewLocation(FVector(ScaleFactor / 2, ScaleFactor / 2, ScaleFactor));
		ViewportClient->SetViewRotation(FRotator(-90, 0, 0));

		// 如果曝光没有设置为固定，它会在我们凝视虚空时闪烁
		ViewportClient->ExposureSettings.bFixed = true;

		// 我们需要视口客户端开始聚焦，否则它不会被选中，直到我们点击它里面。
		ViewportClient->ReceivedFocus(ViewportClient->Viewport);
	}
	
	// Adjust our live preview (3D) viewport
	{
		SetCommonViewportClientOptions(LivePreviewViewportClient.Get());
		LivePreviewViewportClient->ToggleOrbitCamera(true);

		// TODO: This should not be hardcoded
		LivePreviewViewportClient->SetViewLocation(FVector(-200, 100, 100));
		LivePreviewViewportClient->SetLookAtLocation(FVector(0, 0, 0));
	}

	// Adjust camera view to focus on the scene
	FDAssistoMode->FocusLivePreviewCameraOnSelection();

	// 将视口命令列表连接到我们的工具箱命令列表，这样工具箱未处理的热键将由视口处理(允许我们使用在详细信息面板或FD编辑器的其他地方单击后视口注册的任何热键)。
	// 注意，命令列表的“Append”调用可能应该被称为“AppendTo”，因为它将被调用对象添加为参数命令列表的子对象。
	// 也就是说，在 ToolkitCommands中查找之后，我们将在 2DviewportCommands中查找。
	TSharedPtr<SFDAssistor2DViewport> ViewportWidget = StaticCastSharedPtr<SFDAssistor2DViewport>(ViewportTabContent->GetFirstViewport());
	ViewportWidget->GetCommandList()->Append(ToolkitCommands);

	
}

#undef LOCTEXT_NAMESPACE