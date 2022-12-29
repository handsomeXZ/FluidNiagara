#include "FullBodyOverlayEditor/FDFBOverlayAssetEditorTooltik.h"

#include "FullBodyOverlayEditor/FDFBOverlayAssetEditor.h"
#include "FullBodyOverlayEditor/FDFBOverlayModeUILayer.h"
#include "FullBodyOverlayEditor/FDAssistor2DViewportClient.h"
#include "FullBodyOverlayEditor/FDAssistor3DViewportClient.h"
#include "FullBodyOverlayEditor/FDFBOverlayEditorSubsystem.h"
#include "FullBodyOverlayEditor/SFDAssistor2DViewport.h"
//#include "FullBodyOverlayEditor/FDFBOverlayEditorMode.h"
//#include "FullBodyOverlayEditor/FDFBOverlayEditorModeCommands.h"

#include "PreviewScene.h"
#include "AdvancedPreviewScene.h"
#include "EditorViewportTabContent.h"
#include "AssetEditorModeManager.h"
#include "EdModeInteractiveToolsContext.h"
#include "UVEditorUXSettings.h"


#define LOCTEXT_NAMESPACE "FDFBOverlayAssetEditorToolkit"
const FName FFDFBOverlayAssetEditorToolkit::Live3DPreviewTabID(TEXT("FDEditor_Live3DPreviewTab"));

FFDFBOverlayAssetEditorToolkit::FFDFBOverlayAssetEditorToolkit(UAssetEditor* InOwningAssetEditor)
	:FBaseAssetToolkit(InOwningAssetEditor)
{
	check(Cast<UFDFBOverlayAssetEditor>(InOwningAssetEditor));

	// ���ǽ��滻��������ǵ�StandaloneDefaultLayout����������ϸ���������ߵĴ�ֱ�У��ұ��������ӿڡ�
	//�����ڶ�ջ�϶�������ʽ��extensionid���Ա��Ժ���UILayer�ṩ������չʱ�������ǡ�
	//
	// ע��:�Բ��ֵĸı�Ӧ�ð����Բ���ID�����ӣ���UVEditorLayout[X] -> UVEditorLayout[X+1]��
	//�������û���ڱ༭������ȫ���ò���Ĭ��ֵ�����ֿ��ܻ�һ���㡣
	StandaloneDefaultLayout = FTabManager::NewLayout(FName("FDAssistorLayout"))
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->SetExtensionId("EditorSidePanelArea")
					->SetHideTabWell(true)
				)
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
					->SetSizeCoefficient(0.4f)
					->AddTab(Live3DPreviewTabID, ETabState::OpenedTab) // Live3DPreviewTabID ֮���� RegisterTabSpawners() ��������
					->SetExtensionId("Viewport3DArea")
					->SetHideTabWell(true)
				)
			)
		);



	// ���UStaticMeshEditorUISubsystemָ�����κ���չ����
	// ��չ����ΪFModeToolkit�ṩ���Ѷ����λ�ã��Ը��ӹ������ѡ�����ϸ���ѡ�



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
	Live3DPreviewViewportClient = MakeShared<FFDAssistor3DViewportClient>(
		Live3DPreviewEditorModeManager.Get(), Live3DPreviewScene.Get(), nullptr, ViewportButtonsAPI);

	Live3DPreviewViewportDelegate = [this](FAssetEditorViewportConstructionArgs InArgs)
	{
		return SNew(SFDAssistor2DViewport, InArgs)
			.EditorViewportClient(Live3DPreviewViewportClient);
	};


}

FFDFBOverlayAssetEditorToolkit::~FFDFBOverlayAssetEditorToolkit()
{
	// We need to force the uv editor mode deletion now because otherwise the preview and unwrap worlds
	// will end up getting destroyed before the mode's Exit() function gets to run, and we'll get some
	// warnings when we destroy any mode actors.
	//EditorModeManager->DestroyMode(UFDFBOverlayEditorMode::EM_FDFBOverlayEditorModeId);

	// The UV subsystem is responsible for opening/focusing UV editor instances, so we should
	// notify it that this one is closing.
	UFDFBOverlayEditorSubsystem* FDSubsystem = GEditor->GetEditorSubsystem<UFDFBOverlayEditorSubsystem>();
	if (FDSubsystem)
	{
		TArray<TObjectPtr<UObject>> ObjectsWeWereEditing;
		OwningAssetEditor->GetObjectsToEdit(ObjectsWeWereEditing);
		FDSubsystem->NotifyThatFDEditorClosed(ObjectsWeWereEditing);
	}
}

