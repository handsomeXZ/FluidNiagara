
#pragma once

#include "CoreMinimal.h"

#include "FDAssistorContextObject.h"

#include "FDAssistorLiveAssetAndChannelAPI.generated.h"


/**
 * Allows tools to interact with the assets and their UV layers
*/
UCLASS()
class FDASSISTOR_API UFDAssistorAssetAndChannelAPI : public UFDAssistorContextObject
{
	GENERATED_BODY()
public:

	TArray<int32> GetCurrentChannelVisibility()
	{
		if (GetCurrentChannelVisibilityFunc)
		{
			return GetCurrentChannelVisibilityFunc();
		}
		return TArray<int32>();
	}

	void RequestChannelVisibilityChange(const TArray<int32>& ChannelPerAsset, bool bEmitUndoTransaction=true)
	{
		if (RequestChannelVisibilityChangeFunc)
		{
			RequestChannelVisibilityChangeFunc(ChannelPerAsset, bEmitUndoTransaction);
		}
	}

	void NotifyOfAssetChannelCountChange(int32 AssetID)
	{
		if (NotifyOfAssetChannelCountChangeFunc)
		{
			NotifyOfAssetChannelCountChangeFunc(AssetID);
		}
	}


	TUniqueFunction<TArray<int32>()> GetCurrentChannelVisibilityFunc;
	TUniqueFunction<void(const TArray<int32>&, bool)> RequestChannelVisibilityChangeFunc;
	TUniqueFunction<void(int32 AssetID)> NotifyOfAssetChannelCountChangeFunc;

};
