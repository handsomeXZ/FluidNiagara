// Copyright HandsomeCheese. All Rights Reserved.
#include "Tools/FDOverlayEditorAutoCalTool.h"


#include "DynamicMesh/DynamicMesh3.h"
#include "InteractiveToolManager.h"
#include "MeshOpPreviewHelpers.h" // UMeshOpPreviewWithBackgroundCompute
#include "ContextObjectStore.h"
#include "InteractiveToolManager.h"
#include "Factories/Factory.h"
#include "RendererInterface.h"
#include "AssetToolsModule.h"
#include "Misc\PathViews.h"
#include "Factories/Texture2dFactoryNew.h"
#include "Kismet\KismetMathLibrary.h"
#include "AssetRegistry\IAssetRegistry.h"
#include "Engine\TextureRenderTarget2DArray.h"
#include "AssetRegistry\AssetRegistryModule.h"
#include "Engine\Texture2DArray.h"
#include "Framework\Notifications\NotificationManager.h"
#include "Widgets\Notifications\SNotificationList.h"

#include "Operators/FDOverlayEditorAutoCalOp.h"
#include "UObject/ConstructorHelpers.h"
#include "MeshDescriptionToDynamicMesh.h"
#include "MeshOpPreviewHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TextureRenderTarget2DArrayResource.h"

#include "FDOverlayMeshInput.h"

#define LOCTEXT_NAMESPACE "FDOverlayEditorAutoCalTool"

using namespace UE::Geometry;

namespace MeshPreviewTools
{
	TSharedPtr<FDynamicMesh3> GetSharedDynamicMesh(TObjectPtr<UStaticMesh> StaticMeshIn)
	{
		FDynamicMesh3 DynamicMesh;
		FMeshDescriptionToDynamicMesh Converter;
		Converter.Convert(StaticMeshIn->GetMeshDescription(0), DynamicMesh);
		return MakeShared<UE::Geometry::FDynamicMesh3>(DynamicMesh);
	}

	FAuxiliaryMeshPreview GetDefaultPreview(TObjectPtr<UWorld> LivePreviewWorldIn, UMaterialInstanceDynamic* MDI, EFDOverlayEditorAutoCalType type, TSharedPtr<FDynamicMesh3> DynamicMeshIn)
	{
		FAuxiliaryMeshPreview LinePreview;
		LinePreview.type = type;
		LinePreview.Preview = NewObject<UMeshOpPreviewWithBackgroundCompute>();
		LinePreview.Preview->Setup(LivePreviewWorldIn);
		LinePreview.Preview->PreviewMesh->UpdatePreview(DynamicMeshIn.Get());
		LinePreview.Preview->PreviewMesh->SetVisible(false);
		LinePreview.Preview->PreviewMesh->GetRootComponent()->SetTranslucentSortPriority(1);
		LinePreview.Preview->ConfigureMaterials(MDI, MDI);

		return LinePreview;
	}

	void AddDefaultPreview(TArray<FAuxiliaryMeshPreview>& Array, TObjectPtr<UWorld> LivePreviewWorldIn, UMaterialInstanceDynamic* MDI, EFDOverlayEditorAutoCalType type, TSharedPtr<FDynamicMesh3> DynamicMeshIn)
	{
		FAuxiliaryMeshPreview LinePreview;
		LinePreview.type = type;
		LinePreview.Preview = NewObject<UMeshOpPreviewWithBackgroundCompute>();
		LinePreview.Preview->Setup(LivePreviewWorldIn);
		LinePreview.Preview->PreviewMesh->UpdatePreview(DynamicMeshIn.Get());
		LinePreview.Preview->PreviewMesh->SetVisible(false);
		LinePreview.Preview->PreviewMesh->GetRootComponent()->SetTranslucentSortPriority(1);
		LinePreview.Preview->ConfigureMaterials(MDI, MDI);

		Array.Add(LinePreview);
	}

	int FindDefaultPreviewNum(TArray<FAuxiliaryMeshPreview>& Array, EFDOverlayEditorAutoCalType type)
	{
		int num = 0;
		for (const FAuxiliaryMeshPreview& view : Array)
		{
			if (view.type == type)
			{
				num ++;
			}
		}

		return num;
	}

	void RemoveDefaultPreview(TArray<FAuxiliaryMeshPreview>& Array, EFDOverlayEditorAutoCalType type, int NumToDelete)
	{
		for (int i = 0; i < Array.Num(); i++)
		{
			if (NumToDelete > 0 && Array[i].type == type)
			{
				Array.RemoveAt(Array.Num() - 1);
				NumToDelete--;
			}
		}
	}

	void UpdatePreview(TArray<FAuxiliaryMeshPreview>& Array, EFDOverlayEditorAutoCalType type, bool bVisible, FTransform& Transform)
	{
		for (FAuxiliaryMeshPreview& view : Array)
		{
			if (view.type == type)
			{
				view.Preview->PreviewMesh->SetVisible(bVisible);
				view.Preview->PreviewMesh->SetTransform(Transform);
			}
		}
	}

	void UpdatePreviews(TArray<FAuxiliaryMeshPreview>& Array, EFDOverlayEditorAutoCalType type, bool bVisible, TArray<FTransform>& Transform)
	{
		for (int i = 0, j = 0; i < Array.Num(); i++)
		{
			FAuxiliaryMeshPreview& view = Array[i];
			if (view.type == type)
			{
				view.Preview->PreviewMesh->SetVisible(bVisible);
				view.Preview->PreviewMesh->SetTransform(Transform[j++]);
			}
		}
	}

}
bool UFDOverlayEditorAutoCalToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	return Targets && Targets->Num() > 0;
}

UInteractiveTool* UFDOverlayEditorAutoCalToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	UFDOverlayEditorAutoCalTool* NewTool = NewObject<UFDOverlayEditorAutoCalTool>(SceneState.ToolManager);
	NewTool->Initialize(*Targets, *LivePreviewWorld);

	return NewTool;
}

