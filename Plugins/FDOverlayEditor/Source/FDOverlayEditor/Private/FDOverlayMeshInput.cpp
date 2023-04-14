// Copyright HandsomeCheese. All Rights Reserved.
#include "FDOverlayMeshInput.h"

#include "DynamicMesh/DynamicMesh3.h"
#include "MeshOpPreviewHelpers.h"
#include "ModelingToolTargetUtil.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h" //FDynamicMeshUVOverlay
#include "GeometryBase.h"
#include "Materials\MaterialInstanceDynamic.h"
#include "Engine\Texture2DArray.h"
#include "ToolSetupUtil.h"

using namespace UE::Geometry;

bool UFDOverlayMeshInput::InitializeMeshes(UToolTarget* Target, TSharedPtr<FDynamicMesh3> AppliedCanonicalIn,
	UMeshOpPreviewWithBackgroundCompute* AppliedPreviewIn, int32 AssetIDIn, int32 UVLayerIndexIn, UWorld* UnwrapWorld,
	UWorld* LivePreviewWorldIn, UMaterialInterface* DefaultBakeAppliedMaterialInterfaceIn, UMaterialInterface* EmissiveBakeAppliedMaterialInterfaceIn,
	UMaterialInterface* TranslucencyBakeUnwrapMaterialInterfaceIn, UMaterialInterface* TransitionBakeAppliedMaterialInterfaceIn, UMaterialInterface* DefaultBakeUnwrapMaterialInterfaceIn, 
	TFunction<FVector3d(const FVector2f&)> UVToVertPositionFuncIn, TFunction<FVector2f(const FVector3d&)> VertPositionToUVFuncIn)
{

	SourceTarget = Target;
	AssetID = AssetIDIn;
	MaterialIndex = -1; // TODO: 不为 -1 则只保存对应材质ID的模型信息。
	UVToVertPosition = UVToVertPositionFuncIn;
	VertPositionToUV = VertPositionToUVFuncIn;
	UVLayerIndex = UVLayerIndexIn;
	AppliedCanonical = AppliedCanonicalIn;
	DefaultBakeAppliedMaterialInterface = DefaultBakeAppliedMaterialInterfaceIn;
	EmissiveBakeAppliedMaterialInterface = EmissiveBakeAppliedMaterialInterfaceIn;
	TranslucencyBakeUnwrapMaterialInterface = TranslucencyBakeUnwrapMaterialInterfaceIn;
	TransitionBakeAppliedMaterialInterface = TransitionBakeAppliedMaterialInterfaceIn;
	DefaultBakeUnwrapMaterialInterface = DefaultBakeUnwrapMaterialInterfaceIn;

	if (!AppliedCanonical->HasAttributes())
	{
		return false;
	}

	AppliedPreview = AppliedPreviewIn;


	// Set up the unwrapped mesh
	UnwrapCanonical = MakeShared<FDynamicMesh3>();
	GenerateUVUnwrapMesh(*AppliedCanonical->Attributes()->GetUVLayer(UVLayerIndex), *UnwrapCanonical, UVToVertPosition);
	UnwrapCanonical->SetShapeChangeStampEnabled(true);

	// Set up the unwrap preview
	UnwrapPreview = NewObject<UMeshOpPreviewWithBackgroundCompute>();
	UnwrapPreview->Setup(UnwrapWorld);
	UnwrapPreview->PreviewMesh->UpdatePreview(UnwrapCanonical.Get());

	FComponentMaterialSet MSet = UE::ToolTarget::GetMaterialSet(SourceTarget);
	PrevMaterialSet.Materials.Empty(MSet.Materials.Num());
	DefaultBakeAppliedMaterialSet.Materials.Empty(MSet.Materials.Num());
	EmissiveBakeAppliedMaterialSet.Materials.Empty(MSet.Materials.Num());
	TranslucencyBakeAppliedMaterialSet.Materials.Empty(MSet.Materials.Num());
	TransitionBakeAppliedMaterialSet.Materials.Empty(MSet.Materials.Num());
	BakeUnwrapMaterialSet.Materials.Empty(MSet.Materials.Num());
	int MID = 0;
	for (UMaterialInterface* MI : MSet.Materials)
	{
		FName UniqueDynamicName = MakeUniqueObjectName(SourceTarget, UMaterialInstanceDynamic::StaticClass(), FName(FString::Printf(TEXT("PrevMID_%03d"), MID)));
		UMaterialInstanceDynamic* MIPrevDynamic = UMaterialInstanceDynamic::Create(MI, SourceTarget, UniqueDynamicName);
		PrevMaterialSet.Materials.Add(MIPrevDynamic);

		UniqueDynamicName = MakeUniqueObjectName(SourceTarget, UMaterialInstanceDynamic::StaticClass(), FName(FString::Printf(TEXT("DefaultBakeAppliedMID_%03d"), MID)));
		UMaterialInstanceDynamic* MIDefaultBakeAppliedDynamic = UMaterialInstanceDynamic::Create(DefaultBakeAppliedMaterialInterface, SourceTarget, UniqueDynamicName);
		DefaultBakeAppliedMaterialSet.Materials.Add(MIDefaultBakeAppliedDynamic);

		UniqueDynamicName = MakeUniqueObjectName(SourceTarget, UMaterialInstanceDynamic::StaticClass(), FName(FString::Printf(TEXT("EmissiveBakeAppliedMID_%03d"), MID)));
		UMaterialInstanceDynamic* MIEmissiveBakeAppliedDynamic = UMaterialInstanceDynamic::Create(EmissiveBakeAppliedMaterialInterface, SourceTarget, UniqueDynamicName);
		EmissiveBakeAppliedMaterialSet.Materials.Add(MIEmissiveBakeAppliedDynamic);

		UniqueDynamicName = MakeUniqueObjectName(SourceTarget, UMaterialInstanceDynamic::StaticClass(), FName(FString::Printf(TEXT("TranslucencyBakeAppliedMID_%03d"), MID)));
		UMaterialInstanceDynamic* MITranslucencyBakeAppliedDynamic = UMaterialInstanceDynamic::Create(TranslucencyBakeUnwrapMaterialInterface, SourceTarget, UniqueDynamicName);
		TranslucencyBakeAppliedMaterialSet.Materials.Add(MITranslucencyBakeAppliedDynamic);

		UniqueDynamicName = MakeUniqueObjectName(SourceTarget, UMaterialInstanceDynamic::StaticClass(), FName(FString::Printf(TEXT("TransitionBakeAppliedMID_%03d"), MID)));
		UMaterialInstanceDynamic* MITransitionBakeAppliedDynamic = UMaterialInstanceDynamic::Create(TransitionBakeAppliedMaterialInterface, SourceTarget, UniqueDynamicName);
		TransitionBakeAppliedMaterialSet.Materials.Add(MITransitionBakeAppliedDynamic);

		UniqueDynamicName = MakeUniqueObjectName(SourceTarget, UMaterialInstanceDynamic::StaticClass(), FName(FString::Printf(TEXT("BakeUnwrapMID_%03d"), MID)));
		UMaterialInstanceDynamic* MIBakeUnwrapDynamic = UMaterialInstanceDynamic::Create(DefaultBakeUnwrapMaterialInterface, SourceTarget, UniqueDynamicName);
		BakeUnwrapMaterialSet.Materials.Add(MIBakeUnwrapDynamic);

		MID++;
	}

	// 默认都是直接使用的 PrevMaterialSet.Materials
	AppliedPreview->ConfigureMaterials(PrevMaterialSet.Materials, DefaultBakeAppliedMaterialInterface);
	UnwrapPreview->ConfigureMaterials(BakeUnwrapMaterialSet.Materials, DefaultBakeUnwrapMaterialInterface);

	return true;
}