FText FFDFBOverlayAssetEditorToolkit::GetToolkitName() const
{
	return LOCTEXT("FDFBOverlayTabName", "TestTabName");
}

FName FFDFBOverlayAssetEditorToolkit::GetToolkitFName() const
{
	return FName(FString::Printf(TEXT("FDFBOverlay%p"), this));
}


FText FFDFBOverlayAssetEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("FDFBOverlayBaseToolkitName", "FDFullBodyOverlay");
}

FText FFDFBOverlayAssetEditorToolkit::GetToolkitToolTipText() const
{
	return LOCTEXT("FDFBOverlayToolTipText", "TestToolTipText");
}

void FFDFBOverlayAssetEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	// �����ƹ�FBaseAssetToolkit::RegisterTabSpawners����Ϊ�������ǲ���Ҫ��ѡ������ƺ�û���ṩ�������κζ�����
	// TODO: What is FAssetEditorToolkit::RegisterTabSpawners actually used for?
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	// TODO: What is this actually used for?
	FDFBOverlayMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_UVEditor", "FDAssistor FullBodyOverlay"));

	// ����������������StandaloneDefaultLayout�����õ�ѡ�(�ڹ��캯����)��
	// �������ǲ�����������壬��Ϊ������FModeToolkit�����еĻ�����ʩ����ġ�����ֻΪ���ǵ��Զ���ѡ����ò���������2D��3D�ӿڡ�
	InTabManager->RegisterTabSpawner(ViewportTabID, FOnSpawnTab::CreateSP(this, &FFDFBOverlayAssetEditorToolkit::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("2DViewportTabLabel", "2D Viewport"))
		.SetGroup(FDFBOverlayMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(Live3DPreviewTabID, FOnSpawnTab::CreateSP(this,
		&FFDFBOverlayAssetEditorToolkit::SpawnTab_LivePreview))
		.SetDisplayName(LOCTEXT("3DViewportTabLabel", "3D Viewport"))
		.SetGroup(FDFBOverlayMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));
}

void FFDFBOverlayAssetEditorToolkit::OnClose()
{
	// ���κλģʽһ������رգ���������������Ȼ����
	// ���Ƿǳ���Ҫ�ģ�����ǰ�򿪵ı�ǩ�����ᱻ���Ϊ���رա���
	// �⽫���������´򿪱༭��ʱ������ȷ�ػ��ձ�ǩ����Ϊÿ�����¼����Ʊ�ǩ��
	GetEditorModeManager().ActivateDefaultMode();

	FAssetEditorToolkit::OnClose();
}

void FFDFBOverlayAssetEditorToolkit::AddViewportOverlayWidget(TSharedRef<SWidget> InViewportOverlayWidget)
{
	// TODO: Unimplemented, is empty function
	// TODO: What is this actually used for?
}
void FFDFBOverlayAssetEditorToolkit::RemoveViewportOverlayWidget(TSharedRef<SWidget> InViewportOverlayWidget)
{
	// TODO: Unimplemented, is empty function
	// TODO: What is this actually used for?
}

TSharedRef<SDockTab> FFDFBOverlayAssetEditorToolkit::SpawnTab_LivePreview(const FSpawnTabArgs& Args)
{
	TSharedRef< SDockTab > DockableTab =
		SNew(SDockTab);

	const FString LayoutId = FString("FDFBOverlayLive3DPreviewViewport");
	LivePreviewTabContent->Initialize(Live3DPreviewViewportDelegate, DockableTab, LayoutId);
	return DockableTab;
}

