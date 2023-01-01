#pragma once

#include "CoreMinimal.h"

#include "FDOverlayContextObject.h"

#include "FDOverlayLive2DPreviewAPI.generated.h"


//USTRUCT()
//struct UVEDITORTOOLS_API FUDIMBlock
//{
//	GENERATED_BODY();
//
//	UPROPERTY()
//	int32 UDIM = 1001;
//
//	int32 BlockU() const;
//	int32 BlockV() const;
//	void SetFromBlocks(int32 BlockU, int32 BlockV);
//};

/**
 * Allows tools to interact with the 2d preview viewport 
 */
UCLASS()
class FDOVERLAYEDITOR_API UFDOverlayLive2DViewportAPI : public UFDOverlayContextObject
{
	GENERATED_BODY()
public:

	//void SetUDIMBlocks(TArray<FUDIMBlock>& BlocksIn, bool bBroadcast = true)
	//{
	//	UDIMBlocks = BlocksIn;
	//	if (bBroadcast)
	//	{
	//		OnUDIMBlockChange.Broadcast(UDIMBlocks);
	//	}
	//}

	//const TArray<FUDIMBlock>& GetUDIMBlocks() const
	//{
	//	return UDIMBlocks;
	//}

	void SetDrawGrid(bool bDrawGridIn, bool bBroadcast = true)
	{
		bDrawGrid = bDrawGridIn;
		if (bBroadcast)
		{
			OnDrawGridChange.Broadcast(bDrawGrid);
		}
	}

	const bool GetDrawGrid() const
    {
		return bDrawGrid;
	}

	void SetDrawRulers(bool bDrawRulersIn, bool bBroadcast = true)
	{
		bDrawRulers = bDrawRulersIn;
		if (bBroadcast)
		{
			OnDrawRulersChange.Broadcast(bDrawRulers);
		}
	}

	const bool GetDrawRulers() const
    {
		return bDrawRulers;
	}


	//DECLARE_MULTICAST_DELEGATE_OneParam(FOnUDIMBlockChange, const TArray<FUDIMBlock>& UDIMBlocks);
	//FOnUDIMBlockChange OnUDIMBlockChange;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDrawGridChange, bool bDrawGrid);
	FOnDrawGridChange OnDrawGridChange;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDrawRulersChange, bool bDrawRulers);
	FOnDrawRulersChange OnDrawRulersChange;

protected:

	//UPROPERTY(Transient)
	//TArray<FUDIMBlock> UDIMBlocks;

	UPROPERTY(Transient)
	bool bDrawGrid;

	UPROPERTY(Transient)
	bool bDrawRulers;
};
