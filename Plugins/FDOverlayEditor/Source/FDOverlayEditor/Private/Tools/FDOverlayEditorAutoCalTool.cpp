#include "Tools/FDOverlayEditorAutoCalTool.h"


#include "DynamicMesh/DynamicMesh3.h"
#include "InteractiveToolManager.h"
#include "MeshOpPreviewHelpers.h" // UMeshOpPreviewWithBackgroundCompute
#include "ContextObjectStore.h"
#include "InteractiveToolManager.h"
#include "Factories/Factory.h"
#include "RendererInterface.h"

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

	OnFinishCS.BindUObject(this, UpdateOutputTexture);

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

	OnFinishCS.UnBind();

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
	for (TObjectPtr<UFDOverlayMeshInput> Target : Targets)
	{
		for (int MID = 0; MID < Target->MaxMaterialIndex; MID++)
		{
			FExtraParams ExtraParams;

			FString AssetPath = GetAssetPath(Settings->AssetPathFormat, Settings->Name, MID);
			UTexture2D* OutputTexture = FindOrCreate(AssetPath);
			ExtraParams.OutputTexture = OutputTexture;

			FFDAutoCalCSInterface::Dispatch(Target->AppliedCanonical, ExtraParams, [this](UTexture2D* OutputTexture) {
				this->OnFinishCS.ExecuteIfBound(OutputTexture);
				});
		}
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

void UFDOverlayEditorAutoCalTool::UpdateOutputTexture(UTexture2D* OutputTexture)
{
	OutputTexture->UpdateResource();
	OutputTexture->PostEditChange();
	OutputTexture->MarkPackageDirty();
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
	return AssetTools.CreateAsset(FString(FPathViews::GetCleanFilename(PackagePath)), FString(FPathViews::GetPath(PackagePath)), UTexture2D::StaticClass(), NewFactory);

}