void FFDFBOverlayAssetEditorToolkit::CreateWidgets()
{
	// ��UAssetEditor::Init()�ڼ䱻���ã��ڹ��߰�����֮�󣬵��ڵ���InitAssetEditor֮ǰ��
	// ���������Ҫ����Զ���ģʽ�������������ǿ��ܻ��������������Ȼ��Ҫ���û�����CreateWidgets��
	//��Ϊ�����������viewport�ͻ��˵Ķ������ȵȡ�

	FBaseAssetToolkit::CreateWidgets();
}

// �� FBaseAssetToolkit::CreateWidgets ����
TSharedPtr<FEditorViewportClient> FFDFBOverlayAssetEditorToolkit::CreateEditorViewportClient() const
{
	// ע�⣬���������ﲻ�ܿɿ��ص����ӿڿͻ��ˣ���Ϊ���ǽ��������ݵ������Ǵ�GetViewportDelegate()��õ��ӿ�ί�д������ӿ��У�
	// ���Ҹ�ί�п���(��)Ӱ�����FAssetEditorViewportConstructionArgs�����ã���ViewportType��
	// �෴��������PostInitAssetEditor()�����ӿڿͻ��˵�����
	check(EditorModeManager.IsValid());
	return MakeShared<FFDAssistor2DViewportClient>(EditorModeManager.Get(), UnwrapScene.Get(),
		FDFBOverlay2DViewport, ViewportButtonsAPI, FDAssistorLive2DViewportAPI);
}

// �� FBaseAssetToolkit::CreateWidgets ����
AssetEditorViewportFactoryFunction FFDFBOverlayAssetEditorToolkit::GetViewportDelegate()
{
	AssetEditorViewportFactoryFunction TempViewportDelegate = [this](FAssetEditorViewportConstructionArgs InArgs)
	{
		return SAssignNew(FDFBOverlay2DViewport, SFDAssistor2DViewport, InArgs)
			.EditorViewportClient(ViewportClient);
	};

	return TempViewportDelegate;
}

void FFDFBOverlayAssetEditorToolkit::CreateEditorModeManager()
{
	EditorModeManager = MakeShared<FAssetEditorModeManager>();
	
	// TODO: What is this actually used for?

	// ģʽ��������ģʽ�͹��������������Ȩ��������������Ԥ������ʹ���ǵ�GetWorld()��������Ԥ���������磬
	// ��������ͨ�Ĺؿ��༭���������Ҫ����Ϊ�������Ǵ��� Ԥ������gizmo actors �ȵĵط���
	StaticCastSharedPtr<FAssetEditorModeManager>(EditorModeManager)->SetPreviewScene(UnwrapScene.Get());
}

