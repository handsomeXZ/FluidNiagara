// Copyright Epic Games, Inc. All Rights Reserved.

#include "FullBodyOverlayEditor/FDFBOverlayEditorMode.h"

#include "FullBodyOverlayEditor/FDFBOverlayAssetEditorTooltik.h"
#include "EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "FullBodyOverlayEditor/FDFBOverlayEditorModeCommands.h"
#include "FullBodyOverlayEditor/FDFBOverlayEditorSubsystem.h"
#include "FullBodyOverlayEditor/FDAssistorMeshInput.h"
#include "Context/FDAssistorContextObject.h"
#include "Context/FDAssistorLive2DPreviewAPI.h"
#include "Context/FDAssistorLive3DPreviewAPI.h"
#include "Context/FDAssistorViewportButtonsAPI.h"
#include "Context/FDAssistorInitializationContext.h"
#include "Context/Selection/FDAssistorSelectionAPI.h"
#include "Context/FDAssistorAssetInputsContext.h"
#include "FDAssistorEditorModeToolkit.h"
#include "ContextObjectStore.h"
#include "AssetEditorModeManager.h"

// step 2: register a ToolBuilder in FFDFBOverlayEditorMode::Enter() below

#define LOCTEXT_NAMESPACE "FDFBOverlayEditorMode"

using namespace UE::Geometry;

const FEditorModeID UFDFBOverlayEditorMode::EM_FDFBOverlayEditorModeId = TEXT("EM_FDFBOverlayEditorMode");

const FString UFDFBOverlayEditorMode::ToolName = TEXT("FDFBOverlay_ActorInfoTool");

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
			UFDFBOverlayEditorMode* Mode = Cast<UFDFBOverlayEditorMode>(Object);
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
			UFDFBOverlayEditorMode* Mode = Cast<UFDFBOverlayEditorMode>(Object);
			return !(Mode && Mode->IsActive() && !Mode->IsDefaultToolActive());
		}

		virtual FString ToString() const override
		{
			return TEXT("UVEditorModeLocals::FUVEditorBeginToolChange");
		}
	};

}

UFDFBOverlayEditorMode::UFDFBOverlayEditorMode()
{
	// appearance and icon in the editing mode ribbon can be customized here
	Info = FEditorModeInfo(UFDFBOverlayEditorMode::EM_FDFBOverlayEditorModeId,
		LOCTEXT("ModeName", "FDFBOverlay"),
		FSlateIcon(),
		false);
}



void UFDFBOverlayEditorMode::ActorSelectionChangeNotify()
{
}

void UFDFBOverlayEditorMode::Enter()
{
	UEdMode::Enter();
	
	// TODO: What is this actually used for?
	//bPIEModeActive = false;
	//if (GEditor->PlayWorld != NULL || GEditor->bIsSimulatingInEditor)
	//{
	//	bPIEModeActive = true;
	//	SetSimulationWarning(true);
	//}

	//BeginPIEDelegateHandle = FEditorDelegates::PreBeginPIE.AddLambda([this](bool bSimulating)
	//	{
	//		bPIEModeActive = true;
	//		SetSimulationWarning(true);
	//	});

	//EndPIEDelegateHandle = FEditorDelegates::EndPIE.AddLambda([this](bool bSimulating)
	//	{
	//		bPIEModeActive = false;
	//		ActivateDefaultTool();
	//		SetSimulationWarning(false);
	//	});

	//CancelPIEDelegateHandle = FEditorDelegates::CancelPIE.AddLambda([this]()
	//	{
	//		bPIEModeActive = false;
	//		ActivateDefaultTool();
	//		SetSimulationWarning(false);
	//	});


	InitializeModeContexts();

	bIsActive = true;
	
}

void UFDFBOverlayEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FFDAssistorEditorModeToolkit);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UFDFBOverlayEditorMode::GetModeCommands() const
{
	return FFDFBOverlayEditorModeCommands::Get().GetCommands();
}

