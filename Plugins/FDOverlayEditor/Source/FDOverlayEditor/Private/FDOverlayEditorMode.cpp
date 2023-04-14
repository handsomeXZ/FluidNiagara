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
			// 由于 UVUnwrap 可能存在背面三角，所以需要材质开启 “双面”
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


	// 辅助函数，用于添加模式自己创建的上下文，而不是从资产编辑器中获取。
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

	// 收集目标的三维动态网格。每个资产总是有一个，每个资产的AssetID将是这些数组的索引。
	// 单独的输入对象(代表特定的UV层)将指向这些现有的3d网格。
	for (UToolTarget* Target : ToolTargets)
	{
		// applied canonical mesh 是应用了所有图层变化的3d网格。如果切换到不同的层，更改将保留在 applied canonical 中。
		TSharedPtr<FDynamicMesh3> AppliedCanonical = MakeShared<FDynamicMesh3>(UE::ToolTarget::GetDynamicMeshCopy(Target));
		AppliedCanonicalMeshes.Add(AppliedCanonical);
		
		// 创建 applied canonical 的预览版本。工具可以附加计算到这个，尽管他们必须小心，如果我们允许多个层显示一个资产，
		// 以避免试图附加两个 Computes 到同一个预览对象(在这种情况下，一个将被抛出)
		UMeshOpPreviewWithBackgroundCompute* AppliedPreview = NewObject<UMeshOpPreviewWithBackgroundCompute>();
		AppliedPreview->Setup(LivePreviewWorld);
		AppliedPreview->PreviewMesh->UpdatePreview(AppliedCanonical.Get());

		//FComponentMaterialSet MaterialSet = UE::ToolTarget::GetMaterialSet(Target);

		//AppliedPreview->ConfigureMaterials(MaterialSet.Materials,
		//	ToolSetupUtil::GetDefaultWorkingMaterial(GetToolManager()));
		AppliedPreviews.Add(AppliedPreview);
	}


	// 当创建UV unwrap时，这些函数将确定UV值和结果的unwrap网格顶点位置之间的映射。
	// 如果我们向下看打开的网格，Z轴朝向我们，我们希望U是正确的，V是向上的。在Unreal的左手坐标系中，这意味着我们将U映射到世界Y，将V映射到世界X。
	// 此外，Unreal在内部将导入网格的V坐标更改为1-V，我们在显示UV的时候撤销这一点，因为用户可能希望看到原始的UV(对于使用UDIM资产的用户来说，这将特别令人困惑，其中内部存储的V经常以负结束)。
	// ScaleFactor只是缩放网格。缩放网格可以更容易地进一步放大显示，而不是在相机接近平面距离时出现问题。


	// 构造工具实际操作的完整输入对象。
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

		// 我们并不需要线框显示模式，如果有需要后面可以参考下列注释代码。
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

		// 绑定到委托，这样我们就可以检测到变化
		//ToolInputObject->OnCanonicalModified.AddWeakLambda(this, [this]
		//(UUVEditorToolMeshInput* InputObject, const UFDOverlayMeshInput::FCanonicalModifiedInfo&) {
		//		ModifiedAssetIDs.Add(InputObject->AssetID);
		//	});

		ToolInputObjects.Add(ToolInputObject);
	}

	// 为 layer/channel selection 做准备
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
