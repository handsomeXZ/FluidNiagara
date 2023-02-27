#include "FDOverlayAssetEditorTooltik.h"

#include "FDOverlayAssetEditor.h"
#include "FDOverlayModeUILayer.h"
#include "FDOverlay2DViewportClient.h"
#include "FDOverlay3DViewportClient.h"
#include "FDOverlayEditorSubsystem.h"
#include "FDOverlayEditorModule.h"
#include "FDOverlayEditorMode.h"
#include "FDOverlayEditorModeCommands.h"
#include "SWidget/SFDOverlay2DViewport.h"
#include "SWidget/SFDOverlay3DViewport.h"
#include "Tools/FDOverlayStyle.h"

#include "Framework\Commands\UIAction.h"
#include "PreviewScene.h"
#include "AdvancedPreviewScene.h"
#include "EditorViewportTabContent.h"
#include "AssetEditorModeManager.h"
#include "EdModeInteractiveToolsContext.h"
#include "UVEditorUXSettings.h"


#define LOCTEXT_NAMESPACE "FDOverlayAssetEditorToolkit"
const FName FFDOverlayAssetEditorToolkit::Live3DPreviewTabID(TEXT("FDEditor_Live3DPreviewTab"));

FFDOverlayAssetEditorToolkit::FFDOverlayAssetEditorToolkit(UAssetEditor* InOwningAssetEditor)
	:FBaseAssetToolkit(InOwningAssetEditor)
{
	check(Cast<UFDOverlayAssetEditor>(InOwningAssetEditor));

	// 我们将替换父类给我们的StandaloneDefaultLayout，在这个布局中，属性详细面板是左边的一个垂直列，右边有两个视口。
	// 我们在堆栈上定义显式的ExtensionIds，以便以后在UILayer提供布局扩展时引用它们。 
	// 注意：对布局的改变应该包括对布局ID的增量，即UVEditorLayout[X] -> UVEditorLayout[X+1]。
	// 否则，布局可能会被打乱 没有完全重置为编辑器内部的布局默认值。
	StandaloneDefaultLayout = FTabManager::NewLayout(FName("FDOverlayLayout"))
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.2f)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.4f)
						->AddTab(ViewportTabID, ETabState::OpenedTab)	// ViewportTabID 之后在 RegisterTabSpawners() 进行设置
						->SetExtensionId("Viewport2DArea")
						->SetHideTabWell(true)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.6f)
						->SetExtensionId("EditorSidePanelArea")
						->SetHideTabWell(true)
					)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.8f)
					->AddTab(Live3DPreviewTabID, ETabState::OpenedTab) // Live3DPreviewTabID 之后在 RegisterTabSpawners() 进行设置
					->SetExtensionId("Viewport3DArea")
					->SetHideTabWell(true)
				)
				
			)
		);



	// 添加任何由UStaticMeshEditorUISubsystem指定的扩展器。
	// 这些扩展器为FModeToolkit提供了定义的位置，使其能够连接到 工具调色板标签和细节面板标签。
	LayoutExtender = MakeShared<FLayoutExtender>();
	FFDOverlayEditorModule* FDOverlayEditorModule = &FModuleManager::LoadModuleChecked<FFDOverlayEditorModule>("FDOverlayEditor");
	FDOverlayEditorModule->OnRegisterLayoutExtensions().Broadcast(*LayoutExtender);
	StandaloneDefaultLayout->ProcessExtensions(*LayoutExtender);


	// 该API对象作为视口工具栏和工具之间的通信点。
	// 我们在这里创建它，以便我们可以在初始化模式时将它传递到2d和3d视口。



	//我们可以在CreateEditorViewportClient()中创建预览场景，就像FBaseAssetToolkit那样，但它似乎更直观地创建他们立即并稍后传入。
	FPreviewScene::ConstructionValues PreviewSceneArgs;
	UnwrapScene = MakeUnique<FPreviewScene>(PreviewSceneArgs);
	Live3DPreviewScene = MakeUnique<FAdvancedPreviewScene>(PreviewSceneArgs);
	Live3DPreviewScene->SetFloorVisibility(false, true);

	Live3DPreviewEditorModeManager = MakeShared<FAssetEditorModeManager>();
	Live3DPreviewEditorModeManager->SetPreviewScene(Live3DPreviewScene.Get());
	Live3DPreviewInputRouter = Live3DPreviewEditorModeManager->GetInteractiveToolsContext()->InputRouter;

	LivePreviewTabContent = MakeShareable(new FEditorViewportTabContent());
	Live3DPreviewViewportClient = MakeShared<FFDOverlay3DViewportClient>(
		Live3DPreviewEditorModeManager.Get(), Live3DPreviewScene.Get(), nullptr, ViewportButtonsAPI);

	Live3DPreviewViewportDelegate = [this](FAssetEditorViewportConstructionArgs InArgs)
	{
		return SNew(SFDOverlay3DViewport, InArgs)
			.EditorViewportClient(Live3DPreviewViewportClient);
	};


}

