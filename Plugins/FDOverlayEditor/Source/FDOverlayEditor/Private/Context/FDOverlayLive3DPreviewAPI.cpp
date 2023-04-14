// Copyright HandsomeCheese. All Rights Reserved.
#include "Context/FDOverlayLive3DPreviewAPI.h"


using namespace UE::Geometry;

void UFDOverlayLive3DPreviewAPI::Initialize(UWorld* WorldIn, UInputRouter* RouterIn,
	TUniqueFunction<FOnToggleOverlayChannel&()> OnToggleOverlayChannelDelegateIn,
	TUniqueFunction<FOnToggleOverlayRender& ()> OnToggleOverlayRenderDelegateIn,
	TUniqueFunction<void(const FAxisAlignedBox3d& BoundingBox)> SetLivePreviewCameraToLookAtVolumeFuncIn)
{
	World = WorldIn;
	InputRouter = RouterIn;
	OnToggleOverlayChannelDelegateFunc = MoveTemp(OnToggleOverlayChannelDelegateIn);
	OnToggleOverlayRenderDelegateFunc = MoveTemp(OnToggleOverlayRenderDelegateIn);
	SetLivePreviewCameraToLookAtVolumeFunc = MoveTemp(SetLivePreviewCameraToLookAtVolumeFuncIn);
}

void UFDOverlayLive3DPreviewAPI::OnToolEnded(UInteractiveTool* DeadTool)
{
	OnDrawHUD.RemoveAll(DeadTool);
	OnRender.RemoveAll(DeadTool);
}