UFDOverlayEditorAutoCalTool::UFDOverlayEditorAutoCalTool()
{
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultArrowAsset;
		ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultSphereAsset;
		ConstructorHelpers::FObjectFinder<UMaterial> DefaultArrowMaterialInterface;
		ConstructorHelpers::FObjectFinder<UMaterial> DefaultSphereMaterialInterface;
		FConstructorStatics()
			:DefaultArrowAsset(TEXT("StaticMesh'/FDOverlayEditor/S_Arrow.S_Arrow'")),
			DefaultSphereAsset(TEXT("StaticMesh'/FDOverlayEditor/Sphere.Sphere'")),
			DefaultArrowMaterialInterface(TEXT("Material'/FDOverlayEditor/M_Arrow.M_Arrow'")),
			DefaultSphereMaterialInterface(TEXT("Material'/FDOverlayEditor/M_Sphere.M_Sphere'")){}

	};
	static FConstructorStatics ConstructorStatics;

	ArrowMesh_Static = ConstructorStatics.DefaultArrowAsset.Object;
	SphereMesh_Static = ConstructorStatics.DefaultSphereAsset.Object;
	DefaultArrow_MI = ConstructorStatics.DefaultArrowMaterialInterface.Object;
	DefaultSphere_MI = ConstructorStatics.DefaultSphereMaterialInterface.Object;

	FName UniqueDynamicName = MakeUniqueObjectName(this, UMaterialInstanceDynamic::StaticClass(), FName(TEXT("DMI_Arrow")));
	DefaultArrow_DMI = UMaterialInstanceDynamic::Create(DefaultArrow_MI, this, UniqueDynamicName);
	UniqueDynamicName = MakeUniqueObjectName(this, UMaterialInstanceDynamic::StaticClass(), FName(TEXT("DMI_Sphere")));
	DefaultSphere_DMI = UMaterialInstanceDynamic::Create(DefaultSphere_MI, this, UniqueDynamicName);

	if (ArrowMesh_Static != nullptr && DefaultArrow_DMI != nullptr)
	{
		ArrowMesh_Dynamic = MeshPreviewTools::GetSharedDynamicMesh(ArrowMesh_Static);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't find StaticMesh'/FDOverlayEditor/S_Arrow.S_Arrow'"));
		UE_LOG(LogTemp, Warning, TEXT("Can't find Material'/FDOverlayEditor/M_Arrow.M_Arrow'"));
	}

	if (SphereMesh_Static != nullptr && DefaultSphere_DMI != nullptr)
	{
		SphereMesh_Dynamic = MeshPreviewTools::GetSharedDynamicMesh(SphereMesh_Static);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't find StaticMesh'/FDOverlayEditor/Sphere.Sphere'"));
		UE_LOG(LogTemp, Warning, TEXT("Can't find Material'/FDOverlayEditor/M_Sphere.M_Sphere'"));
	}
}


void UFDOverlayEditorAutoCalTool::Initialize(const TArray<TObjectPtr<UFDOverlayMeshInput>>& TargetsIn, TObjectPtr<UWorld> LivePreviewWorldIn)
{
	Targets = TargetsIn;
	LivePreviewWorld = LivePreviewWorldIn;
}


void UFDOverlayEditorAutoCalTool::Setup()
{
	check(Targets.Num() > 0);

	UInteractiveTool::Setup();

	Settings = NewObject<UFDOverlayEditorAutoCalProperties>(this);
	Settings->RestoreProperties(this);
	AddToolPropertySource(Settings);

	UContextObjectStore* ContextStore = GetToolManager()->GetContextObjectStore();

	UFDOverlayAutoCalToolAPI* AutoCalToolAPI = ContextStore->FindContext<UFDOverlayAutoCalToolAPI>();
	AutoCalToolAPI->OnOutputTypeChange.AddUObject(this, &ThisClass::OnChangeOutputType);
	UFDOverlayLive3DPreviewAPI* Live3DPreviewAPI = ContextStore->FindContext<UFDOverlayLive3DPreviewAPI>();
	Live3DPreviewAPI->OnApplyChangesDelegate.AddUObject(this, &ThisClass::ExecuteBakePass);

	for (TObjectPtr<UFDOverlayMeshInput> Target : Targets)
	{
		Target->AppliedPreview->OnMeshUpdated.AddWeakLambda(this, [this, &Target](UMeshOpPreviewWithBackgroundCompute* Preview)
			{
				//Target->UpdateUnwrapPreviewFromAppliedPreview();
			});
	}
	

	MeshPreviewTools::AddDefaultPreview(AuxiliaryMeshPreviews, LivePreviewWorld, DefaultArrow_DMI, EFDOverlayEditorAutoCalType::Line, ArrowMesh_Dynamic);
	MeshPreviewTools::AddDefaultPreview(AuxiliaryMeshPreviews, LivePreviewWorld, DefaultSphere_DMI, EFDOverlayEditorAutoCalType::Point, SphereMesh_Dynamic);

	InitializeCurve();
	InitializeMeshResource();
	UpdatedAuxiliaryMeshPreview();
}

void UFDOverlayEditorAutoCalTool::Shutdown(EToolShutdownType ShutdownType)
{
	Settings->SaveProperties(this);

	for (TObjectPtr<UFDOverlayMeshInput> Target : Targets)
	{
		Target->AppliedPreview->OnMeshUpdated.RemoveAll(this);
	}

	if (ShutdownType == EToolShutdownType::Accept)
	{
		
	}
	else
	{
		// Reset the inputs
		for (TObjectPtr<UFDOverlayMeshInput> Target : Targets)
		{
			//Target->UpdatePreviewsFromCanonical();
		}
	}

	for (FAuxiliaryMeshPreview& view : AuxiliaryMeshPreviews)
	{
		view.Preview->Shutdown();
	}

	Settings = nullptr;
	Targets.Empty();
	CurveKeys.Empty();
	BakeBuffer.Empty();
	AuxiliaryMeshPreviews.Empty();

	ArrowMesh_Static = nullptr;
	ArrowMesh_Dynamic = nullptr;
	DefaultArrow_MI = nullptr;
	DefaultArrow_DMI = nullptr;
	LivePreviewWorld = nullptr;
}


