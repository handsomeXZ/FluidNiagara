// Copyright HandsomeCheese. All Rights Reserved.

#include "FDOverlayEditorMode.h"

#include "FDOverlayAssetEditorTooltik.h"
#include "FDOverlayEditorModeCommands.h"
#include "FDOverlayEditorSubsystem.h"
#include "FDOverlayMeshInput.h"
#include "FDOverlayEditorModeToolkit.h"
#include "FDOverlay3DViewportClient.h"
#include "Context/FDOverlayContextObject.h"
#include "Context/FDOverlayLive2DPreviewAPI.h"
#include "Context/FDOverlayLive3DPreviewAPI.h"
#include "Context/FDOverlayViewportButtonsAPI.h"
#include "Context/FDOverlayInitializationContext.h"
#include "Context/Selection/FDOverlaySelectionAPI.h"
#include "Context/FDOverlayAssetInputsContext.h"
#include "Tools/FDOverlayEditorAutoCalTool.h"

#include "UVEditorUXSettings.h"

#include "EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "ContextObjectStore.h"
#include "AssetEditorModeManager.h"
#include "TargetInterfaces/MaterialProvider.h"
#include "TargetInterfaces/AssetBackedTarget.h"
#include "TargetInterfaces/DynamicMeshCommitter.h"
#include "TargetInterfaces/DynamicMeshProvider.h"
#include "TargetInterfaces/MeshDescriptionProvider.h"
#include "TargetInterfaces/DynamicMeshSource.h"
#include "ModelingToolTargetUtil.h"
#include "MeshOpPreviewHelpers.h" // UMeshOpPreviewWithBackgroundCompute
#include "ToolSetupUtil.h"
#include "MeshDescriptionToDynamicMesh.h"
#include "DynamicMeshToMeshDescription.h"
#include "MeshDescription.h"
#include "DynamicMesh\DynamicMesh3.h"
#include "UDynamicMesh.h"
#include "UObject\ConstructorHelpers.h"


// step 2: register a ToolBuilder in FFDOverlayEditorMode::Enter() below

#define LOCTEXT_NAMESPACE "FDOverlayEditorMode"

using namespace UE::Geometry;

const FEditorModeID UFDOverlayEditorMode::EM_FDOverlayEditorModeId = TEXT("EM_FDOverlayEditorMode");

const FString UFDOverlayEditorMode::ToolName = TEXT("FDOverlay_ActorInfoTool");

namespace FDEditorModeLocals
{
	// The layer we open when we first open the UV editor
	const int32 DefaultUVLayerIndex = 0;

	const FText UVLayerChangeTransactionName = LOCTEXT("UVLayerChangeTransactionName", "Change UV Layer");

	void GetCameraState(const FEditorViewportClient& ViewportClientIn, FViewCameraState& CameraStateOut)
	{
		FViewportCameraTransform ViewTransform = ViewportClientIn.GetViewTransform();
		CameraStateOut.bIsOrthographic = false;
		CameraStateOut.bIsVR = false;
		CameraStateOut.Position = ViewTransform.GetLocation();
		CameraStateOut.HorizontalFOVDegrees = ViewportClientIn.ViewFOV;
		CameraStateOut.AspectRatio = ViewportClientIn.AspectRatio;

		// if using Orbit camera, the rotation in the ViewTransform is not the current camera rotation, it
		// is set to a different rotation based on the Orbit. So we have to convert back to camera rotation.
		FRotator ViewRotation = (ViewportClientIn.bUsingOrbitCamera) ?
			ViewTransform.ComputeOrbitMatrix().InverseFast().Rotator() : ViewTransform.GetRotation();
		CameraStateOut.Orientation = ViewRotation.Quaternion();
	}

	/**
	 * Support for undoing a tool start in such a way that we go back to the mode's default
	 * tool on undo.
	 */
	class FUVEditorBeginToolChange : public FToolCommandChange
	{
	public:
		virtual void Apply(UObject* Object) override
		{
			// Do nothing, since we don't allow a re-do back into a tool
		}

		virtual void Revert(UObject* Object) override
		{
			UFDOverlayEditorMode* Mode = Cast<UFDOverlayEditorMode>(Object);
			// Don't really need the check for default tool since we theoretically shouldn't
			// be issuing this transaction for starting the default tool, but still...
			if (Mode && !Mode->IsDefaultToolActive())
			{
				Mode->GetInteractiveToolsContext()->EndTool(EToolShutdownType::Cancel);
				Mode->ActivateDefaultTool();
			}
		}

