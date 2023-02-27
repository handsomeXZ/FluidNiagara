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

	// ���ǽ��滻��������ǵ�StandaloneDefaultLayout������������У�������ϸ�������ߵ�һ����ֱ�У��ұ��������ӿڡ�
	// �����ڶ�ջ�϶�����ʽ��ExtensionIds���Ա��Ժ���UILayer�ṩ������չʱ�������ǡ� 
	// ע�⣺�Բ��ֵĸı�Ӧ�ð����Բ���ID����������UVEditorLayout[X] -> UVEditorLayout[X+1]��
	// ���򣬲��ֿ��ܻᱻ���� û����ȫ����Ϊ�༭���ڲ��Ĳ���Ĭ��ֵ��
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
						->AddTab(ViewportTabID, ETabState::OpenedTab)	// ViewportTabID ֮���� RegisterTabSpawners() ��������
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
					->AddTab(Live3DPreviewTabID, ETabState::OpenedTab) // Live3DPreviewTabID ֮���� RegisterTabSpawners() ��������
					->SetExtensionId("Viewport3DArea")
					->SetHideTabWell(true)
				)
				
			)
		);



	// ����κ���UStaticMeshEditorUISubsystemָ������չ����
	// ��Щ��չ��ΪFModeToolkit�ṩ�˶����λ�ã�ʹ���ܹ����ӵ� ���ߵ�ɫ���ǩ��ϸ������ǩ��
	LayoutExtender = MakeShared<FLayoutExtender>();
	FFDOverlayEditorModule* FDOverlayEditorModule = &FModuleManager::LoadModuleChecked<FFDOverlayEditorModule>("FDOverlayEditor");
	FDOverlayEditorModule->OnRegisterLayoutExtensions().Broadcast(*LayoutExtender);
	StandaloneDefaultLayout->ProcessExtensions(*LayoutExtender);


	// ��API������Ϊ�ӿڹ������͹���֮���ͨ�ŵ㡣
	// ���������ﴴ�������Ա����ǿ����ڳ�ʼ��ģʽʱ�������ݵ�2d��3d�ӿڡ�



	//���ǿ�����CreateEditorViewportClient()�д���Ԥ������������FBaseAssetToolkit�����������ƺ���ֱ�۵ش��������������Ժ��롣
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
	// �����ƹ�FBaseAssetToolkit::RegisterTabSpawners����Ϊ�������ǲ���Ҫ��ѡ������ƺ�û���ṩ�������κζ�����
	// TODO: What is FAssetEditorToolkit::RegisterTabSpawners actually used for?
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	// TODO: What is this actually used for?
	FDOverlayMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_UVEditor", "FDOverlay FullBodyOverlay"));

	// ����������������StandaloneDefaultLayout�����õ�ѡ�(�ڹ��캯����)��
	// �������ǲ�����������壬��Ϊ������FModeToolkit�����еĻ�����ʩ����ġ�����ֻΪ���ǵ��Զ���ѡ����ò���������2D��3D�ӿڡ�
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
	// ���κλģʽһ������رգ���������������Ȼ����
	// ���Ƿǳ���Ҫ�ģ�����ǰ�򿪵ı�ǩ�����ᱻ���Ϊ���رա���
	// �⽫���������´򿪱༭��ʱ������ȷ�ػ��ձ�ǩ����Ϊÿ�����¼����Ʊ�ǩ��
	GetEditorModeManager().ActivateDefaultMode();

	FAssetEditorToolkit::OnClose();
}

// ��ģʽ���������һ������ʱ����Щ�����(ͨ������������)��ģʽ�������е��ã�����ӻ�ɾ�� accept/cancel overlay ��
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
	// ��UAssetEditor::Init()�ڼ䱻���ã��ڹ��߰�����֮�󣬵��ڵ���InitAssetEditor֮ǰ��
	// ���������Ҫ����Զ���ģʽ�������������ǿ��ܻ��������������Ȼ��Ҫ���û�����CreateWidgets��
	//��Ϊ�����������viewport�ͻ��˵Ķ������ȵȡ�

	FBaseAssetToolkit::CreateWidgets();
}

