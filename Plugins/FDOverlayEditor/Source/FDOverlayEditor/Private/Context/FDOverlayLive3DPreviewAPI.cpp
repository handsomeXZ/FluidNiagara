#include "Context/FDOverlayLive3DPreviewAPI.h"


using namespace UE::Geometry;

void UFDOverlayLive3DPreviewAPI::Initialize(UWorld* WorldIn, UInputRouter* RouterIn,
	TUniqueFunction<FOnToggleOverlayChannel&()> OnToggleOverlayChannelDelegateIn,
	TUniqueFunction<void(const FAxisAlignedBox3d& BoundingBox)> SetLivePreviewCameraToLookAtVolumeFuncIn)
{
	World = WorldIn;
	InputRouter = RouterIn;
	OnToggleOverlayChannelDelegateFunc = MoveTemp(OnToggleOverlayChannelDelegateIn);
	SetLivePreviewCameraToLookAtVolumeFunc = MoveTemp(SetLivePreviewCameraToLookAtVolumeFuncIn);
}

void UFDOverlayLive3DPreviewAPI::OnToolEnded(UInteractiveTool* DeadTool)
{
	OnDrawHUD.RemoveAll(DeadTool);
	OnRender.RemoveAll(DeadTool);
}