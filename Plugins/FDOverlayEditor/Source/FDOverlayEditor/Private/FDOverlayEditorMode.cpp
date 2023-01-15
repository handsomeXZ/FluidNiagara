// Copyright Epic Games, Inc. All Rights Reserved.

#include "FDOverlayEditorMode.h"

#include "FDOverlayAssetEditorTooltik.h"
#include "FDOverlayEditorModeCommands.h"
#include "FDOverlayEditorSubsystem.h"
#include "FDOverlayMeshInput.h"
#include "FDOverlayEditorModeToolkit.h"
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
		ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset;
		FConstructorStatics()
			:MaterialAsset(TEXT("Material'/FDOverlayEditor/M_Default.M_Default'")) {}
	};
	static FConstructorStatics ConstructorStatics;

	DefaultBakeMaterialInterface = ConstructorStatics.MaterialAsset.Object;
}



void UFDOverlayEditorMode::Enter()
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
	FEditorViewportClient& LivePreviewViewportClient, FAssetEditorModeManager& LivePreviewModeManager, 
	UFDOverlayViewportButtonsAPI& ViewportButtonsAPI, UFDOverlayLive2DViewportAPI& FDOverlayLive2DViewportAPI)
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
	//if (!ContextStore.FindContext<UFDOverlayInitializationContext>())
	//{
	//	UFDOverlayInitializationContext* InitContext = NewObject<UFDOverlayInitializationContext>();
	//	InitContext->LivePreviewITC = Cast<UFDOverlayInitializationContext>(LivePreviewModeManager.GetInteractiveToolsContext());
	//	ContextStore.AddContextObject(InitContext);
	//}

	if (!ContextStore.FindContext<UFDOverlayViewportButtonsAPI>())
	{
		ContextStore.AddContextObject(&ViewportButtonsAPI);
	}

	if (!ContextStore.FindContext<UFDOverlayLive2DViewportAPI>())
	{
		ContextStore.AddContextObject(&FDOverlayLive2DViewportAPI);
	}


}
//void UFDOverlayEditorMode::Render(IToolsContextRenderAPI* RenderAPI)
//{
//	if (SelectionAPI)
//	{
//		SelectionAPI->Render(RenderAPI);
//	}
//}
//
//void UFDOverlayEditorMode::DrawHUD(FCanvas* Canvas, IToolsContextRenderAPI* RenderAPI)
//{
//	if (SelectionAPI)
//	{
//		SelectionAPI->DrawHUD(Canvas, RenderAPI);
//	}
//}

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

	//UFDOverlayViewportButtonsAPI* ViewportButtonsAPI = ContextStore->FindContext<UFDOverlayViewportButtonsAPI>();
	//check(ViewportButtonsAPI);
	//ContextsToUpdateOnToolEnd.Add(ViewportButtonsAPI);

	//UFDOverlayLive2DViewportAPI* Live2DPreviewAPI = ContextStore->FindContext<UFDOverlayLive2DViewportAPI>();
	//check(Live2DPreviewAPI);
	//ContextsToUpdateOnToolEnd.Add(Live2DPreviewAPI);


	// �����������������ģʽ�Լ������������ģ������Ǵ��ʲ��༭���л�ȡ��
	//auto AddContextObject = [this, ContextStore](UFDOverlayContextObject* Object)
	//{
	//	if (ensure(ContextStore->AddContextObject(Object)))
	//	{
	//		ContextsToShutdown.Add(Object);
	//	}
	//	ContextsToUpdateOnToolEnd.Add(Object);
	//};

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