FFDOverlayAssetEditorToolkit::~FFDOverlayAssetEditorToolkit()
{
	// We need to force the uv editor mode deletion now because otherwise the preview and unwrap worlds
	// will end up getting destroyed before the mode's Exit() function gets to run, and we'll get some
	// warnings when we destroy any mode actors.
	//EditorModeManager->DestroyMode(UFDOverlayEditorMode::EM_FDOverlayEditorModeId);

	// The UV subsystem is responsible for opening/focusing UV editor instances, so we should
	// notify it that this one is closing.
	UFDOverlayEditorSubsystem* FDSubsystem = GEditor->GetEditorSubsystem<UFDOverlayEditorSubsystem>();
	if (FDSubsystem)
	{
		TArray<TObjectPtr<UObject>> ObjectsWeWereEditing;
		OwningAssetEditor->GetObjectsToEdit(ObjectsWeWereEditing);
		FDSubsystem->NotifyThatFDEditorClosed(ObjectsWeWereEditing);
	}
}

FText FFDOverlayAssetEditorToolkit::GetToolkitName() const
{
	return LOCTEXT("FDOverlayTabName", "TestTabName");
}

FName FFDOverlayAssetEditorToolkit::GetToolkitFName() const
{
	return FName(FString::Printf(TEXT("FDOverlay%p"), this));
}


FText FFDOverlayAssetEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("FDOverlayBaseToolkitName", "FDFullBodyOverlay");
}

FText FFDOverlayAssetEditorToolkit::GetToolkitToolTipText() const
{
	return LOCTEXT("FDOverlayToolTipText", "TestToolTipText");
}

void FFDOverlayAssetEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	// 我们绕过FBaseAssetToolkit::RegisterTabSpawners，因为除了我们不想要的选项卡，它似乎没有提供给我们任何东西。
	// TODO: What is FAssetEditorToolkit::RegisterTabSpawners actually used for?
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	// TODO: What is this actually used for?
	FDOverlayMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_UVEditor", "FDOverlay FullBodyOverlay"));

	// 这里我们设置了在StandaloneDefaultLayout中引用的选项卡(在构造函数中)。
	// 这里我们不处理工具栏面板，因为这是由FModeToolkit中现有的基础设施处理的。我们只为我们的自定义选项卡设置产卵器，即2D和3D视口。
	InTabManager->RegisterTabSpawner(ViewportTabID, FOnSpawnTab::CreateSP(this, &FFDOverlayAssetEditorToolkit::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("2DViewportTabLabel", "2D Viewport"))
		.SetGroup(FDOverlayMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(Live3DPreviewTabID, FOnSpawnTab::CreateSP(this,
		&FFDOverlayAssetEditorToolkit::SpawnTab_LivePreview))
		.SetDisplayName(LOCTEXT("3DViewportTabLabel", "3D Viewport"))
		.SetGroup(FDOverlayMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));
}

void FFDOverlayAssetEditorToolkit::OnClose()
{
	// 给任何活动模式一个机会关闭，而工具箱主机仍然存在
	// 这是非常重要的，否则当前打开的标签将不会被标记为“关闭”。
	// 这将导致在重新打开编辑器时不能正确地回收标签，并为每个打开事件复制标签。
	GetEditorModeManager().ActivateDefaultMode();

	FAssetEditorToolkit::OnClose();
}

// 当模式启动或结束一个工具时，这些被间接(通过工具箱主机)从模式工具箱中调用，以添加或删除 accept/cancel overlay 。
void FFDOverlayAssetEditorToolkit::AddViewportOverlayWidget(TSharedRef<SWidget> InViewportOverlayWidget)
{
	TSharedPtr<SFDOverlay3DViewport> ViewportWidget = StaticCastSharedPtr<SFDOverlay3DViewport>(ViewportTabContent->GetFirstViewport());
	ViewportWidget->AddOverlayWidget(InViewportOverlayWidget);
}
void FFDOverlayAssetEditorToolkit::RemoveViewportOverlayWidget(TSharedRef<SWidget> InViewportOverlayWidget)
{
	TSharedPtr<SFDOverlay3DViewport> ViewportWidget = StaticCastSharedPtr<SFDOverlay3DViewport>(ViewportTabContent->GetFirstViewport());
	ViewportWidget->RemoveOverlayWidget(InViewportOverlayWidget);
}