void UFDOverlayMeshInput::GenerateUVUnwrapMesh(const FDynamicMeshUVOverlay& UVOverlay, FDynamicMesh3& UnwrapMeshOut,
	TFunctionRef<FVector3d(const FVector2f&)> UVToVertPositionIn)
{
	UnwrapMeshOut.Clear();

	// The unwrap mesh will have an overlay on top of it with the corresponding UVs,
	// in case we want to draw the texture on it, etc. However note that we can't
	// just do a Copy() call using the source overlay because the parent vertices will differ.
	UnwrapMeshOut.EnableAttributes(); // Makes one UV layer

	UnwrapMeshOut.Attributes()->SetNumUVLayers(1);
	UnwrapMeshOut.Attributes()->SetNumNormalLayers(1);
	FDynamicMeshUVOverlay* UnwrapMeshUVOverlay = UnwrapMeshOut.Attributes()->GetUVLayer(0);
	FDynamicMeshNormalOverlay* UnwrapMeshNormalOverlay = UnwrapMeshOut.Attributes()->GetNormalLayer(0);
	UnwrapMeshOut.Attributes()->EnableMaterialID();
	FDynamicMeshMaterialAttribute* UnwrapMeshMaterialAttribute = UnwrapMeshOut.Attributes()->GetMaterialID();

	// Create a vert for each uv overlay element
	UnwrapMeshOut.BeginUnsafeVerticesInsert();
	UnwrapMeshUVOverlay->BeginUnsafeElementsInsert();
	UnwrapMeshNormalOverlay->BeginUnsafeElementsInsert();
	for (int32 ElementID : UVOverlay.ElementIndicesItr())
	{
		FVector2f UVElement = UVOverlay.GetElement(ElementID);

		UnwrapMeshOut.InsertVertex(ElementID, UVToVertPositionIn(UVElement), true);

		UnwrapMeshUVOverlay->InsertElement(ElementID, &UVElement.X, true);
		UnwrapMeshUVOverlay->SetParentVertex(ElementID, ElementID);

		UnwrapMeshNormalOverlay->InsertElement(ElementID, &UVElement.X, true);

	}
	UnwrapMeshOut.EndUnsafeVerticesInsert();
	UnwrapMeshUVOverlay->EndUnsafeElementsInsert();
	UnwrapMeshNormalOverlay->EndUnsafeElementsInsert();

	UnwrapMeshOut.EnableVertexUVs(FVector2f(0, 0));
	/*UnwrapMeshOut.EnableTriangleGroups();*/
	UnwrapMeshOut.EnableVertexNormals(FVector3f(0, 0, 1));

	for (int32 vid = 0; vid < UnwrapMeshOut.VertexCount(); vid++)
	{
		FVector2f uv = UVOverlay.GetElement(vid);
		UnwrapMeshOut.SetVertexUV(vid, uv);
		UnwrapMeshUVOverlay->SetElement(vid, uv);
	}

	// Insert a tri connecting the same vids as elements in the overlay.
	const FDynamicMesh3* OverlayParentMesh = UVOverlay.GetParentMesh();
	UnwrapMeshOut.BeginUnsafeTrianglesInsert();
	for (int32 Tid : OverlayParentMesh->TriangleIndicesItr())
	{
		if (UVOverlay.IsSetTriangle(Tid))
		{
			FIndex3i UVTri = UVOverlay.GetTriangle(Tid);
			int32 MID = OverlayParentMesh->Attributes()->GetMaterialID()->GetValue(Tid);
			MaxMaterialIndex = MaxMaterialIndex >= MID ? MaxMaterialIndex : MID;
			UnwrapMeshOut.InsertTriangle(Tid, UVTri, 0, true);
			UnwrapMeshUVOverlay->SetTriangle(Tid, UVTri);
			UnwrapMeshMaterialAttribute->SetNewValue(Tid, MID);
		}
	}
	UnwrapMeshOut.EndUnsafeTrianglesInsert();


}

