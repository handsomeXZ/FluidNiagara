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
	// ����������������ModeUILayer���ӣ���ΪInitAssetEditor�������ǹرյġ�
	// �����༭���������ط�ִ�д˲��裬���������ǵ����λ�á�
	TSharedPtr<class IToolkitHost> PinnedToolkitHost = ToolkitHost.Pin();
	check(PinnedToolkitHost.IsValid());
	ModeUILayer = MakeShareable(new FFDAssistorEditorModeUILayer(PinnedToolkitHost.Get()));
	ModeUILayer->SetModeMenuCategory(FDAssistorEditorMenuCategory);

	// 
	TArray<TObjectPtr<UObject>> ObjectsToEdit;
	OwningAssetEditor->GetObjectsToEdit(ObjectsToEdit);

	TArray<FTransform> ObjectTransforms;
	ObjectTransforms.SetNum(ObjectsToEdit.Num());

	// �����̬������ʼ���˸��������ģ�ʹ�� Mode��Enter()�� ContextStore �п��ò�������������
	UUVEditorMode::InitializeAssetEditorContexts(*EditorModeManager->GetInteractiveToolsContext()->ContextObjectStore,
		ObjectsToEdit, ObjectTransforms, *LivePreviewViewportClient, *LivePreviewEditorModeManager,
		*ViewportButtonsAPI, *FDAssistorLive2DViewportAPI);

	// Ŀǰ�������������е�UIԪ�أ������仹������FD�༭��ģʽ�����Ǳ༭��ʼ�չ�����ģʽ(�����������һ��ģʽ�У��Ա��ڱ�Ҫʱ�����ƶ�����һ���ʲ��༭����)��
	// ������Ҫ����UVģʽ���������Ĺ����䡣
	check(EditorModeManager.IsValid());
	EditorModeManager->ActivateMode(UFDAssistorEditorMode::EM_FDAssistorModeId);
	UFDAssistorEditorMode* FDAssistoMode = Cast<UFDAssistorEditorMode>(
		EditorModeManager->GetActiveScriptableMode(UFDAssistorEditorMode::EM_FDAssistorModeId));
	check(FDAssistoMode);

	// �����û�����޸Ĳ��֣����ǽ�ȷ��������һ��2d�ӿڣ��⽫�������ǵ�ģʽ����tick��
	// ���ǲ�����Ҫ���ù������ѡ�����Ϊ�����ɵײ������ʩ����ġ�
	if (!TabManager->FindExistingLiveTab(ViewportTabID))
	{
		TabManager->TryInvokeTab(ViewportTabID);
	}

	// ���"Apply Changes"��ť.ʵ���ϣ��������κ�ʱ�����������ǰ�ȫ�ģ������ڹ�����ע��֮ǰ��������������ݷ���PostInitAssetEditor()�����ס�
	// TODO:���ǿ��Կ��ǽ���Щ�󶨵�ģʽ�Ĳ����ŵ�һЩģʽ�����б��У�Ȼ����ģʽ�ڽ���/�˳�ʱ�������ṩ��ӵ�е��ʲ��༭������/������ø���������ʱ���������ӡ�
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
		UFDAssistorEditorSubsystem* FDAssistorSubsystem = GEditor->GetEditorSubsystem<UFDAssistorEditorSubsystem>();
		if (FDAssistorSubsystem)
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
		SetCommonViewportClientOptions(LivePreviewViewportClient.Get());
		LivePreviewViewportClient->ToggleOrbitCamera(true);

		// TODO: This should not be hardcoded
		LivePreviewViewportClient->SetViewLocation(FVector(-200, 100, 100));
		LivePreviewViewportClient->SetLookAtLocation(FVector(0, 0, 0));
	}

	// Adjust camera view to focus on the scene
	FDAssistoMode->FocusLivePreviewCameraOnSelection();

	// ���ӿ������б����ӵ����ǵĹ����������б�����������δ������ȼ������ӿڴ���(��������ʹ������ϸ��Ϣ����FD�༭���������ط��������ӿ�ע����κ��ȼ�)��
	// ע�⣬�����б�ġ�Append�����ÿ���Ӧ�ñ���Ϊ��AppendTo������Ϊ���������ö������Ϊ���������б���Ӷ���
	// Ҳ����˵���� ToolkitCommands�в���֮�����ǽ��� 2DviewportCommands�в��ҡ�
	TSharedPtr<SFDAssistor2DViewport> ViewportWidget = StaticCastSharedPtr<SFDAssistor2DViewport>(ViewportTabContent->GetFirstViewport());
	ViewportWidget->GetCommandList()->Append(ToolkitCommands);

	
}

#undef LOCTEXT_NAMESPACE