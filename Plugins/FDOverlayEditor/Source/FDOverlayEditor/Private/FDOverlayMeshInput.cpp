#include "FDOverlayMeshInput.h"

#include "DynamicMesh/DynamicMesh3.h"
#include "MeshOpPreviewHelpers.h"
#include "ModelingToolTargetUtil.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h" //FDynamicMeshUVOverlay
#include "GeometryBase.h"
#include "Materials\MaterialInstanceDynamic.h"
#include "Engine\Texture2DArray.h"

using namespace UE::Geometry;

bool UFDOverlayMeshInput::InitializeMeshes(UToolTarget* Target, TSharedPtr<FDynamicMesh3> AppliedCanonicalIn,
	UMeshOpPreviewWithBackgroundCompute* AppliedPreviewIn, int32 AssetIDIn, int32 UVLayerIndexIn, UWorld* UnwrapWorld,
	UWorld* LivePreviewWorld, UMaterialInterface* WorkingMaterialIn, UMaterialInterface* DefaultBakeMaterialInterfaceIn, TFunction<FVector3d(const FVector2f&)> UVToVertPositionFuncIn,
	TFunction<FVector2f(const FVector3d&)> VertPositionToUVFuncIn)
{

	SourceTarget = Target;
	AssetID = AssetIDIn;
	MaterialIndex = -1; // TODO: 不为 -1 则只保存对应材质ID的模型信息。
	UVToVertPosition = UVToVertPositionFuncIn;
	VertPositionToUV = VertPositionToUVFuncIn;
	UVLayerIndex = UVLayerIndexIn;
	AppliedCanonical = AppliedCanonicalIn;
	DefaultWorkingMaterial = WorkingMaterialIn;
	DefaultBakeMaterialInterface = DefaultBakeMaterialInterfaceIn;

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

	FComponentMaterialSet MSet = UE::ToolTarget::GetMaterialSet(Target);
	PrevMaterialSet.Materials.Empty(MSet.Materials.Num());
	BakeMaterialSet.Materials.Empty(MSet.Materials.Num());
	int MID = 0;
	for (UMaterialInterface* MI : MSet.Materials)
	{
		FName UniqueDynamicName = MakeUniqueObjectName(Target, UMaterialInstanceDynamic::StaticClass(), FName(FString::Printf(TEXT("PrevMID_%03d"), MID)));
		UMaterialInstanceDynamic* MIPrevDynamic = UMaterialInstanceDynamic::Create(MI, Target, UniqueDynamicName);
		PrevMaterialSet.Materials.Add(MIPrevDynamic);

		UniqueDynamicName = MakeUniqueObjectName(Target, UMaterialInstanceDynamic::StaticClass(), FName(FString::Printf(TEXT("BakeMID_%03d"), MID)));
		UMaterialInstanceDynamic* MIBakeDynamic = UMaterialInstanceDynamic::Create(DefaultBakeMaterialInterface, Target, UniqueDynamicName);
		BakeMaterialSet.Materials.Add(MIBakeDynamic);

		MID++;
	}

	AppliedPreview->ConfigureMaterials(PrevMaterialSet.Materials, DefaultWorkingMaterial);
	UnwrapPreview->ConfigureMaterials(PrevMaterialSet.Materials, DefaultWorkingMaterial);

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
			MaxMaterialIndex = MaxMaterialIndex >= MID ? MaxMaterialIndex:MID;
			UnwrapMeshOut.InsertTriangle(Tid, UVTri, 0, true);
			UnwrapMeshUVOverlay->SetTriangle(Tid, UVTri);
			UnwrapMeshMaterialAttribute->SetNewValue(Tid, MID);
		}
	}
	UnwrapMeshOut.EndUnsafeTrianglesInsert();
}

void UFDOverlayMeshInput::ShowToMesh(const UTexture2DArray* BakedSource)
{
	for (int Mid = 0; Mid < BakeMaterialSet.Materials.Num(); Mid++)
	{
		UMaterialInstanceDynamic* MIDynamic = (UMaterialInstanceDynamic*)BakeMaterialSet.Materials[Mid];
		MIDynamic->SetTextureParameterValue(FName(TEXT("Input")), (UTexture*)BakedSource);
		MIDynamic->SetScalarParameterValue(FName(TEXT("MID")), Mid);
		UE_LOG(LogTemp, Warning, TEXT("Change DMI Success %d"), Mid);
	}
	AppliedPreview->ConfigureMaterials(BakeMaterialSet.Materials, DefaultWorkingMaterial);
	UnwrapPreview->ConfigureMaterials(BakeMaterialSet.Materials, DefaultWorkingMaterial);

	
}