void UFDOverlayMeshInput::ShowToMesh(const UTexture2DArray* BakedSource)
{
	UMaterialInstanceDynamic* MI = nullptr;
	FComponentMaterialSet* MSet = nullptr;
	
	FindCurrentRenderMaterial(MI, MSet);
	for (int Mid = 0; Mid < MSet->Materials.Num(); Mid++)
	{
		UMaterialInstanceDynamic* MIDynamic = (UMaterialInstanceDynamic*)MSet->Materials[Mid];
		MIDynamic->SetTextureParameterValue(FName(TEXT("Input")), (UTexture*)BakedSource);
		MIDynamic->SetScalarParameterValue(FName(TEXT("MID")), Mid);
		UE_LOG(LogTemp, Warning, TEXT("Change DMI Success %d"), Mid);
	}
	AppliedPreview->ConfigureMaterials(MSet->Materials, MI);

	for (int Mid = 0; Mid < BakeUnwrapMaterialSet.Materials.Num(); Mid++)
	{
		UMaterialInstanceDynamic* MIDynamic = (UMaterialInstanceDynamic*)BakeUnwrapMaterialSet.Materials[Mid];
		MIDynamic->SetTextureParameterValue(FName(TEXT("Input")), (UTexture*)BakedSource);
		MIDynamic->SetScalarParameterValue(FName(TEXT("MID")), Mid);
		UE_LOG(LogTemp, Warning, TEXT("Change DMI Success %d"), Mid);
	}
	UnwrapPreview->ConfigureMaterials(BakeUnwrapMaterialSet.Materials, DefaultBakeUnwrapMaterialInterface);

	
}

void UFDOverlayMeshInput::ChangeDynamicMaterialDisplayChannel(uint8 Channel, bool bIsDisplay)
{	
	UMaterialInstanceDynamic* MI = nullptr;
	FComponentMaterialSet* MSet = nullptr;

	FindCurrentRenderMaterial(MI, MSet);
	for (int Mid = 0; Mid < MSet->Materials.Num(); Mid++)
	{
		UMaterialInstanceDynamic* MIDynamic = (UMaterialInstanceDynamic*)MSet->Materials[Mid];
		MIDynamic->SetScalarParameterValue(FName(FString::Printf(TEXT("Channel%d"), Channel)), bIsDisplay);
	}
	AppliedPreview->ConfigureMaterials(MSet->Materials, MI);

	for (int Mid = 0; Mid < BakeUnwrapMaterialSet.Materials.Num(); Mid++)
	{
		UMaterialInstanceDynamic* MIDynamic = (UMaterialInstanceDynamic*)BakeUnwrapMaterialSet.Materials[Mid];
		MIDynamic->SetScalarParameterValue(FName(FString::Printf(TEXT("Channel%d"), Channel)), bIsDisplay);
	}
	UnwrapPreview->ConfigureMaterials(BakeUnwrapMaterialSet.Materials, DefaultBakeUnwrapMaterialInterface);
}

