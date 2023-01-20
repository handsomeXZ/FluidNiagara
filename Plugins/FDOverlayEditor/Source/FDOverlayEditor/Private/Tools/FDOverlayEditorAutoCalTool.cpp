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


#include "FDOverlayMeshInput.h"

#define LOCTEXT_NAMESPACE "FDOverlayEditorAutoCalTool"

using namespace UE::Geometry;


bool UFDOverlayEditorAutoCalToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	return Targets && Targets->Num() > 0;
}

UInteractiveTool* UFDOverlayEditorAutoCalToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	UFDOverlayEditorAutoCalTool* NewTool = NewObject<UFDOverlayEditorAutoCalTool>(SceneState.ToolManager);
	NewTool->SetTarget(*Targets);
	
	return NewTool;
}

void UFDOverlayEditorAutoCalTool::SetTarget(const TArray<TObjectPtr<UFDOverlayMeshInput>>& TargetsIn)
{
	Targets = TargetsIn;
	
}


void UFDOverlayEditorAutoCalTool::Setup()
{
	check(Targets.Num() > 0);

	UInteractiveTool::Setup();

	Settings = NewObject<UFDOverlayEditorAutoCalProperties>(this);
	Settings->RestoreProperties(this);
	AddToolPropertySource(Settings);

	UContextObjectStore* ContextStore = GetToolManager()->GetContextObjectStore();


	for (TObjectPtr<UFDOverlayMeshInput> Target : Targets)
	{
		Target->AppliedPreview->OnMeshUpdated.AddWeakLambda(this, [this, &Target](UMeshOpPreviewWithBackgroundCompute* Preview)
			{
				//Target->UpdateUnwrapPreviewFromAppliedPreview();
			});
	}

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

	Settings = nullptr;
	Targets.Empty();
	CurveKeys.Empty();
	BakeBuffer.Empty();
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
	auto MultiInitialize = [this](TArray<TArray<FCurveKey>>& CurveList, TArray<float>& RangeList) {
		if (Settings->MultiLineData.Num() == 0)
		{
			TArray<FCurveKey> arr;
			arr.Emplace(FCurveKey(0, 0, 0, 0));
			CurveList.Empty(1);
			RangeList.Empty(1);
			CurveList.Add(MoveTemp(arr));
			RangeList.Emplace(float(0));
			return;
		}

		CurveList.Empty(Settings->MultiLineData.Num());
		RangeList.Empty(Settings->MultiLineData.Num());
		for (const FMultiLineData& data : Settings->MultiLineData)
		{
			float RangeMin = 0, RangMax = 0;
			TArray<FCurveKey> arr;
			if (data.UVCurve)
			{
				arr.Empty(data.UVCurve->FloatCurve.GetNumKeys());
				for (const FRichCurveKey& key : data.UVCurve->FloatCurve.GetConstRefOfKeys())
				{
					arr.Emplace(FCurveKey(key.Time, key.Value, key.ArriveTangent, key.LeaveTangent));
				}
				data.UVCurve->FloatCurve.GetValueRange(RangeMin, RangMax);
			}
			else
			{
				arr.Empty(1);
				arr.Emplace(FCurveKey(0, 0, 0, 0));
			}
			CurveList.Add(MoveTemp(arr));
			RangeList.Emplace(float(RangMax - RangeMin));
		}
	};

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
		MultiInitialize(MultiLineCurveKeys, MultiLineCurveRange);
	}
	else if (Settings->LayoutType == EFDOverlayEditorAutoCalType::MultiPoint)
	{
		MultiInitialize(MultiPointCurveKeys, MultiPointCurveRange);
	}
}


void UFDOverlayEditorAutoCalTool::InitializeBakeParams(FExtraParams& ExtraParams, int TargetID, FVector3f Origin, FVector3f Direction, const TArray<FCurveKey>& CurveKeysIn)
{
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
		float d = GetDistanceAlongLine(FVector(Vert.Position), FVector(ExtraParams.Params.GradientOrigin), FVector(ExtraParams.Params.GradientDir));
		if (GradientMax < d)
		{
			GradientMax = d;
		}
	}
	
	ExtraParams.Params.GradientMax = GradientMax;
	ExtraParams.Params.TriangleNum = Triangles[TargetID].Num();
	ExtraParams.Params.VertexNum = Vertices[TargetID].Num();
}