		virtual bool HasExpired(UObject* Object) const override
		{
			// To not be expired, we must be active and in some non-default tool.
			UFDOverlayEditorMode* Mode = Cast<UFDOverlayEditorMode>(Object);
			return !(Mode && Mode->IsActive() && !Mode->IsDefaultToolActive());
		}

		virtual FString ToString() const override
		{
			return TEXT("UVEditorModeLocals::FUVEditorBeginToolChange");
		}
	};

}

UFDOverlayEditorMode::UFDOverlayEditorMode()
{
	// appearance and icon in the editing mode ribbon can be customized here
	Info = FEditorModeInfo(UFDOverlayEditorMode::EM_FDOverlayEditorModeId,
		LOCTEXT("ModeName", "FDOverlay"),
		FSlateIcon(),
		false);
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UMaterial> DefaultAppliedMaterialAsset;
		ConstructorHelpers::FObjectFinder<UMaterial> EmissiveAppliedMaterialAsset;
		ConstructorHelpers::FObjectFinder<UMaterial> TranslucencyAppliedMaterialAsset;
		ConstructorHelpers::FObjectFinder<UMaterial> TransitionAppliedMaterialAsset;
		ConstructorHelpers::FObjectFinder<UMaterial> UnwrapMaterialAsset;
		FConstructorStatics()
			:DefaultAppliedMaterialAsset(TEXT("Material'/FDOverlayEditor/M_Default.M_Default'")),
			EmissiveAppliedMaterialAsset(TEXT("Material'/FDOverlayEditor/M_DefaultEmissive.M_DefaultEmissive'")),
			TranslucencyAppliedMaterialAsset(TEXT("Material'/FDOverlayEditor/M_DefaultTranslucency.M_DefaultTranslucency'")),
			TransitionAppliedMaterialAsset(TEXT("Material'/FDOverlayEditor/M_DefaultTransition.M_DefaultTransition'")),
			UnwrapMaterialAsset(TEXT("Material'/FDOverlayEditor/M_Default_UV.M_Default_UV'")){}
			// ���� UVUnwrap ���ܴ��ڱ������ǣ�������Ҫ���ʿ��� ��˫�桱
	};
	static FConstructorStatics ConstructorStatics;

	DefaultBakeAppliedMaterialInterface = ConstructorStatics.DefaultAppliedMaterialAsset.Object;
	EmissiveBakeAppliedMaterialInterface = ConstructorStatics.EmissiveAppliedMaterialAsset.Object;
	TranslucencyBakeAppliedMaterialInterface = ConstructorStatics.TranslucencyAppliedMaterialAsset.Object;
	TransitionBakeAppliedMaterialInterface = ConstructorStatics.TransitionAppliedMaterialAsset.Object;
	DefaultBakeUnwrapMaterialInterface = ConstructorStatics.UnwrapMaterialAsset.Object;
}



void UFDOverlayEditorMode::Enter()
{
	UEdMode::Enter();
	
	bPIEModeActive = false;
	if (GEditor->PlayWorld != NULL || GEditor->bIsSimulatingInEditor)
	{
		bPIEModeActive = true;
		//SetSimulationWarning(true);
	}

	BeginPIEDelegateHandle = FEditorDelegates::PreBeginPIE.AddLambda([this](bool bSimulating)
	{
		bPIEModeActive = true;
		//SetSimulationWarning(true);
	});

	EndPIEDelegateHandle = FEditorDelegates::EndPIE.AddLambda([this](bool bSimulating)
	{
		bPIEModeActive = false;
		//ActivateDefaultTool();
		//SetSimulationWarning(false);
	});



	SettingProperties = NewObject<UFDOverlaySettingProperties>(this);

	int32 WatchId = SettingProperties->WatchProperty(SettingProperties->type,
		[this](EAutoCalToolOutputType type) {
			UE_LOG(LogTemp, Warning, TEXT("Test WatchProperty"));
			UContextObjectStore* ContextStore = GetInteractiveToolsContext()->ToolManager->GetContextObjectStore();
			UFDOverlayAutoCalToolAPI* AutoCalToolAPI = ContextStore->FindContext<UFDOverlayAutoCalToolAPI>();
			if (AutoCalToolAPI)
			{
				AutoCalToolAPI->SetOutputType(type);
			}
		});
	SettingProperties->SilentUpdateWatcherAtIndex(WatchId);
	SettingProperties->CheckAndUpdateWatched();
	PropertyObjectsToTick.Add(SettingProperties);

	RegisterTools();
	InitializeModeContexts();
	InitializeTargets();
	bIsActive = true;

}

void UFDOverlayEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FFDOverlayEditorModeToolkit);
}


void UFDOverlayEditorMode::InitializeAssetEditorContexts(UContextObjectStore& ContextStore,
	const TArray<TObjectPtr<UObject>>& AssetsIn, const TArray<FTransform>& TransformsIn, 
	FEditorViewportClient& LivePreviewViewportClient, FEditorViewportClient& Live2DViewportClient, 
	FAssetEditorModeManager& LivePreviewModeManager, UFDOverlayViewportButtonsAPI& ViewportButtonsAPI)
{
	using namespace FDEditorModeLocals;
	UFDOverlayAssetInputsContext* AssetInputsContext = ContextStore.FindContext<UFDOverlayAssetInputsContext>();
	if (!AssetInputsContext)
	{
		AssetInputsContext = NewObject<UFDOverlayAssetInputsContext>();
		AssetInputsContext->Initialize(AssetsIn, TransformsIn);
		ContextStore.AddContextObject(AssetInputsContext);
	}

	UFDOverlayLive3DPreviewAPI* Live3DPreviewAPI = ContextStore.FindContext<UFDOverlayLive3DPreviewAPI>();
	if (!Live3DPreviewAPI)
	{
		Live3DPreviewAPI = NewObject<UFDOverlayLive3DPreviewAPI>();
		Live3DPreviewAPI->Initialize(
			LivePreviewModeManager.GetPreviewScene()->GetWorld(),
			LivePreviewModeManager.GetInteractiveToolsContext()->InputRouter,
			[LivePreviewViewportClientPtr = &LivePreviewViewportClient]() -> FOnToggleOverlayChannel& {
				return ((FFDOverlay3DViewportClient*)LivePreviewViewportClientPtr)->OnToggleOverlayChannel();
			},
			[LivePreviewViewportClientPtr = &LivePreviewViewportClient]() -> FOnToggleOverlayRender& {
				return ((FFDOverlay3DViewportClient*)LivePreviewViewportClientPtr)->OnToggleOverlayRender();
			},
			[LivePreviewViewportClientPtr = &LivePreviewViewportClient](const FAxisAlignedBox3d& BoundingBox) {
				// We check for the Viewport here because it might not be open at the time this
				// method is called, e.g. during startup with an initially closed tab. And since
				// the FocusViewportOnBox method doesn't check internally that the Viewport is
				// available, this can crash.
				if (LivePreviewViewportClientPtr && LivePreviewViewportClientPtr->Viewport)
				{
					LivePreviewViewportClientPtr->FocusViewportOnBox((FBox)BoundingBox, true);
				}
			}
			);
		ContextStore.AddContextObject(Live3DPreviewAPI);
	}

	UFDOverlayLive2DViewportAPI* Live2DViewportAPI = ContextStore.FindContext<UFDOverlayLive2DViewportAPI>();
	if (!Live2DViewportAPI)
	{
		Live2DViewportAPI = NewObject<UFDOverlayLive2DViewportAPI>();
		Live2DViewportAPI->InitializeDelegate(
			[Live2DViewportClientPtr = &Live2DViewportClient]() -> FOnSwitchMaterialIDMode& {
				return ((FFDOverlay2DViewportClient*)Live2DViewportClientPtr)->OnSwitchMaterialIDMode();
			},
			[Live2DViewportClientPtr = &Live2DViewportClient]() -> FOnMaterialIDAdd& {
				return ((FFDOverlay2DViewportClient*)Live2DViewportClientPtr)->OnMaterialIDAdd();
			},
			[Live2DViewportClientPtr = &Live2DViewportClient]() -> FOnMaterialIDSub& {
				return ((FFDOverlay2DViewportClient*)Live2DViewportClientPtr)->OnMaterialIDSub();
			}
		);
		ContextStore.AddContextObject(Live2DViewportAPI);
	}

	UFDOverlayAutoCalToolAPI* AutoCalToolAPI = ContextStore.FindContext<UFDOverlayAutoCalToolAPI>();
	if (!AutoCalToolAPI)
	{
		AutoCalToolAPI = NewObject<UFDOverlayAutoCalToolAPI>();
		ContextStore.AddContextObject(AutoCalToolAPI);
	}

	if (!ContextStore.FindContext<UFDOverlayViewportButtonsAPI>())
	{
		ContextStore.AddContextObject(&ViewportButtonsAPI);
	}

}


