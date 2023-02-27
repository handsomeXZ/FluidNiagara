#include "Context/FDOverlayLive2DPreviewAPI.h"


void UFDOverlayLive2DViewportAPI::InitializeDelegate(TUniqueFunction<FOnSwitchMaterialIDMode& ()> OnSwitchMaterialIDModeDelegateFuncIn, 
	TUniqueFunction<FOnMaterialIDAdd& ()> OnMaterialIDAddDelegateFuncIn, 
	TUniqueFunction<FOnMaterialIDSub& ()> OnMaterialIDSubDelegateFuncIn)
{
	OnSwitchMaterialIDModeDelegateFunc = MoveTemp(OnSwitchMaterialIDModeDelegateFuncIn);
	OnMaterialIDAddDelegateFunc = MoveTemp(OnMaterialIDAddDelegateFuncIn);
	OnMaterialIDSubDelegateFunc = MoveTemp(OnMaterialIDSubDelegateFuncIn);
}