//FDynamicMesh3 UFDOverlayEditorMode::GetDynamicMeshCopy(UToolTarget* Target, bool bWantMeshTangents)
//{
//	//IPersistentDynamicMeshSource* DynamicMeshSource = Cast<IPersistentDynamicMeshSource>(Target);
//	//if (DynamicMeshSource)
//	//{
//	//	UDynamicMesh* DynamicMesh = DynamicMeshSource->GetDynamicMeshContainer();
//	//	FDynamicMesh3 Mesh;
//	//	DynamicMesh->ProcessMesh([&](const FDynamicMesh3& ReadMesh) { Mesh = ReadMesh; });
//	//	return Mesh;
//	//}
//
//	// TODO: Handle tangent computation. For now skip if tangents requested.
//	IDynamicMeshProvider* DynamicMeshProvider = Cast<IDynamicMeshProvider>(Target);
//	if (DynamicMeshProvider && !bWantMeshTangents)
//	{
//		return DynamicMeshProvider->GetDynamicMesh();
//	}
//	FDynamicMesh3 mesh;
//	return mesh;
//
//
//	//IMeshDescriptionProvider* MeshDescriptionProvider = Cast<IMeshDescriptionProvider>(Target);
//	//FDynamicMesh3 Mesh(EMeshComponents::FaceGroups);
//	//Mesh.EnableAttributes();
//	//if (MeshDescriptionProvider)
//	//{
//	//	FMeshDescriptionToDynamicMesh Converter;
//	//	if (bWantMeshTangents)
//	//	{
//	//		FGetMeshParameters GetMeshParams;
//	//		GetMeshParams.bWantMeshTangents = true;
//	//		FMeshDescription MeshDescriptionCopy = MeshDescriptionProvider->GetMeshDescriptionCopy(GetMeshParams);
//	//		Converter.Convert(&MeshDescriptionCopy, Mesh, bWantMeshTangents);
//	//	}
//	//	else
//	//	{
//	//		Converter.Convert(MeshDescriptionProvider->GetMeshDescription(), Mesh, bWantMeshTangents);
//	//	}
//
//	//	return Mesh;
//	//}
//
//	//ensure(false);
//	//return Mesh;
//}

void UFDOverlayEditorMode::InitializeTargets()
{
	using namespace FDEditorModeLocals;

	check(LivePreviewWorld);

	// Build the tool targets that provide us with 3d dynamic meshes
	UFDOverlayEditorSubsystem* UVSubsystem = GEditor->GetEditorSubsystem<UFDOverlayEditorSubsystem>();
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
			AppliedPreviews[AssetID], AssetID, DefaultUVLayerIndex,
			GetWorld(), LivePreviewWorld, ToolSetupUtil::GetDefaultWorkingMaterial(GetToolManager()), DefaultBakeMaterialInterface, 
			FUVEditorUXSettings::UVToVertPosition, FUVEditorUXSettings::VertPositionToUV))
		{
			return;
		}

		if (Transforms.Num() == ToolTargets.Num())
		{
			ToolInputObject->AppliedPreview->PreviewMesh->SetTransform(Transforms[AssetID]);
		}

		//ToolInputObject->UnwrapPreview->PreviewMesh->SetMaterial(
		//	0, ToolSetupUtil::GetCustomTwoSidedDepthOffsetMaterial(
		//		GetToolManager(),
		//		FUVEditorUXSettings::GetTriangleColorByTargetIndex(AssetID),
		//		FUVEditorUXSettings::UnwrapTriangleDepthOffset,
		//		FUVEditorUXSettings::UnwrapTriangleOpacity));

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


void UFDOverlayEditorMode::RegisterTools() 
{
	const FFDOverlayEditorModeCommands& CommandInfos = FFDOverlayEditorModeCommands::Get();


	UFDOverlayEditorAutoCalToolBuilder* FDOverlayEditorAutoCalToolBuilder = NewObject<UFDOverlayEditorAutoCalToolBuilder>();
	FDOverlayEditorAutoCalToolBuilder->Targets = &ToolInputObjects;
	RegisterTool(CommandInfos.AutoCalTool, TEXT("AutoCalTool"), FDOverlayEditorAutoCalToolBuilder);

}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UFDOverlayEditorMode::GetModeCommands() const
{
	return FFDOverlayEditorModeCommands::Get().GetCommands();
}

#undef LOCTEXT_NAMESPACE