void UFDOverlayEditorMode::InitializeModeContexts() 
{
	UContextObjectStore* ContextStore = GetInteractiveToolsContext()->ToolManager->GetContextObjectStore();

	UFDOverlayAssetInputsContext* AssetInputsContext = ContextStore->FindContext<UFDOverlayAssetInputsContext>();
	check(AssetInputsContext);
	AssetInputsContext->GetAssetInputs(OriginalObjectsToEdit, Transforms);
	ContextsToUpdateOnToolEnd.Add(AssetInputsContext);

	UFDOverlayLive3DPreviewAPI* Live3DPreviewAPI = ContextStore->FindContext<UFDOverlayLive3DPreviewAPI>();
	check(Live3DPreviewAPI);
	LivePreviewWorld = Live3DPreviewAPI->GetLivePreviewWorld();
	ContextsToUpdateOnToolEnd.Add(Live3DPreviewAPI);


	// �����������������ģʽ�Լ������������ģ������Ǵ��ʲ��༭���л�ȡ��
	auto AddContextObject = [this, ContextStore](UFDOverlayContextObject* Object)
	{
		if (ensure(ContextStore->AddContextObject(Object)))
		{
			ContextsToShutdown.Add(Object);
		}
		ContextsToUpdateOnToolEnd.Add(Object);
	};


}