void UFDOverlayEditorAutoCalTool::OnTick(float DeltaTime)
{
	for (TObjectPtr<UFDOverlayMeshInput> Target : Targets)
	{
		Target->AppliedPreview->Tick(DeltaTime);
	}
}

void UFDOverlayEditorAutoCalTool::InitializeCurve()
{

	if (Settings->LayoutType == EFDOverlayEditorAutoCalType::Line || Settings->LayoutType == EFDOverlayEditorAutoCalType::Point)
	{
		float RangeMin = 0, RangMax = 0;
		if (Settings->UVCurve)
		{
			CurveKeys.Empty(Settings->UVCurve->FloatCurve.GetNumKeys());
			for (const FRichCurveKey& key : Settings->UVCurve->FloatCurve.GetConstRefOfKeys())
			{
				CurveKeys.Emplace(FCurveKey(key.Time, key.Value, key.ArriveTangent, key.LeaveTangent));
			}

			Settings->UVCurve->FloatCurve.GetValueRange(RangeMin, RangMax);
		}
		else
		{
			CurveKeys.Empty(1);
			CurveKeys.Emplace(FCurveKey(0, 0, 0, 0));
		}

		CurveRange = RangMax - RangeMin;
	}
	else if(Settings->LayoutType == EFDOverlayEditorAutoCalType::MultiLine)
	{
		if (Settings->MultiLineData.Num() == 0)
		{
			MultiLineCurveKeys.Empty(1);
			MultiLineCurveRange.Empty(1);
			MultiLineCurveKeysNum.Empty(1);
			MultiLineCurveKeys.Add(FCurveKey(0, 0, 0, 0));
			MultiLineCurveRange.Emplace(float(0));
			MultiLineCurveKeysNum.Add(1);
			return;
		}
		MultiLineCurveKeysNum.Empty();
		MultiLineCurveKeys.Empty();
		MultiLineCurveRange.Empty(Settings->MultiLineData.Num());
		for (const FMultiLineData& data : Settings->MultiLineData)
		{
			float RangeMin = 0, RangMax = 0;
			if (data.UVCurve)
			{
				for (const FRichCurveKey& key : data.UVCurve->FloatCurve.GetConstRefOfKeys())
				{
					MultiLineCurveKeys.Emplace(FCurveKey(key.Time, key.Value, key.ArriveTangent, key.LeaveTangent));
				}
				data.UVCurve->FloatCurve.GetValueRange(RangeMin, RangMax);
				MultiLineCurveKeysNum.Add(data.UVCurve->FloatCurve.GetNumKeys());
			}
			else
			{
				MultiLineCurveKeys.Emplace(FCurveKey(0, 0, 0, 0));
				MultiLineCurveKeysNum.Add(1);
			}
			
			MultiLineCurveRange.Emplace(float(RangMax - RangeMin));
		}
	}
	else if (Settings->LayoutType == EFDOverlayEditorAutoCalType::MultiPoint)
	{
		if (Settings->MultiPointData.Num() == 0)
		{
			MultiPointCurveKeys.Empty(1);
			MultiPointCurveRange.Empty(1);
			MultiPointCurveKeysNum.Empty(1);
			MultiPointCurveKeys.Emplace(FCurveKey(0, 0, 0, 0));
			MultiPointCurveRange.Emplace(float(0));
			MultiPointCurveKeysNum.Add(1);
			return;
		}
		MultiPointCurveKeysNum.Empty();
		MultiPointCurveKeys.Empty();
		MultiPointCurveRange.Empty(Settings->MultiPointData.Num());
		for (const FMultiPointData& data : Settings->MultiPointData)
		{
			float RangeMin = 0, RangMax = 0;
			if (data.UVCurve)
			{
				for (const FRichCurveKey& key : data.UVCurve->FloatCurve.GetConstRefOfKeys())
				{
					MultiPointCurveKeys.Emplace(FCurveKey(key.Time, key.Value, key.ArriveTangent, key.LeaveTangent));
				}
				data.UVCurve->FloatCurve.GetValueRange(RangeMin, RangMax);
				MultiPointCurveKeysNum.Add(data.UVCurve->FloatCurve.GetNumKeys());
			}
			else
			{
				MultiPointCurveKeys.Emplace(FCurveKey(0, 0, 0, 0));
				MultiPointCurveKeysNum.Add(1);
			}
			MultiPointCurveRange.Emplace(float(RangMax - RangeMin));
		}
	}
}


void UFDOverlayEditorAutoCalTool::InitializeBakeParams(FExtraParams& ExtraParams, int TargetID, FVector3f Origin, FVector3f Direction, const TArray<FCurveKey>& CurveKeysIn)
{
	ExtraParams.bSuturedUV = Settings->bSuturedUV;
	ExtraParams.BakeBuffer = BakeBuffer[TargetID];
	ExtraParams.MIDNum = Targets[TargetID]->MaxMaterialIndex + 1;
	ExtraParams.TargetID = TargetID;
	ExtraParams.Size = FIntPoint(Settings->XYSize, Settings->XYSize);
	ExtraParams.Params.CurveRange = CurveRange;
	ExtraParams.CurveKeys = CurveKeysIn;
	ExtraParams.Params.KeyNum = CurveKeys.Num();
	ExtraParams.Params.GradientDir = Direction;
	ExtraParams.Params.GradientOrigin = Origin;
	ExtraParams.Params.UVCurveOrigin = FVector3f(GetUVOffsetOrigin(Settings->UVOffset, FVector(Origin), FVector(Direction)));

	double GradientMax = 0;
	for (FUVVertex& Vert : Vertices[TargetID])
	{
		// 计算 GradientMax
		if (Settings->LayoutType == EFDOverlayEditorAutoCalType::Line)
		{
			float d = GetDistanceAlongLine(FVector(Vert.Position), FVector(ExtraParams.Params.GradientOrigin), FVector(ExtraParams.Params.GradientDir));
			if (GradientMax < d)
			{
				GradientMax = d;
			}
		}
		else if (Settings->LayoutType == EFDOverlayEditorAutoCalType::Point)
		{
			float d = Length(FVector(Vert.Position) - FVector(ExtraParams.Params.GradientOrigin));
			if (GradientMax < d)
			{
				GradientMax = d;
			}
		}
	}
	
	ExtraParams.Params.GradientMax = GradientMax;
	ExtraParams.Params.TriangleNum = Triangles[TargetID].Num();
	ExtraParams.Params.VertexNum = Vertices[TargetID].Num();

}