void FFDFBOverlayAssetEditorToolkit::PostInitAssetEditor() 
{
	// ����������������ModeUILayer���ӣ���ΪInitAssetEditor�������ǹرյġ�
	// �����༭���������ط�ִ�д˲��裬���������ǵ����λ�á�
	TSharedPtr<class IToolkitHost> PinnedToolkitHost = ToolkitHost.Pin();
	check(PinnedToolkitHost.IsValid());
	ModeUILayer = MakeShareable(new FFDFBOverlayEditorModeUILayer(PinnedToolkitHost.Get()));
	ModeUILayer->SetModeMenuCategory(FDFBOverlayMenuCategory);

	// 
	TArray<TObjectPtr<UObject>> ObjectsToEdit;
	OwningAssetEditor->GetObjectsToEdit(ObjectsToEdit);

	TArray<FTransform> ObjectTransforms;
	ObjectTransforms.SetNum(ObjectsToEdit.Num());

	// �����̬������ʼ���˸��������ģ�ʹ�� Mode��Enter()�� ContextStore �п��ò�������������
	/*UFDFBOverlayEditorMode::InitializeAssetEditorContexts(*EditorModeManager->GetInteractiveToolsContext()->ContextObjectStore,
		ObjectsToEdit, ObjectTransforms, *Live3DPreviewViewportClient, *Live3DPreviewEditorModeManager,
		*ViewportButtonsAPI, *FDAssistorLive2DViewportAPI);*/

	// Ŀǰ�������������е�UIԪ�أ������仹������FD�༭��ģʽ�����Ǳ༭��ʼ�չ�����ģʽ(�����������һ��ģʽ�У��Ա��ڱ�Ҫʱ�����ƶ�����һ���ʲ��༭����)��
	// ������Ҫ����UVģʽ���������Ĺ����䡣
	/*check(EditorModeManager.IsValid());
	EditorModeManager->ActivateMode(UFDFBOverlayEditorMode::EM_FDFBOverlayEditorModeId);
	UFDFBOverlayEditorMode* FDAssistoMode = Cast<UFDFBOverlayEditorMode>(
		EditorModeManager->GetActiveScriptableMode(UFDFBOverlayEditorMode::EM_FDFBOverlayEditorModeId));
	check(FDAssistoMode);*/

	// �����û�����޸Ĳ��֣����ǽ�ȷ��������һ��2d�ӿڣ��⽫�������ǵ�ģʽ����tick��
	// ���ǲ�����Ҫ���ù������ѡ�����Ϊ�����ɵײ������ʩ����ġ�
	if (!TabManager->FindExistingLiveTab(ViewportTabID))
	{
		TabManager->TryInvokeTab(ViewportTabID);
	}

	// ���"Apply Changes"��ť.ʵ���ϣ��������κ�ʱ�����������ǰ�ȫ�ģ������ڹ�����ע��֮ǰ��������������ݷ���PostInitAssetEditor()�����ס�
	// TODO:���ǿ��Կ��ǽ���Щ�󶨵�ģʽ�Ĳ����ŵ�һЩģʽ�����б��У�Ȼ����ģʽ�ڽ���/�˳�ʱ�������ṩ��ӵ�е��ʲ��༭������/������ø���������ʱ���������ӡ�
	// ToolkitCommands: List of UI commands for this toolkit.   This should be filled in by the derived class!
//	ToolkitCommands->MapAction(
//		FFDFBOverlayEditorModeCommands::Get().ApplyChanges, FUIAction()
///*		FExecuteAction::CreateUObject(FDAssistoMode, &UFDFBOverlayEditorMode::ApplyChanges),
//		FCanExecuteAction::CreateUObject(FDAssistoMode, &UFDFBOverlayEditorMode::CanApplyChanges)*/);
//	FName ParentToolbarName;
//	const FName ToolBarName = GetToolMenuToolbarName(ParentToolbarName);
//	UToolMenu* AssetToolbar = UToolMenus::Get()->ExtendMenu(ToolBarName);
//	FToolMenuSection& Section = AssetToolbar->FindOrAddSection("Asset");
//	Section.AddEntry(FToolMenuEntry::InitToolBarButton(FFDFBOverlayEditorModeCommands::Get().ApplyChanges));

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
		UFDFBOverlayEditorSubsystem* FDFBOverlaySubsystem = GEditor->GetEditorSubsystem<UFDFBOverlayEditorSubsystem>();
		if (FDFBOverlaySubsystem)
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
	TSharedPtr<SFDAssistor2DViewport> ViewportWidget = StaticCastSharedPtr<SFDAssistor2DViewport>(ViewportTabContent->GetFirstViewport());
	ViewportWidget->GetCommandList()->Append(ToolkitCommands);

	
}

//const FSlateBrush* FUVEditorToolkit::GetDefaultTabIcon() const
//{
//	return FUVEditorStyle::Get().GetBrush("UVEditor.OpenUVEditor");
//}

FLinearColor FFDFBOverlayAssetEditorToolkit::GetDefaultTabColor() const
{
	return FLinearColor::White;
}

// ����һ��������(����edģʽ�Ĺ�����)�й�������ʲ��༭����������ʱ���ã�Ҳ����˵��������ʲ��༭���������д�ʱ�͵��á�
void FFDFBOverlayAssetEditorToolkit::OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit)
{
	ModeUILayer->OnToolkitHostingStarted(Toolkit);
}

void FFDFBOverlayAssetEditorToolkit::OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit)
{
	ModeUILayer->OnToolkitHostingFinished(Toolkit);
}

#undef LOCTEXT_NAMESPACE