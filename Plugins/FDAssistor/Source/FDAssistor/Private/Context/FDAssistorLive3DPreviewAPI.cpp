#include "FDAssistorLive3DPreviewAPI.h"


void UFDAssistorLive3DPreviewAPI::Initialize(UWorld* WorldIn, UInputRouter* RouterIn,
	TUniqueFunction<void(FViewCameraState& CameraStateOut)> GetLivePreviewCameraStateFuncIn,
	TUniqueFunction<void(const FAxisAlignedBox3d& BoundingBox)> SetLivePreviewCameraToLookAtVolumeFuncIn)
{
	World = WorldIn;
	InputRouter = RouterIn;
	GetLivePreviewCameraStateFunc = MoveTemp(GetLivePreviewCameraStateFuncIn);
	SetLivePreviewCameraToLookAtVolumeFunc = MoveTemp(SetLivePreviewCameraToLookAtVolumeFuncIn);
}

