#include "FDOverlayMeshInput.h"
#include "DynamicMesh/DynamicMesh3.h"

using namespace UE::Geometry;

bool UFDOverlayMeshInput::InitializeMeshes(UToolTarget* Target, TSharedPtr<FDynamicMesh3> AppliedCanonicalIn,
	UMeshOpPreviewWithBackgroundCompute* AppliedPreviewIn, int32 AssetIDIn, int32 UVLayerIndexIn, UWorld* UnwrapWorld,
	UWorld* LivePreviewWorld, UMaterialInterface* WorkingMaterialIn, TFunction<FVector3d(const FVector2f&)> UVToVertPositionFuncIn,
	TFunction<FVector2f(const FVector3d&)> VertPositionToUVFuncIn)
{
	
	return false;
}