void UFDOverlayEditorMode::InitializeTargets()
{
	using namespace FDEditorModeLocals;

	check(LivePreviewWorld);

	// Build the tool targets that provide us with 3d dynamic meshes
	UFDOverlayEditorSubsystem* UVSubsystem = GEditor->GetEditorSubsystem<UFDOverlayEditorSubsystem>();
	UContextObjectStore* ContextStore = GetInteractiveToolsContext()->ToolManager->GetContextObjectStore();
	UFDOverlayLive3DPreviewAPI* Live3DPreviewAPI = ContextStore->FindContext<UFDOverlayLive3DPreviewAPI>();
	UFDOverlayLive2DViewportAPI* Live2DViewportAPI = ContextStore->FindContext<UFDOverlayLive2DViewportAPI>();

	UVSubsystem->BuildTargets(OriginalObjectsToEdit, GetToolTargetRequirements(), ToolTargets);

	// �ռ�Ŀ�����ά��̬����ÿ���ʲ�������һ����ÿ���ʲ���AssetID������Щ�����������
	// �������������(�����ض���UV��)��ָ����Щ���е�3d����
	for (UToolTarget* Target : ToolTargets)
	{
		// applied canonical mesh ��Ӧ��������ͼ��仯��3d��������л�����ͬ�Ĳ㣬���Ľ������� applied canonical �С�
		TSharedPtr<FDynamicMesh3> AppliedCanonical = MakeShared<FDynamicMesh3>(UE::ToolTarget::GetDynamicMeshCopy(Target));
		AppliedCanonicalMeshes.Add(AppliedCanonical);
		
		// ���� applied canonical ��Ԥ���汾�����߿��Ը��Ӽ��㵽������������Ǳ���С�ģ������������������ʾһ���ʲ���
		// �Ա�����ͼ�������� Computes ��ͬһ��Ԥ������(����������£�һ�������׳�)
		UMeshOpPreviewWithBackgroundCompute* AppliedPreview = NewObject<UMeshOpPreviewWithBackgroundCompute>();
		AppliedPreview->Setup(LivePreviewWorld);
		AppliedPreview->PreviewMesh->UpdatePreview(AppliedCanonical.Get());

		//FComponentMaterialSet MaterialSet = UE::ToolTarget::GetMaterialSet(Target);

		//AppliedPreview->ConfigureMaterials(MaterialSet.Materials,
		//	ToolSetupUtil::GetDefaultWorkingMaterial(GetToolManager()));
		AppliedPreviews.Add(AppliedPreview);
	}


	// ������UV unwrapʱ����Щ������ȷ��UVֵ�ͽ����unwrap���񶥵�λ��֮���ӳ�䡣
	// ����������¿��򿪵�����Z�ᳯ�����ǣ�����ϣ��U����ȷ�ģ�V�����ϵġ���Unreal����������ϵ�У�����ζ�����ǽ�Uӳ�䵽����Y����Vӳ�䵽����X��
	// ���⣬Unreal���ڲ������������V�������Ϊ1-V����������ʾUV��ʱ������һ�㣬��Ϊ�û�����ϣ������ԭʼ��UV(����ʹ��UDIM�ʲ����û���˵���⽫�ر��������������ڲ��洢��V�����Ը�����)��
	// ScaleFactorֻ��������������������Ը����׵ؽ�һ���Ŵ���ʾ��������������ӽ�ƽ�����ʱ�������⡣


	// ���칤��ʵ�ʲ����������������
	for (int32 AssetID = 0; AssetID < ToolTargets.Num(); ++AssetID)
	{
		UFDOverlayMeshInput* ToolInputObject = NewObject<UFDOverlayMeshInput>();

		if (!ToolInputObject->InitializeMeshes(ToolTargets[AssetID], AppliedCanonicalMeshes[AssetID],
			AppliedPreviews[AssetID], AssetID, DefaultUVLayerIndex, GetWorld(), LivePreviewWorld, DefaultBakeAppliedMaterialInterface, 
			EmissiveBakeAppliedMaterialInterface, TranslucencyBakeAppliedMaterialInterface, TransitionBakeAppliedMaterialInterface, DefaultBakeUnwrapMaterialInterface,
			FUVEditorUXSettings::UVToVertPosition, FUVEditorUXSettings::VertPositionToUV))
		{
			return;
		}

		if (Transforms.Num() == ToolTargets.Num())
		{
			ToolInputObject->AppliedPreview->PreviewMesh->SetTransform(Transforms[AssetID]);
		}
		
		Live3DPreviewAPI->OnToggleOverlayChannelDelegate().AddUObject(ToolInputObject, &UFDOverlayMeshInput::ChangeDynamicMaterialDisplayChannel);
		Live3DPreviewAPI->OnToggleOverlayRenderDelegate().AddUObject(ToolInputObject, &UFDOverlayMeshInput::ChangeDynamicMaterialDisplayRender);
		Live2DViewportAPI->OnSwitchMaterialIDModeDelegate().AddUObject(ToolInputObject, &UFDOverlayMeshInput::SwitchDynamicMaterialDisplayIDMode);
		Live2DViewportAPI->OnMaterialIDAddDelegate().AddUObject(ToolInputObject, &UFDOverlayMeshInput::AddDynamicMaterialDisplayID);
		Live2DViewportAPI->OnMaterialIDSubDelegate().AddUObject(ToolInputObject, &UFDOverlayMeshInput::SubDynamicMaterialDisplayID);

		ToolInputObject->UnwrapPreview->PreviewMesh->SetMaterial(
			0, ToolSetupUtil::GetCustomTwoSidedDepthOffsetMaterial(
				GetToolManager(),
				FUVEditorUXSettings::GetTriangleColorByTargetIndex(AssetID),
				FUVEditorUXSettings::UnwrapTriangleDepthOffset,
				FUVEditorUXSettings::UnwrapTriangleOpacity));

		// ���ǲ�����Ҫ�߿���ʾģʽ���������Ҫ������Բο�����ע�ʹ��롣
		//// Set up the wireframe display of the unwrapped mesh.
		//UMeshElementsVisualizer* WireframeDisplay = NewObject<UMeshElementsVisualizer>(this);
		//WireframeDisplay->CreateInWorld(GetWorld(), FTransform::Identity);

		//WireframeDisplay->Settings->DepthBias = FUVEditorUXSettings::WireframeDepthOffset;
		//WireframeDisplay->Settings->bAdjustDepthBiasUsingMeshSize = false;
		//WireframeDisplay->Settings->bShowWireframe = true;
		//WireframeDisplay->Settings->bShowBorders = true;
		//WireframeDisplay->Settings->WireframeColor = FUVEditorUXSettings::GetWireframeColorByTargetIndex(AssetID).ToFColorSRGB();
		//WireframeDisplay->Settings->BoundaryEdgeColor = FUVEditorUXSettings::GetBoundaryColorByTargetIndex(AssetID).ToFColorSRGB(); ;
		//WireframeDisplay->Settings->bShowUVSeams = false;
		//WireframeDisplay->Settings->bShowNormalSeams = false;
		//// These are not exposed at the visualizer level yet
		//// TODO: Should they be?
		//WireframeDisplay->WireframeComponent->BoundaryEdgeThickness = 2;

		//// The wireframe will track the unwrap preview mesh
		//WireframeDisplay->SetMeshAccessFunction([ToolInputObject](UMeshElementsVisualizer::ProcessDynamicMeshFunc ProcessFunc) {
		//	ToolInputObject->UnwrapPreview->ProcessCurrentMesh(ProcessFunc);
		//	});

		//// The settings object and wireframe are not part of a tool, so they won't get ticked like they
		//// are supposed to (to enable property watching), unless we add this here.
		//PropertyObjectsToTick.Add(WireframeDisplay->Settings);
		//WireframesToTick.Add(WireframeDisplay);

		//// The tool input object will hold on to the wireframe for the purposes of updating it and cleaning it up
		//ToolInputObject->WireframeDisplay = WireframeDisplay;

		// �󶨵�ί�У��������ǾͿ��Լ�⵽�仯
		//ToolInputObject->OnCanonicalModified.AddWeakLambda(this, [this]
		//(UUVEditorToolMeshInput* InputObject, const UFDOverlayMeshInput::FCanonicalModifiedInfo&) {
		//		ModifiedAssetIDs.Add(InputObject->AssetID);
		//	});

		ToolInputObjects.Add(ToolInputObject);
	}

	// Ϊ layer/channel selection ��׼��
	//InitializeAssetNames(ToolTargets, AssetNames);
	//PendingUVLayerIndex.SetNumZeroed(ToolTargets.Num());


	// Finish initializing the selection api
	//SelectionAPI->SetTargets(ToolInputObjects);
	DefaultBakeAppliedMaterialInterface = nullptr;
	EmissiveBakeAppliedMaterialInterface = nullptr;
	TranslucencyBakeAppliedMaterialInterface = nullptr;
	TransitionBakeAppliedMaterialInterface = nullptr;
	DefaultBakeUnwrapMaterialInterface = nullptr;
}