template<typename T>
void UFDOverlayEditorAutoCalTool::InitializeBakeParams(FMultiExtraParams& ExtraParams, int TargetID, const TArray<T>& DataList, const TArray<float>& MultiCurveRange, const TArray<FCurveKey>& MultiCurveKeys, const TArray<int>& MultiCurveKeysNum)
{
	ExtraParams.bSuturedUV = Settings->bSuturedUV;
	ExtraParams.BakeBuffer = BakeBuffer[TargetID];
	ExtraParams.MIDNum = Targets[TargetID]->MaxMaterialIndex + 1;
	ExtraParams.TargetID = TargetID;
	ExtraParams.Size = FIntPoint(Settings->XYSize, Settings->XYSize);
	ExtraParams.CurveKeys = MultiCurveKeys;

	int i = 0;
	for (const T& data : DataList)
	{
		double GradientMax = 0;
		for (FUVVertex& Vert : Vertices[TargetID])
		{
			// 计算 GradientMax
			if (Settings->LayoutType == EFDOverlayEditorAutoCalType::MultiLine)
			{
				float d = GetDistanceAlongLine(FVector(Vert.Position), FVector(data.LineOrigin), FVector(data.LineDirection));
				if (GradientMax < d)
				{
					GradientMax = d;
				}
			}
			else if (Settings->LayoutType == EFDOverlayEditorAutoCalType::MultiPoint)
			{
				float d = Length(FVector(Vert.Position) - FVector(data.LineOrigin));
				if (GradientMax < d)
				{
					GradientMax = d;
				}
			}
		}
		ExtraParams.ParamsArr.Emplace(FMultiParamsArr(data.LineOrigin, data.LineDirection, GradientMax, 
		FVector3f(GetUVOffsetOrigin(data.UVOffset, FVector(data.LineOrigin), FVector(data.LineDirection))), 
			MultiCurveRange[i], MultiCurveKeysNum[i]));
		i++;
	}

	ExtraParams.Params.ParamsArrNum = i;
	ExtraParams.Params.TriangleNum = Triangles[TargetID].Num();
	ExtraParams.Params.VertexNum = Vertices[TargetID].Num();
	ExtraParams.Params.VRICFlag = Settings->VRICType == EVRICType::PLD ? 1:0;
	ExtraParams.Params.VRIC = Settings->VRIC;
	
}

void UFDOverlayEditorAutoCalTool::InitializeMeshResource()
{
	if (Vertices.Num() == Targets.Num() && Triangles.Num() == Targets.Num())
	{
		return;
	}
	else
	{
		check(Vertices.Num() == Triangles.Num());
	}

	for (TObjectPtr<UFDOverlayMeshInput> Target : Targets)
	{
		TArray<FUVVertex> VerticesIn;
		TArray<FTriangle> TrianglesIn;

		TSharedPtr<UE::Geometry::FDynamicMesh3> AppliedCanonical = Target->AppliedCanonical;
		FDynamicMeshUVOverlay* UVOverlay = AppliedCanonical->Attributes()->GetUVLayer(0);

		int VerticesNum = UVOverlay->ElementCount();
		int TrianglesNum = AppliedCanonical->TriangleCount();
		UE_LOG(LogTemp, Warning, TEXT("JTJ TriangleNum: %d"), TrianglesNum);
		VerticesIn.Empty(VerticesNum);
		for (int32 ElementID = 0; ElementID < VerticesNum; ElementID++)
		{
			FVector2f UVElement = UVOverlay->GetElement(ElementID);
			int32 vid = UVOverlay->GetParentVertex(ElementID);
			VerticesIn.Emplace(FUVVertex(FVector3f(AppliedCanonical->GetVertex(vid)), AppliedCanonical->GetVertexNormal(vid), FVector2f(0, 0)));
		}

		TrianglesIn.Empty(TrianglesNum);
		for (int32 i : AppliedCanonical->TriangleIndicesItr())
		{
			if (UVOverlay->IsSetTriangle(i))
			{
				//FIndex3i tri = AppliedCanonical->GetTriangle(i);
				FIndex3i UVTri = UVOverlay->GetTriangle(i);
				TrianglesIn.Emplace(FTriangle(UVTri, AppliedCanonical->Attributes()->GetMaterialID()->GetValue(i)));
				//FIndex3i index3i = UVOverlay->GetTriangle(i);
				for (int j = 0; j < 3; j++)
				{
					FVector2f uv = UVOverlay->GetElement(UVTri.ABC[j]);
					VerticesIn[/*UVOverlay->GetParentVertex(UVTri.ABC[j])*/UVTri.ABC[j]].UV = uv;
				}
			}
		}

		Vertices.Add(MoveTemp(VerticesIn));
		Triangles.Add(MoveTemp(TrianglesIn));
	}



}

void UFDOverlayEditorAutoCalTool::InitializeBakePass(int32 MIDNum, int TargetID)
{
	if (BakeBuffer.Num() >= TargetID + 1)
	{
		BakeBuffer[TargetID]->Init(Settings->XYSize, Settings->XYSize, MIDNum, PF_FloatRGBA);
		return;
	}
	BakeBuffer.Reset(MIDNum);
	//BakeRenderTarget = NewObject<UTextureRenderTarget2D>();
	//BakeRenderTarget->AddToRoot();
	//BakeRenderTarget->ClearColor = FLinearColor::Black;
	//BakeRenderTarget->TargetGamma = 1.0f;
	//BakeRenderTarget->InitCustomFormat(Settings->XYSize, Settings->XYSize, PF_FloatRGBA, false);

	UTextureRenderTarget2DArray* RTbuffer = NewObject<UTextureRenderTarget2DArray>();
	RTbuffer->AddToRoot();
	RTbuffer->ClearColor = FLinearColor::Black;
	RTbuffer->TargetGamma = 1.0f;
	check((Settings->XYSize > 0) && (Settings->XYSize > 0) && (MIDNum > 0));
	RTbuffer->Init(Settings->XYSize, Settings->XYSize, MIDNum, PF_FloatRGBA);
	BakeBuffer.Add(RTbuffer);

}