// �� FBaseAssetToolkit::CreateWidgets ����
TSharedPtr<FEditorViewportClient> FFDOverlayAssetEditorToolkit::CreateEditorViewportClient() const
{
	// ע�⣬���������ﲻ�ܿɿ��ص����ӿڿͻ��ˣ�������ǽ��������ݵ������Ǵ�GetViewportDelegate()��õ��ӿ�ί�д������ӿ��У�
	// ���Ҹ�ί�п���(��)Ӱ�����FAssetEditorViewportConstructionArgs�����ã���ViewportType��
	// �෴��������PostInitAssetEditor()�����ӿڿͻ��˵�����
	check(EditorModeManager.IsValid());
	return MakeShared<FFDOverlay2DViewportClient>(EditorModeManager.Get(), UnwrapScene.Get(),
		FDOverlay2DViewport, ViewportButtonsAPI);
}

// �� FBaseAssetToolkit::CreateWidgets ����
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

	// ģʽ��������ģʽ�͹��������������Ȩ��������������Ԥ������ʹ���ǵ�GetWorld()��������Ԥ���������磬
	// ��������ͨ�Ĺؿ��༭���������Ҫ����Ϊ�������Ǵ��� Ԥ������gizmo actors �ȵĵط���
	StaticCastSharedPtr<FAssetEditorModeManager>(EditorModeManager)->SetPreviewScene(UnwrapScene.Get());
}