const FToolTargetTypeRequirements& UFDOverlayEditorMode::GetToolTargetRequirements()
{
	static const FToolTargetTypeRequirements ToolTargetRequirements =
		FToolTargetTypeRequirements({
			UMaterialProvider::StaticClass(),
			UDynamicMeshCommitter::StaticClass(),
			UDynamicMeshProvider::StaticClass()
			});
	return ToolTargetRequirements;
}

bool UFDOverlayEditorMode::IsDefaultToolActive()
{
	return GetInteractiveToolsContext() && GetInteractiveToolsContext()->IsToolActive(EToolSide::Mouse, DefaultToolIdentifier);
}

void UFDOverlayEditorMode::ActivateDefaultTool()
{
	if (!bPIEModeActive)
	{
		GetInteractiveToolsContext()->StartTool(DefaultToolIdentifier);
	}
}

void UFDOverlayEditorMode::FocusLivePreviewCameraOnSelection()
{
	UContextObjectStore* ContextStore = GetInteractiveToolsContext()->ToolManager->GetContextObjectStore();
	UFDOverlayLive3DPreviewAPI* Live3DPreviewAPI = ContextStore->FindContext<UFDOverlayLive3DPreviewAPI>();
	if (!Live3DPreviewAPI)
	{
		return;
	}

	FAxisAlignedBox3d SelectionBoundingBox;

	for (const UFDOverlayMeshInput* target : ToolInputObjects)
	{
		SelectionBoundingBox.Contain(target->AppliedCanonical->GetBounds());
	}

	Live3DPreviewAPI->SetLivePreviewCameraToLookAtVolume(SelectionBoundingBox);
}