template<typename T>
void UFDOverlayEditorAutoCalTool::AddBakePass(T ExtraParams, int TargetID, EFDOverlayEditorAutoCalType Type)
{

	//FString AssetPath = GetAssetPath(Settings->AssetPathFormat, Settings->Name, MID);
	//ExtraParams.OutputTexture = FindOrCreate(AssetPath);


	//FFDAutoCalCSInterface::Dispatch(MoveTemp(AppliedVertices), MoveTemp(Triangles), ExtraParams, [this](FExtraParams ExtraParams) {
	//	UpdateOutputTexture(ExtraParams);
	//	this->OnFinishCS.ExecuteIfBound(ExtraParams);
	//	});
	switch (Type)
	{
	case EFDOverlayEditorAutoCalType::Line:
		FFDAutoCalCSInterface::Dispatch<T, EFDAutoCalCSType::LineCS>(Vertices[TargetID], Triangles[TargetID], ExtraParams, [this](T ExtraParams) {
			UpdateOutputTexture(ExtraParams.BakeBuffer, ExtraParams.TargetID);
			this->OnFinishCS.ExecuteIfBound();
			});
			
		break;
	case EFDOverlayEditorAutoCalType::Point:
		FFDAutoCalCSInterface::Dispatch<T, EFDAutoCalCSType::PointCS>(Vertices[TargetID], Triangles[TargetID], ExtraParams, [this](T ExtraParams) {
			UpdateOutputTexture(ExtraParams.BakeBuffer, ExtraParams.TargetID);
			this->OnFinishCS.ExecuteIfBound();
			});

		break;
	case EFDOverlayEditorAutoCalType::MultiLine:
		FFDAutoCalCSInterface::Dispatch<T, EFDAutoCalCSType::MultiLineCS>(Vertices[TargetID], Triangles[TargetID], ExtraParams, [this](T ExtraParams) {
			UpdateOutputTexture(ExtraParams.BakeBuffer, ExtraParams.TargetID);
			this->OnFinishCS.ExecuteIfBound();
			});
		break;
	case EFDOverlayEditorAutoCalType::MultiPoint:
		FFDAutoCalCSInterface::Dispatch<T, EFDAutoCalCSType::MultiPointCS>(Vertices[TargetID], Triangles[TargetID], ExtraParams, [this](T ExtraParams) {
			UpdateOutputTexture(ExtraParams.BakeBuffer, ExtraParams.TargetID);
			this->OnFinishCS.ExecuteIfBound();
			});
		break;
	}

}

void UFDOverlayEditorAutoCalTool::OnPropertyModified(UObject* PropertySet, FProperty* Property)
{	
	InitializeCurve();
	
	for (TObjectPtr<UFDOverlayMeshInput> Target : Targets)
	{
		Target->AppliedPreview->InvalidateResult();
	}
	FSlateNotificationManager::Get().AddNotification(FNotificationInfo(LOCTEXT("FDOverlayEditorAutoCalTool", "Update Finished")))->SetCompletionState(SNotificationItem::CS_None);

	// Updated the preview of the auxiliary Mesh
	UpdatedAuxiliaryMeshPreview();

}

void UFDOverlayEditorAutoCalTool::ExecuteBakePass()
{
	switch (Settings->LayoutType)
	{
	case EFDOverlayEditorAutoCalType::Line:
		for (int TargetID = 0; TargetID < Targets.Num(); TargetID++)
		{
			TObjectPtr<UFDOverlayMeshInput> Target = Targets[TargetID];
			InitializeBakePass(Target->MaxMaterialIndex + 1, TargetID);
			FExtraParams ExtraParams;
			InitializeBakeParams(ExtraParams, TargetID, Settings->LineOrigin, Settings->LineDirection, CurveKeys);
			AddBakePass<FExtraParams>(ExtraParams, TargetID, EFDOverlayEditorAutoCalType::Line);
		}
		break;
	case EFDOverlayEditorAutoCalType::Point:
		for (int TargetID = 0; TargetID < Targets.Num(); TargetID++)
		{
			TObjectPtr<UFDOverlayMeshInput> Target = Targets[TargetID];
			InitializeBakePass(Target->MaxMaterialIndex + 1, TargetID);
			FExtraParams ExtraParams;
			InitializeBakeParams(ExtraParams, TargetID, Settings->PointOrigin, Settings->UVDirection, CurveKeys);
			AddBakePass<FExtraParams>(ExtraParams, TargetID, EFDOverlayEditorAutoCalType::Point);
		}
		break;
	case EFDOverlayEditorAutoCalType::MultiLine:
		if (Settings->MultiLineData.Num() == 0)
		{
			break;
		}
		for (int TargetID = 0; TargetID < Targets.Num(); TargetID++)
		{
			TObjectPtr<UFDOverlayMeshInput> Target = Targets[TargetID];
			InitializeBakePass(Target->MaxMaterialIndex + 1, TargetID);
			FMultiExtraParams ExtraParams;
			InitializeBakeParams<FMultiLineData>(ExtraParams, TargetID, Settings->MultiLineData, MultiLineCurveRange, MultiLineCurveKeys, MultiLineCurveKeysNum);
			AddBakePass<FMultiExtraParams>(ExtraParams, TargetID, EFDOverlayEditorAutoCalType::MultiLine);
		}
		break;
	case EFDOverlayEditorAutoCalType::MultiPoint:
		if (Settings->MultiPointData.Num() == 0)
		{
			break;
		}
		for (int TargetID = 0; TargetID < Targets.Num(); TargetID++)
		{
			TObjectPtr<UFDOverlayMeshInput> Target = Targets[TargetID];
			InitializeBakePass(Target->MaxMaterialIndex + 1, TargetID);
			FMultiExtraParams ExtraParams;
			InitializeBakeParams<FMultiPointData>(ExtraParams, TargetID, Settings->MultiPointData, MultiPointCurveRange, MultiPointCurveKeys, MultiPointCurveKeysNum);
			AddBakePass<FMultiExtraParams>(ExtraParams, TargetID, EFDOverlayEditorAutoCalType::MultiPoint);
		}
		break;
	}
}