void FFDOverlayAssetEditorToolkit::PostInitAssetEditor()
{
	// ����������������ModeUILayer���ӣ���ΪInitAssetEditor�������ǹرյġ�
	// �����༭���������ط�ִ�д˲��裬���������ǵ����λ�á�
	TSharedPtr<class IToolkitHost> PinnedToolkitHost = ToolkitHost.Pin();
	check(PinnedToolkitHost.IsValid());
	ModeUILayer = MakeShareable(new FFDOverlayEditorModeUILayer(PinnedToolkitHost.Get()));
	ModeUILayer->SetModeMenuCategory(FDOverlayMenuCategory);

	// 
	TArray<TObjectPtr<UObject>> ObjectsToEdit;
	OwningAssetEditor->GetObjectsToEdit(ObjectsToEdit);

	TArray<FTransform> ObjectTransforms;
	ObjectTransforms.SetNum(ObjectsToEdit.Num());

	// �����̬������ʼ���˸��������ģ�ʹ�� Mode��Enter()�� ContextStore �п��ò�������������
	UFDOverlayEditorMode::InitializeAssetEditorContexts(*EditorModeManager->GetInteractiveToolsContext()->ContextObjectStore,
		ObjectsToEdit, ObjectTransforms, *Live3DPreviewViewportClient, *ViewportClient, *Live3DPreviewEditorModeManager,
		*ViewportButtonsAPI);
		
	// Ŀǰ�������������е�UIԪ�أ������仹������FD�༭��ģʽ�����Ǳ༭��ʼ�չ�����ģʽ(�����������һ��ģʽ�У��Ա��ڱ�Ҫʱ�����ƶ�����һ���ʲ��༭����)��
	// ������Ҫ����UVģʽ���������Ĺ����䡣
	check(EditorModeManager.IsValid());
	EditorModeManager->ActivateMode(UFDOverlayEditorMode::EM_FDOverlayEditorModeId);
	UFDOverlayEditorMode* FDOverlayEditorMode = Cast<UFDOverlayEditorMode>(
		EditorModeManager->GetActiveScriptableMode(UFDOverlayEditorMode::EM_FDOverlayEditorModeId));
	check(FDOverlayEditorMode);

	// �����û�����޸Ĳ��֣����ǽ�ȷ��������һ��2d�ӿڣ��⽫�������ǵ�ģʽ����tick��
	// ���ǲ�����Ҫ���ù������ѡ�����Ϊ�����ɵײ������ʩ����ġ�
	if (!TabManager->FindExistingLiveTab(ViewportTabID))
	{
		TabManager->TryInvokeTab(ViewportTabID);
	}


	// ע�⣺���ǲ���Ҫǿ�ƴ�ʵʱ�ӿڣ���������ǲ���������������Ҫȷ��δ�����κ�ʵʱԤ��api���ܲ������ӿڶ�ʧʱ������
	// ��ʧʱ�����������Ϊ��Щ�ӿڿͻ��˹��ܶ��������������ȫ������ֻ��ǿ�����򿪣���Ϊ�����ȫ�����Ҷ��û���˵�ƺ�Ҳ�����㡣
	// ��Ϊ���´򿪴��ڿ����ǲ�ֱ�۵ģ����ر���������ס�
	if (!TabManager->FindExistingLiveTab(Live3DPreviewTabID))
	{
		TabManager->TryInvokeTab(Live3DPreviewTabID);
	}

	// ���"Apply Changes"��ť.ʵ���ϣ��������κ�ʱ�����������ǰ�ȫ�ģ������ڹ�����ע��֮ǰ��������������ݷ���PostInitAssetEditor()�����ס�
	// TODO:���ǿ��Կ��ǽ���Щ�󶨵�ģʽ�Ĳ����ŵ�һЩģʽ�����б��У�Ȼ����ģʽ�ڽ���/�˳�ʱ�������ṩ��ӵ�е��ʲ��༭������/������ø���������ʱ���������ӡ�
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
	// TODO�����û��Ҫʵ�֣������Ƿ���԰�����滻Ϊ ����ID Selection button ��
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
		// ͨ����bIsRealtime��־�����Ƿ���Զ�����Ӿ����ģ������ǵĹ�����Ҫʼ�չ�ѡ��
		Client->SetRealtime(true);

		// ���ö�̬ģ��Ч�����������ǵ���Ⱦ ��fade������Ϊ�������ƶ���
		Client->EngineShowFlags.SetTemporalAA(false);
		Client->EngineShowFlags.SetMotionBlur(false);

		// ���� gizmos ���ڵ����ֵ�dithering 
		Client->EngineShowFlags.SetOpaqueCompositeEditorPrimitives(true);

		// ����Ӳ���ڵ���ѯ����ʹ��ʹ�ö�����ɫ���������������������z�����ø������ѣ�
		// ��Ϊ�ǰ�͸�����Ͽ�ʼ�ڵ��Լ�(һ������߽�λ�ڱ��û��ļ���ͼ�κ���)��
		Client->EngineShowFlags.SetDisableOcclusionQueries(true);
	};

	// Adjust our main (2D) viewport:
	{
		SetCommonViewportClientOptions(ViewportClient.Get());

		// ���ҵ��� Ortho ����Ⱦ����������̫�����⣬��������Ӧ��ʹ��͸�ӡ�
		ViewportClient->SetViewportType(ELevelViewportType::LVT_Perspective);

		// �����ǿ���ʹ�õĲ��Ϸ��棬Lit ������������ѡ��
		ViewportClient->SetViewMode(EViewModeIndex::VMI_Lit);

		// scale [0,1] to [0,ScaleFactor]
		// �����������ǵ�������¿������У��㹻Զ���ܹ�������Ե��90��FOV
		double ScaleFactor = 1;
		UFDOverlayEditorSubsystem* FDOverlaySubsystem = GEditor->GetEditorSubsystem<UFDOverlayEditorSubsystem>();
		if (FDOverlaySubsystem)
		{
			ScaleFactor = FUVEditorUXSettings::UVMeshScalingFactor;
		}
		ViewportClient->SetViewLocation(FVector(ScaleFactor / 2, ScaleFactor / 2, ScaleFactor));
		ViewportClient->SetViewRotation(FRotator(-90, 0, 0));

		// ����ع�û������Ϊ�̶��������������������ʱ��˸
		ViewportClient->ExposureSettings.bFixed = true;

		// ������Ҫ�ӿڿͻ��˿�ʼ�۽������������ᱻѡ�У�ֱ�����ǵ�������档
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

	// ���ӿ������б����ӵ����ǵĹ����������б�����������δ������ȼ������ӿڴ���(��������ʹ������ϸ��Ϣ����FD�༭���������ط��������ӿ�ע����κ��ȼ�)��
	// ע�⣬�����б�ġ�Append�����ÿ���Ӧ�ñ���Ϊ��AppendTo������Ϊ���������ö������Ϊ���������б���Ӷ���
	// Ҳ����˵���� ToolkitCommands�в���֮�����ǽ��� 2DviewportCommands�в��ҡ�
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

// ����һ��������(����edģʽ�Ĺ�����)�й�������ʲ��༭����������ʱ���ã�Ҳ����˵��������ʲ��༭���������д�ʱ�͵��á�
void FFDOverlayAssetEditorToolkit::OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit)
{
	ModeUILayer->OnToolkitHostingStarted(Toolkit);
}

void FFDOverlayAssetEditorToolkit::OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit)
{
	ModeUILayer->OnToolkitHostingFinished(Toolkit);
}

#undef LOCTEXT_NAMESPACE