TSharedRef<SDockTab> FFDOverlayAssetEditorToolkit::SpawnTab_LivePreview(const FSpawnTabArgs& Args)
{
	TSharedRef< SDockTab > DockableTab =
		SNew(SDockTab);

	const FString LayoutId = FString("FDOverlayLive3DPreviewViewport");
	LivePreviewTabContent->Initialize(Live3DPreviewViewportDelegate, DockableTab, LayoutId);
	return DockableTab;
}

void FFDOverlayAssetEditorToolkit::CreateWidgets()
{
	// 在UAssetEditor::Init()期间被调用，在工具包创建之后，但在调用InitAssetEditor之前。
	// 如果我们想要添加自定义模式级工具栏，它们可能会在这里，但我们仍然需要调用基本的CreateWidgets，
	//因为它会调用生成viewport客户端的东西，等等。

	FBaseAssetToolkit::CreateWidgets();
}

// 在 FBaseAssetToolkit::CreateWidgets 调用
TSharedPtr<FEditorViewportClient> FFDOverlayAssetEditorToolkit::CreateEditorViewportClient() const
{
	// 注意，我们在这里不能可靠地调整视口客户端，因此我们将把它传递到由我们从GetViewportDelegate()获得的视口委托创建的视口中，
	// 并且该委托可能(将)影响基于FAssetEditorViewportConstructionArgs的设置，即ViewportType。
	// 相反，我们在PostInitAssetEditor()中做视口客户端调整。
	check(EditorModeManager.IsValid());
	return MakeShared<FFDOverlay2DViewportClient>(EditorModeManager.Get(), UnwrapScene.Get(),
		FDOverlay2DViewport, ViewportButtonsAPI);
}

// 在 FBaseAssetToolkit::CreateWidgets 调用
AssetEditorViewportFactoryFunction FFDOverlayAssetEditorToolkit::GetViewportDelegate()
{
	AssetEditorViewportFactoryFunction TempViewportDelegate = [this](FAssetEditorViewportConstructionArgs InArgs)
	{
		return SAssignNew(FDOverlay2DViewport, SFDOverlay2DViewport, InArgs)
			.EditorViewportClient(ViewportClient);
	};

	return TempViewportDelegate;
}

void FFDOverlayAssetEditorToolkit::CreateEditorModeManager()
{
	EditorModeManager = MakeShared<FAssetEditorModeManager>();
	
	// TODO: What is this actually used for?

	// 模式管理器是模式和工具上下文世界的权威，在这里设置预览场景使我们的GetWorld()函数返回预览场景世界，
	// 而不是普通的关卡编辑器。这很重要，因为这是我们创建 预览网格、gizmo actors 等的地方。
	StaticCastSharedPtr<FAssetEditorModeManager>(EditorModeManager)->SetPreviewScene(UnwrapScene.Get());
}