void UFDOverlayEditorAutoCalTool::UpdatedAuxiliaryMeshPreview()
{
	
	// Line
	if (Settings->LayoutType != EFDOverlayEditorAutoCalType::Line)
	{
		FTransform Transform(FVector(Settings->LineDirection).ToOrientationQuat(), FVector(Settings->LineOrigin), FVector(1, 1, 1));
		MeshPreviewTools::UpdatePreview(AuxiliaryMeshPreviews, EFDOverlayEditorAutoCalType::Line, false, Transform);
	}
	else
	{
		FTransform Transform(FVector(Settings->LineDirection).ToOrientationQuat(), FVector(Settings->LineOrigin), FVector(1, 1, 1));
		MeshPreviewTools::UpdatePreview(AuxiliaryMeshPreviews, EFDOverlayEditorAutoCalType::Line, true, Transform);
	}

	// Point
	if (Settings->LayoutType != EFDOverlayEditorAutoCalType::Point)
	{
		FTransform Transform(FVector(Settings->UVDirection).ToOrientationQuat(), FVector(Settings->PointOrigin), FVector(0.03, 0.03, 0.03));
		MeshPreviewTools::UpdatePreview(AuxiliaryMeshPreviews, EFDOverlayEditorAutoCalType::Point, false, Transform);
	}
	else
	{
		FTransform Transform(FVector(Settings->UVDirection).ToOrientationQuat(), FVector(Settings->PointOrigin), FVector(0.03, 0.03, 0.03));
		MeshPreviewTools::UpdatePreview(AuxiliaryMeshPreviews, EFDOverlayEditorAutoCalType::Point, true, Transform);
	}

	// MultiLine
	if (Settings->LayoutType != EFDOverlayEditorAutoCalType::MultiLine)
	{
		TArray<FTransform> Transforms;
		for (FMultiLineData& data : Settings->MultiLineData)
		{
			Transforms.Add(FTransform(FVector(data.LineDirection).ToOrientationQuat(), FVector(data.LineOrigin), FVector(1, 1, 1)));
		}
		MeshPreviewTools::UpdatePreviews(AuxiliaryMeshPreviews, EFDOverlayEditorAutoCalType::MultiLine, false, Transforms);
	}
	else
	{
		int PrevNum = MeshPreviewTools::FindDefaultPreviewNum(AuxiliaryMeshPreviews, EFDOverlayEditorAutoCalType::MultiLine);
		UE_LOG(LogTemp, Warning, TEXT("PrevNum : %d"), PrevNum);
		MeshPreviewTools::RemoveDefaultPreview(AuxiliaryMeshPreviews, EFDOverlayEditorAutoCalType::MultiLine, PrevNum - Settings->MultiLineData.Num());


		TArray<FTransform> Transforms;
		for (FMultiLineData& data : Settings->MultiLineData)
		{
			Transforms.Add(FTransform(FVector(data.LineDirection).ToOrientationQuat(), FVector(data.LineOrigin), FVector(1, 1, 1)));
			if (PrevNum-- <= 0)
			{
				MeshPreviewTools::AddDefaultPreview(AuxiliaryMeshPreviews, LivePreviewWorld, DefaultArrow_DMI, EFDOverlayEditorAutoCalType::MultiLine, ArrowMesh_Dynamic);
			}
		}
		MeshPreviewTools::UpdatePreviews(AuxiliaryMeshPreviews, EFDOverlayEditorAutoCalType::MultiLine, true, Transforms);
	}

	// MultiPoint
	if (Settings->LayoutType != EFDOverlayEditorAutoCalType::MultiPoint)
	{
		TArray<FTransform> Transforms;
		for (FMultiLineData& data : Settings->MultiLineData)
		{
			Transforms.Add(FTransform(FVector(data.LineDirection).ToOrientationQuat(), FVector(data.LineOrigin), FVector(0.03, 0.03, 0.03)));
		}
		MeshPreviewTools::UpdatePreviews(AuxiliaryMeshPreviews, EFDOverlayEditorAutoCalType::MultiPoint, false, Transforms);
	}
	else
	{
		int PrevNum = MeshPreviewTools::FindDefaultPreviewNum(AuxiliaryMeshPreviews, EFDOverlayEditorAutoCalType::MultiPoint);
		UE_LOG(LogTemp, Warning, TEXT("PrevNum : %d"), PrevNum);
		MeshPreviewTools::RemoveDefaultPreview(AuxiliaryMeshPreviews, EFDOverlayEditorAutoCalType::MultiPoint, PrevNum - Settings->MultiLineData.Num());


		TArray<FTransform> Transforms;
		for (FMultiLineData& data : Settings->MultiLineData)
		{
			Transforms.Add(FTransform(FVector(data.LineDirection).ToOrientationQuat(), FVector(data.LineOrigin), FVector(0.03, 0.03, 0.03)));
			if (PrevNum-- <= 0)
			{
				MeshPreviewTools::AddDefaultPreview(AuxiliaryMeshPreviews, LivePreviewWorld, DefaultSphere_DMI, EFDOverlayEditorAutoCalType::MultiPoint, SphereMesh_Dynamic);
			}
		}
		MeshPreviewTools::UpdatePreviews(AuxiliaryMeshPreviews, EFDOverlayEditorAutoCalType::MultiPoint, true, Transforms);
	}
}

bool UFDOverlayEditorAutoCalTool::CanAccept() const
{
	for (TObjectPtr<UFDOverlayMeshInput> Target : Targets)
	{
		if (!Target->AppliedPreview->HaveValidResult())
		{
			return false;
		}
	}
	return true;
}

