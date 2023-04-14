// Copyright HandsomeCheese. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "FDOverlayContextObject.h"
#include "FDOverlay2DViewportClient.h"
#include "FDOverlayLive2DPreviewAPI.generated.h"


/**
 * Allows tools to interact with the 2d preview viewport 
 */
UCLASS()
class FDOVERLAYEDITOR_API UFDOverlayLive2DViewportAPI : public UFDOverlayContextObject
{
	GENERATED_BODY()
public:
	void InitializeDelegate(TUniqueFunction<FOnSwitchMaterialIDMode& ()> OnSwitchMaterialIDModeDelegateFuncIn,
		TUniqueFunction<FOnMaterialIDAdd& ()> OnMaterialIDAddDelegateFuncIn,
		TUniqueFunction<FOnMaterialIDSub& ()> OnMaterialIDSubDelegateFuncIn
	);

	FOnSwitchMaterialIDMode& OnSwitchMaterialIDModeDelegate()
	{
		return OnSwitchMaterialIDModeDelegateFunc();
	}
	FOnMaterialIDAdd& OnMaterialIDAddDelegate()
	{
		return OnMaterialIDAddDelegateFunc();
	}
	FOnMaterialIDSub& OnMaterialIDSubDelegate()
	{
		return OnMaterialIDSubDelegateFunc();
	}
private:
	TUniqueFunction<FOnSwitchMaterialIDMode& ()> OnSwitchMaterialIDModeDelegateFunc;
	TUniqueFunction<FOnMaterialIDAdd& ()> OnMaterialIDAddDelegateFunc;
	TUniqueFunction<FOnMaterialIDSub& ()> OnMaterialIDSubDelegateFunc;
};