void UFDFBOverlayEditorMode::InitializeAssetEditorContexts(UContextObjectStore& ContextStore,
	const TArray<TObjectPtr<UObject>>& AssetsIn, const TArray<FTransform>& TransformsIn, 
	FEditorViewportClient& LivePreviewViewportClient, FAssetEditorModeManager& LivePreviewModeManager, 
	UFDAssistorViewportButtonsAPI& ViewportButtonsAPI, UFDAssistorLive2DViewportAPI& FDAssistorLive2DViewportAPI)
{
	using namespace FDEditorModeLocals;
	//UFDFBOverlayAssetInputsContext* AssetInputsContext = ContextStore.FindContext<UFDFBOverlayAssetInputsContext>();
	//if (!AssetInputsContext)
	//{
	//	AssetInputsContext = NewObject<UFDFBOverlayAssetInputsContext>();
	//	AssetInputsContext->Initialize(AssetsIn, TransformsIn);
	//	ContextStore.AddContextObject(AssetInputsContext);
	//}

	UFDAssistorLive3DPreviewAPI* Live3DPreviewAPI = ContextStore.FindContext<UFDAssistorLive3DPreviewAPI>();
	if (!Live3DPreviewAPI)
	{
		Live3DPreviewAPI = NewObject<UFDAssistorLive3DPreviewAPI>();
		Live3DPreviewAPI->Initialize(
			LivePreviewModeManager.GetPreviewScene()->GetWorld(),
			LivePreviewModeManager.GetInteractiveToolsContext()->InputRouter,
			[LivePreviewViewportClientPtr = &LivePreviewViewportClient](FViewCameraState& CameraStateOut) {
				GetCameraState(*LivePreviewViewportClientPtr, CameraStateOut);
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

	//// Prep the editor-only context that we use to pass things to the mode.
	//if (!ContextStore.FindContext<UFDAssistorInitializationContext>())
	//{
	//	UFDAssistorInitializationContext* InitContext = NewObject<UFDAssistorInitializationContext>();
	//	InitContext->LivePreviewITC = Cast<UFDAssistorInitializationContext>(LivePreviewModeManager.GetInteractiveToolsContext());
	//	ContextStore.AddContextObject(InitContext);
	//}

	if (!ContextStore.FindContext<UFDAssistorViewportButtonsAPI>())
	{
		ContextStore.AddContextObject(&ViewportButtonsAPI);
	}

	if (!ContextStore.FindContext<UFDAssistorLive2DViewportAPI>())
	{
		ContextStore.AddContextObject(&FDAssistorLive2DViewportAPI);
	}


}
//void UFDFBOverlayEditorMode::Render(IToolsContextRenderAPI* RenderAPI)
//{
//	if (SelectionAPI)
//	{
//		SelectionAPI->Render(RenderAPI);
//	}
//}
//
//void UFDFBOverlayEditorMode::DrawHUD(FCanvas* Canvas, IToolsContextRenderAPI* RenderAPI)
//{
//	if (SelectionAPI)
//	{
//		SelectionAPI->DrawHUD(Canvas, RenderAPI);
//	}
//}

void UFDFBOverlayEditorMode::InitializeModeContexts() 
{
	UContextObjectStore* ContextStore = GetInteractiveToolsContext()->ToolManager->GetContextObjectStore();

	UFDAssistorAssetInputsContext* AssetInputsContext = ContextStore->FindContext<UFDAssistorAssetInputsContext>();
	check(AssetInputsContext);
	AssetInputsContext->GetAssetInputs(OriginalObjectsToEdit, Transforms);
	ContextsToUpdateOnToolEnd.Add(AssetInputsContext);

	UFDAssistorLive3DPreviewAPI* Live3DPreviewAPI = ContextStore->FindContext<UFDAssistorLive3DPreviewAPI>();
	check(Live3DPreviewAPI);
	LivePreviewWorld = Live3DPreviewAPI->GetLivePreviewWorld();
	ContextsToUpdateOnToolEnd.Add(Live3DPreviewAPI);

	UFDAssistorViewportButtonsAPI* ViewportButtonsAPI = ContextStore->FindContext<UFDAssistorViewportButtonsAPI>();
	check(ViewportButtonsAPI);
	ContextsToUpdateOnToolEnd.Add(ViewportButtonsAPI);

	UFDAssistorLive2DViewportAPI* Live2DPreviewAPI = ContextStore->FindContext<UFDAssistorLive2DViewportAPI>();
	check(Live2DPreviewAPI);
	ContextsToUpdateOnToolEnd.Add(Live2DPreviewAPI);


	// �����������������ģʽ�Լ������������ģ������Ǵ��ʲ��༭���л�ȡ��
	auto AddContextObject = [this, ContextStore](UFDAssistorContextObject* Object)
	{
		if (ensure(ContextStore->AddContextObject(Object)))
		{
			ContextsToShutdown.Add(Object);
		}
		ContextsToUpdateOnToolEnd.Add(Object);
	};

	/*UUVToolEmitChangeAPI* EmitChangeAPI = NewObject<UUVToolEmitChangeAPI>();
	EmitChangeAPI = NewObject<UUVToolEmitChangeAPI>();
	EmitChangeAPI->Initialize(GetInteractiveToolsContext()->ToolManager);
	AddContextObject(EmitChangeAPI);*/

	//UUVToolAssetAndChannelAPI* AssetAndLayerAPI = NewObject<UUVToolAssetAndChannelAPI>();
	//AssetAndLayerAPI->RequestChannelVisibilityChangeFunc = [this](const TArray<int32>& LayerPerAsset, bool bEmitUndoTransaction) {
	//	SetDisplayedUVChannels(LayerPerAsset, bEmitUndoTransaction);
	//};
	//AssetAndLayerAPI->NotifyOfAssetChannelCountChangeFunc = [this](int32 AssetID) {
	//	// Don't currently need to do anything because the layer selection menu gets populated
	//	// from scratch each time that it's opened.
	//};
	//AssetAndLayerAPI->GetCurrentChannelVisibilityFunc = [this]() {
	//	TArray<int32> VisibleLayers;
	//	VisibleLayers.SetNum(ToolTargets.Num());
	//	for (int32 AssetID = 0; AssetID < ToolTargets.Num(); ++AssetID)
	//	{
	//		VisibleLayers[AssetID] = GetDisplayedChannel(AssetID);
	//	}
	//	return VisibleLayers;
	//};
	//AddContextObject(AssetAndLayerAPI);

	/*SelectionAPI = NewObject<UUVToolSelectionAPI>();
	SelectionAPI->Initialize(GetToolManager(), GetWorld(),
		GetInteractiveToolsContext()->InputRouter, LivePreviewAPI, EmitChangeAPI);
	AddContextObject(SelectionAPI);

	UUVEditorToolPropertiesAPI* UVEditorToolPropertiesAPI = NewObject<UUVEditorToolPropertiesAPI>();
	AddContextObject(UVEditorToolPropertiesAPI);*/


}

/*
void UFDFBOverlayEditorMode::InitializeTargets()
{
	using namespace UVEditorModeLocals;

	check(LivePreviewWorld);

	// Build the tool targets that provide us with 3d dynamic meshes
	UFDFBOverlayEditorSubsystem* UVSubsystem = GEditor->GetEditorSubsystem<UFDFBOverlayEditorSubsystem>();
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

		FComponentMaterialSet MaterialSet = UE::ToolTarget::GetMaterialSet(Target);
		AppliedPreview->ConfigureMaterials(MaterialSet.Materials,
			ToolSetupUtil::GetDefaultWorkingMaterial(GetToolManager()));
		AppliedPreviews.Add(AppliedPreview);
	}


	// ������UV unwrapʱ����Щ������ȷ��UVֵ�ͽ����unwrap���񶥵�λ��֮���ӳ�䡣
	// ����������¿��򿪵�����Z�ᳯ�����ǣ�����ϣ��U����ȷ�ģ�V�����ϵġ���Unreal����������ϵ�У�����ζ�����ǽ�Uӳ�䵽����Y����Vӳ�䵽����X��
	// ���⣬Unreal���ڲ������������V�������Ϊ1-V����������ʾUV��ʱ������һ�㣬��Ϊ�û�����ϣ������ԭʼ��UV(����ʹ��UDIM�ʲ����û���˵���⽫�ر��������������ڲ��洢��V�����Ը�����)��
	// ScaleFactorֻ��������������������Ը����׵ؽ�һ���Ŵ���ʾ��������������ӽ�ƽ�����ʱ�������⡣


	// ���칤��ʵ�ʲ����������������
	for (int32 AssetID = 0; AssetID < ToolTargets.Num(); ++AssetID)
	{
		UFDAssistorMeshInput* ToolInputObject = NewObject<UFDAssistorMeshInput>();

		if (!ToolInputObject->InitializeMeshes(ToolTargets[AssetID], AppliedCanonicalMeshes[AssetID],
			AppliedPreviews[AssetID], AssetID, DefaultUVLayerIndex,
			GetWorld(), LivePreviewWorld, ToolSetupUtil::GetDefaultWorkingMaterial(GetToolManager()),
			FUVEditorUXSettings::UVToVertPosition, FUVEditorUXSettings::VertPositionToUV))
		{
			return;
		}

		if (Transforms.Num() == ToolTargets.Num())
		{
			ToolInputObject->AppliedPreview->PreviewMesh->SetTransform(Transforms[AssetID]);
		}

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
		ToolInputObject->OnCanonicalModified.AddWeakLambda(this, [this]
		(UUVEditorToolMeshInput* InputObject, const UFDAssistorMeshInput::FCanonicalModifiedInfo&) {
				ModifiedAssetIDs.Add(InputObject->AssetID);
			});

		ToolInputObjects.Add(ToolInputObject);
	}

	// Ϊ layer/channel selection ��׼��
	InitializeAssetNames(ToolTargets, AssetNames);
	PendingUVLayerIndex.SetNumZeroed(ToolTargets.Num());


	// Finish initializing the selection api
	SelectionAPI->SetTargets(ToolInputObjects);

}
*/



const FToolTargetTypeRequirements& UFDFBOverlayEditorMode::GetToolTargetRequirements()
{
	static const FToolTargetTypeRequirements ToolTargetRequirements =
		FToolTargetTypeRequirements({
			UMaterialProvider::StaticClass(),
			UDynamicMeshCommitter::StaticClass(),
			UDynamicMeshProvider::StaticClass()
			});
	return ToolTargetRequirements;
}

bool UFDFBOverlayEditorMode::IsDefaultToolActive()
{
	return GetInteractiveToolsContext() && GetInteractiveToolsContext()->IsToolActive(EToolSide::Mouse, DefaultToolIdentifier);
}

void UFDFBOverlayEditorMode::ActivateDefaultTool()
{
	if (!bPIEModeActive)
	{
		GetInteractiveToolsContext()->StartTool(DefaultToolIdentifier);
	}
}






#undef LOCTEXT_NAMESPACE