void UFDOverlayEditorMode::RegisterTools() 
{
	const FFDOverlayEditorModeCommands& CommandInfos = FFDOverlayEditorModeCommands::Get();


	UFDOverlayEditorAutoCalToolBuilder* FDOverlayEditorAutoCalToolBuilder = NewObject<UFDOverlayEditorAutoCalToolBuilder>();
	FDOverlayEditorAutoCalToolBuilder->Targets = &ToolInputObjects;
	FDOverlayEditorAutoCalToolBuilder->LivePreviewWorld = &LivePreviewWorld;
	
	RegisterTool(CommandInfos.AutoCalTool, TEXT("AutoCalTool"), FDOverlayEditorAutoCalToolBuilder);

}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UFDOverlayEditorMode::GetModeCommands() const
{
	return FFDOverlayEditorModeCommands::Get().GetCommands();
}

void UFDOverlayEditorMode::ApplyChanges()
{
	UContextObjectStore* ContextStore = GetInteractiveToolsContext()->ToolManager->GetContextObjectStore();
	UFDOverlayLive3DPreviewAPI* Live3DPreviewAPI = ContextStore->FindContext<UFDOverlayLive3DPreviewAPI>();
	if (Live3DPreviewAPI)
	{
		Live3DPreviewAPI->OnApplyChangesDelegate.Broadcast();
	}
}
bool UFDOverlayEditorMode::CanApplyChanges() const
{
	return !bPIEModeActive /*&& HaveUnappliedChanges()*/;
}

void UFDOverlayEditorMode::OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	
}

void UFDOverlayEditorMode::OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	for (TWeakObjectPtr<UFDOverlayContextObject> Context : ContextsToUpdateOnToolEnd)
	{
		if (ensure(Context.IsValid()))
		{
			Context->OnToolEnded(Tool);
		}
	}
}

void UFDOverlayEditorMode::Exit()
{

	// ToolsContext->EndTool only shuts the tool on the next tick, and ToolsContext->DeactivateActiveTool is
	// inaccessible, so we end up having to do this to force the shutdown right now.
	GetToolManager()->DeactivateTool(EToolSide::Mouse, EToolShutdownType::Cancel);


	for (TObjectPtr<UFDOverlayMeshInput> ToolInput : ToolInputObjects)
	{
		ToolInput->Shutdown();
	}
	ToolInputObjects.Reset();
	//WireframesToTick.Reset();
	OriginalObjectsToEdit.Reset();

	for (TObjectPtr<UMeshOpPreviewWithBackgroundCompute> Preview : AppliedPreviews)
	{
		Preview->Shutdown();
	}
	AppliedPreviews.Reset();
	AppliedCanonicalMeshes.Reset();
	ToolTargets.Reset();


	PropertyObjectsToTick.Empty();
	LivePreviewWorld = nullptr;

	bIsActive = false;

	UContextObjectStore* ContextStore = GetInteractiveToolsContext()->ToolManager->GetContextObjectStore();
	for (TWeakObjectPtr<UFDOverlayContextObject> Context : ContextsToShutdown)
	{
		if (ensure(Context.IsValid()))
		{
			Context->Shutdown();
			ContextStore->RemoveContextObject(Context.Get());
		}
	}


	FEditorDelegates::PreBeginPIE.Remove(BeginPIEDelegateHandle);
	FEditorDelegates::EndPIE.Remove(EndPIEDelegateHandle);
	FEditorDelegates::CancelPIE.Remove(CancelPIEDelegateHandle);
	//FEditorDelegates::PostSaveWorldWithContext.Remove(PostSaveWorldDelegateHandle);

	Super::Exit();
}

UObject* UFDOverlayEditorMode::GetSettingsObject()
{
	if (SettingProperties)
	{
		return SettingProperties;
	}
	return nullptr;
}

void UFDOverlayEditorMode::ModeTick(float DeltaTime)
{
	Super::ModeTick(DeltaTime);


	for (TObjectPtr<UInteractiveToolPropertySet>& Propset : PropertyObjectsToTick)
	{
		if (Propset)
		{
			if (Propset->IsPropertySetEnabled())
			{
				Propset->CheckAndUpdateWatched();
			}
			else
			{
				Propset->SilentUpdateWatched();
			}
		}
	}



}

#undef LOCTEXT_NAMESPACE
