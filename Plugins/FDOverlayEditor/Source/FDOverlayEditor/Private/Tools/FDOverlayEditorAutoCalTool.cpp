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

#include "Operators/FDOverlayEditorAutoCalOp.h"


#include "FDOverlayMeshInput.h"

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
	float RangeMin = 0, RangMax = 0;
	if (Settings->UVCurve)
	{
		CurveKeys.Empty(Settings->UVCurve->FloatCurve.GetNumKeys());
		for (const FRichCurveKey& key : Settings->UVCurve->FloatCurve.GetConstRefOfKeys())
		{
			CurveKeys.Add(FCurveKey(key.Time, key.Value, key.ArriveTangent, key.LeaveTangent));
		}

		Settings->UVCurve->FloatCurve.GetValueRange(RangeMin, RangMax);
	}
	else
	{
		CurveKeys.Add(FCurveKey(0, 0, 0, 0));
	}

	CurveRange = RangMax - RangeMin;
}

void UFDOverlayEditorAutoCalTool::InitializeMeshResource(const TSharedPtr<UE::Geometry::FDynamicMesh3> AppliedCanonical,
	TArray<FAppliedVertex>& AppliedVertices, TArray<FTriangle>& Triangles, FExtraParams& ExtraParams)
{
	FDynamicMeshUVOverlay* UVOverlay = AppliedCanonical->Attributes()->GetUVLayer(0);

	int VerticesNum = UVOverlay->ElementCount();
	int TrianglesNum = AppliedCanonical->TriangleCount();

	AppliedVertices.Reset(VerticesNum);
	double GradientMax = 0;

	auto GetDistanceAlongLine = [](const FVector& Point, const FVector& LineOrigin, const FVector& LineDirection)
	{
		const FVector SafeDir = LineDirection.GetSafeNormal();
		const FVector OutClosestPoint = LineOrigin + (SafeDir * ((Point - LineOrigin) | SafeDir));
		return (float)(OutClosestPoint - LineOrigin).Size();		// LWC_TODO: Precision loss
	};

	for (int32 ElementID = 0; ElementID < VerticesNum; ElementID++)
	{
		FVector2f UVElement = UVOverlay->GetElement(ElementID);
		int32 vid = UVOverlay->GetParentVertex(ElementID);
		FAppliedVertex vert;
		vert.Position = FVector3f(AppliedCanonical->GetVertex(vid));
		vert.Normal = AppliedCanonical->GetVertexNormal(vid);
		vert.UV = FVector2f(0, 0);
		//AppliedVertices.Emplace(FAppliedVertex(AppliedCanonical->GetVertex(i), AppliedCanonical->GetVertexNormal(i), /*AppliedCanonical->GetVertexUV(i)*/ FVector2f(0, 0)));
		AppliedVertices.Add(vert);

		// º∆À„ GradientMax
		float d = GetDistanceAlongLine(FVector(vert.Position), FVector(ExtraParams.Params.GradientOrigin), FVector(ExtraParams.Params.GradientDir));
		if (GradientMax < d)
		{
			GradientMax = d;
		}
	}


	Triangles.Reset(TrianglesNum);
	float xmin = 1, ymin = 1;
	int32 id = 0;
	for (int32 i : AppliedCanonical->TriangleIndicesItr())
	{
		if (UVOverlay->IsSetTriangle(i))
		{
			//FIndex3i tri = AppliedCanonical->GetTriangle(i);
			FIndex3i UVTri = UVOverlay->GetTriangle(i);

			//FTriangle Triangle(trangle.A, trangle.B, trangle.C);
			FTriangle triangle;
			triangle.A = UVTri.A;
			triangle.B = UVTri.B;
			triangle.C = UVTri.C;
			triangle.MID = AppliedCanonical->Attributes()->GetMaterialID()->GetValue(i);
			//Triangles.Emplace(Triangle);
			Triangles.Add(triangle);

			//FIndex3i index3i = UVOverlay->GetTriangle(i);

			for (int j = 0; j < 3; j++)
			{
				FVector2f uv = UVOverlay->GetElement(UVTri.ABC[j]);
				AppliedVertices[/*UVOverlay->GetParentVertex(UVTri.ABC[j])*/UVTri.ABC[j]].UV = uv;

			}
		}

	}

	ExtraParams.Params.GradientMax = GradientMax;
	ExtraParams.Params.TriangleNum = Triangles.Num();
	ExtraParams.Params.VertexNum = AppliedVertices.Num();
}