void UFDOverlayEditorAutoCalTool::UpdateOutputTexture(UTextureRenderTarget2DArray* BakeBufferIn, int TargetID)
{
	UTexture2DArray* NewObj = nullptr;
	
	/*UTexture2D* Result = NewObject<UTexture2D>(Package, *FString(TEXT("OutputTex")), RF_Public | RF_Standalone | RF_Transactional);*/
	if (OutputType == EAutoCalToolOutputType::Texture2DArray)
	{
		FString AssetPath = GetAssetPath(Settings->AssetPathFormat, Settings->Name, TargetID);

		UPackage* Package = CreatePackage(*AssetPath);
		NewObj = BakeBufferIn->ConstructTexture2DArray(Package, FString::Printf(TEXT("Texture2DArray_%s"), *FString(FPathViews::GetCleanFilename(AssetPath))), RF_Public | RF_Standalone | RF_Transactional);
		NewObj->bSourceGeneratedFromSourceTexturesArray = false;
		// Mark the package dirty...
		Package->MarkPackageDirty();
	}
	else if (OutputType == EAutoCalToolOutputType::Texture2Ds)
	{
		FString AssetPath = GetAssetPath(Settings->AssetPathFormat, Settings->Name, TargetID);

		UPackage* Package = CreatePackage(*AssetPath);
		NewObj = NewObject<UTexture2DArray>(Package, FName(FString::Printf(TEXT("Texture2DArray_%s"), *FString(FPathViews::GetCleanFilename(AssetPath)))), RF_Public | RF_Standalone | RF_Transactional);
		NewObj->bSourceGeneratedFromSourceTexturesArray = true;
		NewObj->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
		for (int i = 0; i < BakeBufferIn->Slices; i++)
		{
			AssetPath = GetAssetPath(Settings->AssetPathFormat, Settings->Name, i);
			Package = CreatePackage(*AssetPath);
			UTexture2D* Tex = ConstructTexture2D(BakeBufferIn, Package, FString::Printf(TEXT("Texture2D_%s"), *FString(FPathViews::GetCleanFilename(AssetPath))), RF_Public | RF_Standalone | RF_Transactional, i);
			NewObj->SourceTextures.Add(Tex);

			// Notify the asset registry
			FAssetRegistryModule::AssetCreated((UObject*)Tex);
			
			// Mark the package dirty...
			Tex->MarkPackageDirty();
			Package->MarkPackageDirty();
		}
	}

	
	if (NewObj)
	{
		NewObj->UpdateSourceFromSourceTextures(true);
		NewObj->PostEditChange();
		NewObj->PostLoad();
		NewObj->UpdateResource();
		// Notify the asset registry
		FAssetRegistryModule::AssetCreated((UObject*)NewObj);
		// Mark the Obj dirty...
		NewObj->MarkPackageDirty();
	}
	//ExtraParams.OutputTexture->UpdateResource();
	//ExtraParams.OutputTexture->PostEditChange();
	//ExtraParams.OutputTexture->MarkPackageDirty();
	

	Targets[TargetID]->ShowToMesh(NewObj);
	FSlateNotificationManager::Get().AddNotification(FNotificationInfo(LOCTEXT("FDOverlayEditorAutoCalTool", "Render Finished")))->SetCompletionState(SNotificationItem::CS_Success);
	UE_LOG(LogTemp, Warning, TEXT("Finish This CS And CallBack"));
}

UTexture2D* UFDOverlayEditorAutoCalTool::ConstructTexture2D(UTextureRenderTarget2DArray* BakeBufferIn, UObject* ObjOuter, const FString& NewTexName, EObjectFlags InFlags, int32 SliceId)
{
#if WITH_EDITOR
	if (BakeBufferIn->SizeX == 0 || BakeBufferIn->SizeY == 0 || BakeBufferIn->Slices == 0 || SliceId < 0)
	{
		return nullptr;
	}

	const EPixelFormat PixelFormat = BakeBufferIn->GetFormat();
	ETextureSourceFormat TextureFormat = TSF_Invalid;
	switch (PixelFormat)
	{
	case PF_FloatRGBA:
		TextureFormat = TSF_RGBA16F;
		break;
	}

	if (TextureFormat == TSF_Invalid)
	{
		return nullptr;
	}

	FTextureRenderTarget2DArrayResource* TextureResource = (FTextureRenderTarget2DArrayResource*)BakeBufferIn->GameThread_GetRenderTargetResource();
	if (TextureResource == nullptr)
	{
		return nullptr;
	}

	// Create texture
	UTexture2D* Texture2D = NewObject<UTexture2D>(ObjOuter, FName(*NewTexName), InFlags);
	Texture2D->Source.Init(BakeBufferIn->SizeX, BakeBufferIn->SizeY, 1, 1, TextureFormat);

	bool bSRGB = true;
	// if render target gamma used was 1.0 then disable SRGB for the static texture
	if (FMath::Abs(TextureResource->GetDisplayGamma() - 1.0f) < UE_KINDA_SMALL_NUMBER)
	{
		bSRGB = false;
	}
	
	
	const int32 SrcMipSize = CalculateImageBytes(BakeBufferIn->SizeX, BakeBufferIn->SizeY, 1, PixelFormat);
	const int32 DstMipSize = CalculateImageBytes(BakeBufferIn->SizeX, BakeBufferIn->SizeY, 1, PF_FloatRGBA);
	uint8* SliceData = Texture2D->Source.LockMip(0);
	switch (TextureFormat)
	{
	case TSF_RGBA16F:
	{
		TArray<FFloat16Color> OutputBuffer;
		FReadSurfaceDataFlags ReadSurfaceDataFlags(RCM_UNorm);
		if (TextureResource->ReadPixels(OutputBuffer, SliceId))
		{
			FMemory::Memcpy((FFloat16Color*)(SliceData/* + SliceId * DstMipSize*/), OutputBuffer.GetData(), DstMipSize);
		}
		break;
	}

	default:
		// Missing conversion from PF -> TSF
		check(false);
		break;
	}
	Texture2D->Source.UnlockMip(0);
	Texture2D->SRGB = bSRGB;
	// If HDR source image then choose HDR compression settings..
	Texture2D->CompressionSettings = TextureFormat == TSF_RGBA16F ? TextureCompressionSettings::TC_HDR : TextureCompressionSettings::TC_Default; //-V547 - future proofing
	// Default to no mip generation for cube render target captures.
	Texture2D->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	Texture2D->PostEditChange();

	return Texture2D;
#else
	return nullptr;
#endif // #if WITH_EDITOR
}