void FFDOverlayAssetEditorToolkit::PostInitAssetEditor()
{
	// 我们在这里设置了ModeUILayer连接，因为InitAssetEditor对我们是关闭的。
	// 其他编辑器在其他地方执行此步骤，但这是我们的最佳位置。
	TSharedPtr<class IToolkitHost> PinnedToolkitHost = ToolkitHost.Pin();
	check(PinnedToolkitHost.IsValid());
	ModeUILayer = MakeShareable(new FFDOverlayEditorModeUILayer(PinnedToolkitHost.Get()));
	ModeUILayer->SetModeMenuCategory(FDOverlayMenuCategory);

	// 
	TArray<TObjectPtr<UObject>> ObjectsToEdit;
	OwningAssetEditor->GetObjectsToEdit(ObjectsToEdit);

	TArray<FTransform> ObjectTransforms;
	ObjectTransforms.SetNum(ObjectsToEdit.Num());

	// 这个静态方法初始化了各种上下文，使得 Mode在Enter()的 ContextStore 中可用才能正常工作。
	UFDOverlayEditorMode::InitializeAssetEditorContexts(*EditorModeManager->GetInteractiveToolsContext()->ContextObjectStore,
		ObjectsToEdit, ObjectTransforms, *Live3DPreviewViewportClient, *ViewportClient, *Live3DPreviewEditorModeManager,
		*ViewportButtonsAPI);
		
	// 目前，除了设置所有的UI元素，工具箱还启动了FD编辑器模式，这是编辑器始终工作的模式(东西被打包到一个模式中，以便在必要时可以移动到另一个资产编辑器中)。
	// 我们需要激活UV模式来创建左侧的工具箱。
	check(EditorModeManager.IsValid());
	EditorModeManager->ActivateMode(UFDOverlayEditorMode::EM_FDOverlayEditorModeId);
	UFDOverlayEditorMode* FDOverlayEditorMode = Cast<UFDOverlayEditorMode>(
		EditorModeManager->GetActiveScriptableMode(UFDOverlayEditorMode::EM_FDOverlayEditorModeId));
	check(FDOverlayEditorMode);

	// 不管用户如何修改布局，我们将确保我们有一个2d视口，这将允许我们的模式接收tick。
	// 我们不再需要调用工具面板选项卡，因为这是由底层基础设施处理的。
	if (!TabManager->FindExistingLiveTab(ViewportTabID))
	{
		TabManager->TryInvokeTab(ViewportTabID);
	}


	// 注意：我们不需要强制打开实时视口，但如果我们不这样做，我们需要确保未来的任何实时预览api功能不会在视口丢失时崩溃。
	// 丢失时不会崩溃，因为有些视口客户端功能对这种情况并不健全。我们只是强制它打开，因为这更安全，而且对用户来说似乎也更方便。
	// 因为重新打开窗口可能是不直观的，而关闭它则很容易。
	if (!TabManager->FindExistingLiveTab(Live3DPreviewTabID))
	{
		TabManager->TryInvokeTab(Live3DPreviewTabID);
	}

	// 添加"Apply Changes"按钮.实际上，几乎在任何时候这样做都是安全的，甚至在工具栏注册之前，但将大多数内容放入PostInitAssetEditor()更容易。
	// TODO:我们可以考虑将这些绑定到模式的操作放到一些模式操作列表中，然后让模式在进入/退出时将它们提供给拥有的资产编辑器。当/如果这变得更容易做的时候，重新审视。
	// ToolkitCommands: List of UI commands for this toolkit.   This should be filled in by the derived class!
	ToolkitCommands->MapAction(
		FFDOverlayEditorModeCommands::Get().ApplyChanges,
		FExecuteAction::CreateUObject(FDOverlayEditorMode, &UFDOverlayEditorMode::ApplyChanges),
		FCanExecuteAction::CreateUObject(FDOverlayEditorMode, &UFDOverlayEditorMode::CanApplyChanges));
	FName ParentToolbarName;
	const FName ToolBarName = GetToolMenuToolbarName(ParentToolbarName);
	UToolMenu* AssetToolbar = UToolMenus::Get()->ExtendMenu(ToolBarName);
	FToolMenuSection& Section = AssetToolbar->FindOrAddSection("Asset");
	Section.AddEntry(FToolMenuEntry::InitToolBarButton(FFDOverlayEditorModeCommands::Get().ApplyChanges, TAttribute<FText>(),
	TAttribute<FText>(), FSlateIcon(FFDOverlayStyle::Get().GetStyleSetName(), "FDOverlay.ApplyChanges")));
	
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
		UFDOverlayEditorSubsystem* FDOverlaySubsystem = GEditor->GetEditorSubsystem<UFDOverlayEditorSubsystem>();
		if (FDOverlaySubsystem)
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
		SetCommonViewportClientOptions(Live3DPreviewViewportClient.Get());
		Live3DPreviewViewportClient->ToggleOrbitCamera(true);

		// TODO: This should not be hardcoded
		Live3DPreviewViewportClient->SetViewLocation(FVector(-200, 100, 100));
		Live3DPreviewViewportClient->SetLookAtLocation(FVector(0, 0, 0));
	}

	// Adjust camera view to focus on the scene
	//FDAssistoMode->FocusLivePreviewCameraOnSelection();

	// 将视口命令列表连接到我们的工具箱命令列表，这样工具箱未处理的热键将由视口处理(允许我们使用在详细信息面板或FD编辑器的其他地方单击后视口注册的任何热键)。
	// 注意，命令列表的“Append”调用可能应该被称为“AppendTo”，因为它将被调用对象添加为参数命令列表的子对象。
	// 也就是说，在 ToolkitCommands中查找之后，我们将在 2DviewportCommands中查找。
	TSharedPtr<SFDOverlay2DViewport> ViewportWidget = StaticCastSharedPtr<SFDOverlay2DViewport>(ViewportTabContent->GetFirstViewport());
	ViewportWidget->GetCommandList()->Append(ToolkitCommands);

	
}

//const FSlateBrush* FUVEditorToolkit::GetDefaultTabIcon() const
//{
//	return FUVEditorStyle::Get().GetBrush("UVEditor.OpenUVEditor");
//}

FLinearColor FFDOverlayAssetEditorToolkit::GetDefaultTabColor() const
{
	return FLinearColor::White;
}

// 当另一个工具箱(比如ed模式的工具箱)托管在这个资产编辑器工具箱中时调用，也可以说是在这个资产编辑器工具箱中打开时就调用。
void FFDOverlayAssetEditorToolkit::OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit)
{
	ModeUILayer->OnToolkitHostingStarted(Toolkit);
}

void FFDOverlayAssetEditorToolkit::OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit)
{
	ModeUILayer->OnToolkitHostingFinished(Toolkit);
}

#undef LOCTEXT_NAMESPACE