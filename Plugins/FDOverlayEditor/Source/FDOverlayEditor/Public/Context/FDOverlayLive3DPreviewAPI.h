// Copyright HandsomeCheese. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "FDOverlayContextObject.h"
#include "FDOverlay3DViewportClient.h"

#include "FDOverlayLive3DPreviewAPI.generated.h"
/**
 * Allows tools to interact with the 3d preview viewport, which has a separate
 * world and input router.
 */
UCLASS()
class FDOVERLAYEDITOR_API UFDOverlayLive3DPreviewAPI : public UFDOverlayContextObject
{
	GENERATED_BODY()
public:

	void Initialize(UWorld* WorldIn, UInputRouter* RouterIn,
		TUniqueFunction<FOnToggleOverlayChannel& ()> OnToggleOverlayChannelDelegateIn,
		TUniqueFunction<FOnToggleOverlayRender& ()> OnToggleOverlayRenderDelegateIn,
		TUniqueFunction<void(const UE::Geometry::FAxisAlignedBox3d& BoundingBox)> SetLivePreviewCameraToLookAtVolumeFuncIn);

	UWorld* GetLivePreviewWorld() { return World.Get(); }
	UInputRouter* GetLivePreviewInputRouter() { return InputRouter.Get(); }
	FOnToggleOverlayChannel& OnToggleOverlayChannelDelegate()
	{
		return OnToggleOverlayChannelDelegateFunc();
	}

	FOnToggleOverlayRender& OnToggleOverlayRenderDelegate()
	{
		return OnToggleOverlayRenderDelegateFunc();
	}

	void SetLivePreviewCameraToLookAtVolume(const UE::Geometry::FAxisAlignedBox3d& BoundingBox)
	{
		if (SetLivePreviewCameraToLookAtVolumeFunc)
		{
			SetLivePreviewCameraToLookAtVolumeFunc(BoundingBox);
		}
	}

	virtual void OnToolEnded(UInteractiveTool* DeadTool) override;

	/**
	 * Broadcast by the 3D live preview viewport on Render() so that mechanics/tools can
	 * render there.
	 */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnRender, IToolsContextRenderAPI* RenderAPI);
	FOnRender OnRender;

	/**
	 * Broadcast by the 3D live preview viewport on DrawHUD() so that mechanics/tools can
	 * draw there.
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDrawHUD, FCanvas* Canvas, IToolsContextRenderAPI* RenderAPI);
	FOnDrawHUD OnDrawHUD;


	DECLARE_MULTICAST_DELEGATE(FOnApplyChangesDelegate);
	FOnApplyChangesDelegate OnApplyChangesDelegate;

protected:
	UPROPERTY()
	TWeakObjectPtr<UWorld> World; // 独立的 World

	UPROPERTY()
	TWeakObjectPtr<UInputRouter> InputRouter; // UInputRouter 是高级输入事件源(例如FEdMode)和一组响应这些事件的 InputBehaviors 之间的中介。

	TUniqueFunction<FOnToggleOverlayChannel&()> OnToggleOverlayChannelDelegateFunc;
	TUniqueFunction<FOnToggleOverlayRender& ()> OnToggleOverlayRenderDelegateFunc;
	TUniqueFunction<void(const UE::Geometry::FAxisAlignedBox3d& BoundingBox)> SetLivePreviewCameraToLookAtVolumeFunc;
};