template<typename T>
void UFDOverlayEditorAutoCalTool::InitializeBakeParams(FMultiExtraParams& ExtraParams, int TargetID, const TArray<T>& DataList, const TArray<float>& MultiCurveRange, const TArray<TArray<FCurveKey>>& MultiCurveKeys)
{
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
			float d = GetDistanceAlongLine(FVector(Vert.Position), FVector(data.LineOrigin), FVector(data.LineDirection));
			if (GradientMax < d)
			{
				GradientMax = d;
			}
		}
		ExtraParams.ParamsArr.Emplace(FMultiParamsArr(data.LineOrigin, data.LineDirection, GradientMax, 
		FVector3f(GetUVOffsetOrigin(data.UVOffset, FVector(data.LineOrigin), FVector(data.LineDirection))), 
			MultiCurveRange[i], MultiCurveKeys[i].Num()));
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

		break;
	case EFDOverlayEditorAutoCalType::MultiLine:
		FFDAutoCalCSInterface::Dispatch<T, EFDAutoCalCSType::MultiLineCS>(Vertices[TargetID], Triangles[TargetID], ExtraParams, [this](T ExtraParams) {
			UpdateOutputTexture(ExtraParams.BakeBuffer, ExtraParams.TargetID);
			this->OnFinishCS.ExecuteIfBound();
			});
		break;
	case EFDOverlayEditorAutoCalType::MultiPoint:

		break;
	}

}

void UFDOverlayEditorAutoCalTool::OnPropertyModified(UObject* PropertySet, FProperty* Property)
{	
	InitializeCurve();
	InitializeMeshResource();

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
			InitializeBakeParams<FMultiLineData>(ExtraParams, TargetID, Settings->MultiLineData, MultiLineCurveRange, MultiLineCurveKeys);
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
			InitializeBakeParams<FMultiPointData>(ExtraParams, TargetID, Settings->MultiPointData, MultiPointCurveRange, MultiPointCurveKeys);
			AddBakePass<FMultiExtraParams>(ExtraParams, TargetID, EFDOverlayEditorAutoCalType::MultiPoint);
		}
		break;
	}

	
	for (TObjectPtr<UFDOverlayMeshInput> Target : Targets)
	{
		Target->AppliedPreview->InvalidateResult();
	}
	FSlateNotificationManager::Get().AddNotification(FNotificationInfo(LOCTEXT("FDOverlayEditorAutoCalTool", "Update Finished")))->SetCompletionState(SNotificationItem::CS_Success);
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
	FString AssetPath = GetAssetPath(Settings->AssetPathFormat, TEXT("FDOverlayArray"), TargetID);

	UPackage* Package = CreatePackage(*AssetPath);
	/*UTexture2D* Result = NewObject<UTexture2D>(Package, *FString(TEXT("OutputTex")), RF_Public | RF_Standalone | RF_Transactional);*/
	NewObj = BakeBufferIn->ConstructTexture2DArray(Package, FString(FPathViews::GetCleanFilename(AssetPath)), RF_Public | RF_Standalone | RF_Transactional);
	if (NewObj)
	{
		// Notify the asset registry
		FAssetRegistryModule::AssetCreated((UObject*)NewObj);
		// Mark the package dirty...
		Package->MarkPackageDirty();
		NewObj->MarkPackageDirty();
	}
	//ExtraParams.OutputTexture->UpdateResource();
	//ExtraParams.OutputTexture->PostEditChange();
	//ExtraParams.OutputTexture->MarkPackageDirty();
	

	Targets[TargetID]->ShowToMesh(NewObj);

	UE_LOG(LogTemp, Warning, TEXT("Finish This CS And CallBack"));
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