FString UFDOverlayEditorAutoCalTool::GetAssetPath(FString PathFormat, FString Name, int32 MaterialID) const
{

	const TMap<FString, FStringFormatArg> PathFormatArgs =
	{
		{TEXT("AssetFolder"),	TEXT("/Game")},
		{TEXT("AssetName"),		TEXT("FDOverlay")},
		{TEXT("OutputName"),	Name},
		{TEXT("MIDNum"),	FString::Printf(TEXT("%03d"), MaterialID)},
	};
	FString AssetPath = FString::Format(*PathFormat, PathFormatArgs);
	AssetPath.ReplaceInline(TEXT("//"), TEXT("/"));
	return AssetPath;
}

UTexture2D* UFDOverlayEditorAutoCalTool::FindOrCreate(const FString& AssetPath)
{

	// Find existing
	IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
	TArray<FAssetData> FoundAssets;
	if (AssetRegistry->GetAssetsByPackageName(FName(AssetPath), FoundAssets))
	{
		if (FoundAssets.Num() > 0)
		{
			if (UTexture2D* ExistingOject = CastChecked<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *AssetPath)))
			{
				return ExistingOject;
			}

			// If the above failed then the asset is not the right type, warn the user
			FText ErrorMessage = FText::Format(
				NSLOCTEXT("NiagarBaker", "GetOrCreateAsset_PackageExistsOfWrongType", "Could not bake asset '{0}' as package exists but is not a {1}.\nPlease delete the asset or the output to a different location."),
				FText::FromString(AssetPath),
				FText::FromName(UTexture2D::StaticClass()->GetFName())
			);
			FMessageDialog::Open(EAppMsgType::Ok, ErrorMessage);
			return nullptr;
		}
	}

	// Create new Asset
	IAssetTools& AssetTools = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UFactory* NewFactory = NewObject<UFactory>(GetTransientPackage(), UTexture2DFactoryNew::StaticClass());
	UObject* ResultObject = AssetTools.CreateAsset(FString(FPathViews::GetCleanFilename(AssetPath)), FString(FPathViews::GetPath(AssetPath)), UTexture2D::StaticClass(), NewFactory);
	UTexture2D* Result = ResultObject ? CastChecked<UTexture2D>(ResultObject) : nullptr;
	
	//UPackage* Package = CreatePackage(*AssetPath);
	//UTexture2D* Result = NewObject<UTexture2D>(Package, *FString(TEXT("OutputTex")), RF_Public | RF_Standalone | RF_Transactional);

	if (Result)
	{
		FTextureFormatSettings FormatSettings;
		FormatSettings.CompressionNone = true;
		FormatSettings.CompressionSettings = TC_HDR;
		FormatSettings.SRGB = false;
		
		//TArray<FFloat16Color> NewDataFloat16Color;
		//NewDataFloat16Color.SetNumUninitialized(Settings->XYSize * Settings->XYSize, true);

		//Result->Source.Init(Settings->XYSize, Settings->XYSize, 1, 1, ETextureSourceFormat::TSF_RGBA16F, (uint8*)NewDataFloat16Color.GetData());
		//Result->LODGroup = TEXTUREGROUP_EffectsNotFiltered; // Mipmap filtering, no compression
		Result->SetLayerFormatSettings(0, FormatSettings);
		//Result->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;



		Result->SetPlatformData(new FTexturePlatformData());
		Result->GetPlatformData()->SizeX = Settings->XYSize;
		Result->GetPlatformData()->SizeY = Settings->XYSize;
		Result->GetPlatformData()->PixelFormat = PF_FloatRGBA;

		//Result->DeferCompression = true; // This forces reloading data when the asset is saved
		//Result->UpdateResource();
		//Result->PostEditChange();
		//Result->MarkPackageDirty();

		//uint8* OutData = Result->Source.LockMip(0);
		//FMemory::Memzero(OutData, Settings->XYSize * Settings->XYSize * sizeof(uint32) * 2);
		//Result->Source.UnlockMip(0);
		//Result->DeferCompression = true; // This forces reloading data when the asset is saved
		//Result->MarkPackageDirty();
	}
	
	//check(Result->GetResource()->GetTexture2DRHI());
	//

	return Result;
}

FVector UFDOverlayEditorAutoCalTool::GetUVOffsetOrigin(const float& UVOffset, const FVector& LineOrigin, const FVector& LineDirection)
{
	FVector axis = UKismetMathLibrary::Normal(LineDirection, 0.0001);
	FVector cross = UKismetMathLibrary::Cross_VectorVector(axis, FVector(0, 0, 1));
	float degress = UKismetMathLibrary::RadiansToDegrees(UKismetMathLibrary::Acos(UKismetMathLibrary::Dot_VectorVector(axis, FVector(0, 0, 1))));
	if (UKismetMathLibrary::Cross_VectorVector(cross, FVector(0, 1, 0)).Z <= 0) // 假定 Y轴 为观察方向，与旋转轴叉积.z为负的，定义为逆时针旋转
	{
		degress = 360 - degress;
	}

	const FVector v = UKismetMathLibrary::RotateAngleAxis(FVector(1, 0, 0), degress, cross);

	// DEPRECATED. 不能取随机方向，否则UV轴就无法正常调整了。
	// const FVector v = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(LineOrigin, 90); 

	FVector v2 = UKismetMathLibrary::RotateAngleAxis(v, UVOffset, axis);
	return v2 * 100.0 + FVector(LineOrigin);
}

float UFDOverlayEditorAutoCalTool::GetDistanceAlongLine(const FVector& Point, const FVector& LineOrigin, const FVector& LineDirection)
{
	const FVector SafeDir = LineDirection.GetSafeNormal();
	const FVector OutClosestPoint = LineOrigin + (SafeDir * ((Point - LineOrigin) | SafeDir));
	return (float)(OutClosestPoint - LineOrigin).Size();		// LWC_TODO: Precision loss
}

#undef LOCTEXT_NAMESPACE