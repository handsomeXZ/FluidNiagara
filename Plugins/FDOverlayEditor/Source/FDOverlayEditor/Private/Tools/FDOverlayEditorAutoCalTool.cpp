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


#include "Operators/FDOverlayEditorAutoCalOp.h"
#include "FDAutoCalCS.h"

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

void UFDOverlayEditorAutoCalTool::OnPropertyModified(UObject* PropertySet, FProperty* Property)
{

	auto GetUVOffsetOrigin = [](float UVOffset, FVector3f LineOrigin, FVector3f LineDirection) {
		FVector axis = FVector(Normalize(LineDirection));
		FVector v = FVector(LineOrigin);
		v.X = v.X - 10;
		v = v - FVector(LineOrigin);
		FVector v2 = UKismetMathLibrary::RotateAngleAxis(v, UKismetMathLibrary::DegreesToRadians(UVOffset), axis);
		return v2 + FVector(LineOrigin);
	};

	TArray<FCurveKey> CurveKeys;
	float RangeMin = 0, RangMax = 0;
	if (Settings->UVCurve)
	{
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

	switch (Settings->LayoutType)
	{
	case EFDOverlayEditorAutoCalType::Line:
		for (TObjectPtr<UFDOverlayMeshInput> Target : Targets)
		{
			for (int MID = 0; MID < Target->MaxMaterialIndex; MID++)
			{
				FExtraParams ExtraParams;

				FString AssetPath = GetAssetPath(Settings->AssetPathFormat, Settings->Name, MID);

				OutputTexture = FindOrCreate(AssetPath);

				ExtraParams.OutputTexture = OutputTexture;
				ExtraParams.MaterialID = MID;
				ExtraParams.Params.GradientOrigin = Settings->LineOrigin;
				ExtraParams.Params.GradientDir = Settings->LineDirection;
				ExtraParams.Params.UVCurveOrigin = FVector3f(GetUVOffsetOrigin(Settings->UVOffset, Settings->LineOrigin, Settings->LineDirection));
				ExtraParams.Params.CurveRange = RangMax - RangeMin;
				ExtraParams.Params.GradientMax = 0;
				ExtraParams.CurveKeys = CurveKeys;
				ExtraParams.Size = FIntPoint(Settings->XYSize, Settings->XYSize);


				/*ExtraParams.Params.GradientOrigin*/
				FFDAutoCalCSInterface::Dispatch(Target->AppliedCanonical, ExtraParams, [this](UTexture2D* OutputTexture) {
					UpdateOutputTexture(OutputTexture);
					this->OnFinishCS.ExecuteIfBound(OutputTexture);
					});
			}
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

void UFDOverlayEditorAutoCalTool::UpdateOutputTexture(UTexture2D* OutputTextureIn)
{
	OutputTextureIn->UpdateResource();
	OutputTextureIn->PostEditChange();
	OutputTextureIn->MarkPackageDirty();
}

FString UFDOverlayEditorAutoCalTool::GetAssetPath(FString PathFormat, FString Name, int32 MaterialID) const
{

	const TMap<FString, FStringFormatArg> PathFormatArgs =
	{
		{TEXT("AssetFolder"),	TEXT("/Game")},
		{TEXT("AssetName"),		TEXT("FDOverlay")},
		{TEXT("OutputName"),	Name},
		{TEXT("MaterialID"),	FString::Printf(TEXT("%03d"), MaterialID)},
	};
	FString AssetPath = FString::Format(*PathFormat, PathFormatArgs);
	AssetPath.ReplaceInline(TEXT("//"), TEXT("/"));
	return AssetPath;
}

UTexture2D* UFDOverlayEditorAutoCalTool::FindOrCreate(const FString& AssetPath)
{
	// Create new Asset
	IAssetTools& AssetTools = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UFactory* NewFactory = NewObject<UFactory>(GetTransientPackage(), UTexture2DFactoryNew::StaticClass());
	UObject* ResultObject = AssetTools.CreateAsset(FString(FPathViews::GetCleanFilename(AssetPath)), FString(FPathViews::GetPath(AssetPath)), UTexture2D::StaticClass(), NewFactory);

	return ResultObject ? CastChecked<UTexture2D>(ResultObject) : nullptr;
}