void UFDOverlayEditorAutoCalTool::InitializeBakePass(int32 MIDNum)
{
	if (BakeBuffer.Num() >= MIDNum)
	{
		for (UTextureRenderTarget2DArray* BakeRT : BakeBuffer)
		{
			BakeRT->Init(Settings->XYSize, Settings->XYSize, MIDNum, PF_FloatRGBA);
		}
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

void UFDOverlayEditorAutoCalTool::AddBakePass(TObjectPtr<UFDOverlayMeshInput> Target, int TargetID)
{
	auto GetUVOffsetOrigin = [](const float& UVOffset, const FVector& LineOrigin, const FVector& LineDirection) {
		FVector axis = UKismetMathLibrary::Normal(LineDirection, 0.0001);
		UE_LOG(LogTemp, Warning, TEXT("DirAxis = %f - %f - %f"), axis.X, axis.Y, axis.Z);
		const FVector v = FVector(-1, 0, 0);

		FVector v2 = UKismetMathLibrary::RotateAngleAxis(v, UVOffset /*UKismetMathLibrary::DegreesToRadians(UVOffset)*/, axis);
		UE_LOG(LogTemp, Warning, TEXT("Rotate v2 = %f - %f - %f"), v2.X, v2.Y, v2.Z);

		return v2 * 100.0 + FVector(LineOrigin);
	};


	FExtraParams ExtraParams;

	//FString AssetPath = GetAssetPath(Settings->AssetPathFormat, Settings->Name, MID);
	//ExtraParams.OutputTexture = FindOrCreate(AssetPath);

	ExtraParams.BakeBuffer = BakeBuffer[TargetID];
	ExtraParams.MIDNum = Target->MaxMaterialIndex + 1;
	ExtraParams.TargetID = TargetID;
	ExtraParams.Params.GradientOrigin = Settings->LineOrigin;
	ExtraParams.Params.GradientDir = Settings->LineDirection;
	ExtraParams.Params.UVCurveOrigin = FVector3f(GetUVOffsetOrigin(Settings->UVOffset, FVector(Settings->LineOrigin), FVector(Settings->LineDirection)));
	ExtraParams.Params.CurveRange = CurveRange;
	ExtraParams.Params.GradientMax = 0;
	ExtraParams.Params.KeyNum = CurveKeys.Num();
	ExtraParams.CurveKeys = CurveKeys;
	ExtraParams.Size = FIntPoint(Settings->XYSize, Settings->XYSize);

	TArray<FAppliedVertex> AppliedVertices;
	TArray<FTriangle> Triangles;

	InitializeMeshResource(Target->AppliedCanonical, AppliedVertices, Triangles, ExtraParams);

	FFDAutoCalCSInterface::Dispatch(MoveTemp(AppliedVertices), MoveTemp(Triangles), ExtraParams, [this](FExtraParams& ExtraParams) {
		UpdateOutputTexture(ExtraParams);
		this->OnFinishCS.ExecuteIfBound(ExtraParams);
		});
}

void UFDOverlayEditorAutoCalTool::OnPropertyModified(UObject* PropertySet, FProperty* Property)
{	
	InitializeCurve();

	switch (Settings->LayoutType)
	{
	case EFDOverlayEditorAutoCalType::Line:
		for (int TargetID = 0; TargetID < Targets.Num(); TargetID++)
		{
			TObjectPtr<UFDOverlayMeshInput> Target = Targets[TargetID];
			InitializeBakePass(Target->MaxMaterialIndex + 1);
			AddBakePass(Target, TargetID);
		}
		break;
	case EFDOverlayEditorAutoCalType::Point:
		break;
	}


	
	for (TObjectPtr<UFDOverlayMeshInput> Target : Targets)
	{
		Target->AppliedPreview->InvalidateResult();
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

void UFDOverlayEditorAutoCalTool::UpdateOutputTexture(FExtraParams& ExtraParams)
{
	UTexture2DArray* NewObj = nullptr;
	FString AssetPath = GetAssetPath(Settings->AssetPathFormat, TEXT("FDOverlayArray"), ExtraParams.TargetID);

	UPackage* Package = CreatePackage(*AssetPath);
	/*UTexture2D* Result = NewObject<UTexture2D>(Package, *FString(TEXT("OutputTex")), RF_Public | RF_Standalone | RF_Transactional);*/
	NewObj = ExtraParams.BakeBuffer->ConstructTexture2DArray(Package, FString(FPathViews::GetCleanFilename(AssetPath)), RF_Public | RF_Standalone | RF_Transactional);
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
	

	Targets[ExtraParams.TargetID]->ShowToMesh(NewObj);

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