void UFDOverlayMeshInput::ChangeDynamicMaterialDisplayRender(uint8 RenderModeIn)
{
	RenderMode = RenderModeIn;
	UMaterialInstanceDynamic* MI = nullptr;
	FComponentMaterialSet* MSet = nullptr;

	FindCurrentRenderMaterial(MI, MSet);
	AppliedPreview->ConfigureMaterials(MSet->Materials, MI);
}

void UFDOverlayMeshInput::FindCurrentRenderMaterial(UMaterialInstanceDynamic*& MI, FComponentMaterialSet*& MSet)
{
	switch (RenderMode)
	{
	case 0:
		{
			MI = (UMaterialInstanceDynamic*)(DefaultBakeAppliedMaterialInterface);
			MSet = &DefaultBakeAppliedMaterialSet;
			return;
		}
	case 1:
		{
			MI = (UMaterialInstanceDynamic*)(EmissiveBakeAppliedMaterialInterface);
			MSet = &EmissiveBakeAppliedMaterialSet;
			return;
		}
	case 2:
		{
			MI = (UMaterialInstanceDynamic*)(TranslucencyBakeUnwrapMaterialInterface);
			MSet = &TranslucencyBakeAppliedMaterialSet;
			return;
		}
	case 3:
		{
			MI = (UMaterialInstanceDynamic*)(TransitionBakeAppliedMaterialInterface);
			MSet = &TransitionBakeAppliedMaterialSet;
			return;
		}
	}
}

void UFDOverlayMeshInput::SwitchDynamicMaterialDisplayIDMode(uint8 style /* = 0 */)
{
	switch (style)
	{
		case 0: 
			MaterialIndex = -1;
			break;
		default:
			MaterialIndex = 0;
			break;
	}
	UpdateOpacity();
}
void UFDOverlayMeshInput::AddDynamicMaterialDisplayID()
{
	if (MaterialIndex < MaxMaterialIndex)
	{
		MaterialIndex++;
	}
	UE_LOG(LogTemp, Warning, TEXT("%d"), MaterialIndex);
	UpdateOpacity();
}
void UFDOverlayMeshInput::SubDynamicMaterialDisplayID()
{
	if (MaterialIndex > 0)
	{
		MaterialIndex--;
	}
	UE_LOG(LogTemp, Warning, TEXT("%d"), MaterialIndex);
	UpdateOpacity();
}
void UFDOverlayMeshInput::UpdateOpacity()
{
	for (int Mid = 0; Mid < BakeUnwrapMaterialSet.Materials.Num(); Mid++)
	{
		UMaterialInstanceDynamic* MIDynamic = (UMaterialInstanceDynamic*)BakeUnwrapMaterialSet.Materials[Mid];
		MIDynamic->SetScalarParameterValue(FName(TEXT("Opacity")), MaterialIndex == -1 ? 1 : MaterialIndex == Mid ? 1 : 0);
	}
}

void UFDOverlayMeshInput::Shutdown()
{
	UnwrapCanonical = nullptr;
	UnwrapPreview->Shutdown();
	UnwrapPreview = nullptr;
	AppliedCanonical = nullptr;
	// Can't shut down AppliedPreview because it is owned by mode
	AppliedPreview = nullptr;

	SourceTarget = nullptr;

	OnCanonicalModified.Clear();


	DefaultBakeAppliedMaterialInterface = nullptr;
	EmissiveBakeAppliedMaterialInterface = nullptr;
	TranslucencyBakeUnwrapMaterialInterface = nullptr;
	TransitionBakeAppliedMaterialInterface = nullptr;
	DefaultBakeUnwrapMaterialInterface = nullptr;
	PrevMaterialSet.Materials.Empty();
	DefaultBakeAppliedMaterialSet.Materials.Empty();
	EmissiveBakeAppliedMaterialSet.Materials.Empty();
	TranslucencyBakeAppliedMaterialSet.Materials.Empty();
	TransitionBakeAppliedMaterialSet.Materials.Empty();
	BakeUnwrapMaterialSet.Materials.Empty();

}
