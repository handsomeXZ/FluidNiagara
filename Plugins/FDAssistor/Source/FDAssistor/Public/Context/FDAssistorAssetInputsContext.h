// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FDAssistorContextObject.h"

#include "FDAssistorAssetInputsContext.generated.h"

/**
 * Simple context object used by UV asset editor to pass inputs down to the UV editor mode.
 */
UCLASS()
class UVEDITORTOOLS_API UFDAssistorAssetInputsContext : public UFDAssistorContextObject
{
	GENERATED_BODY()
public:

	void Initialize(TArray<TObjectPtr<UObject>> AssetsIn, TArray<FTransform> TransformsIn)
	{
		Assets = MoveTemp(AssetsIn);
		Transforms = MoveTemp(TransformsIn);
		ensure(Transforms.Num() == Assets.Num());
	}
	 
	void GetAssetInputs(TArray<TObjectPtr<UObject>>& AssetsOut, TArray<FTransform>& TransformsOut) const
	{
		AssetsOut = Assets;
		TransformsOut = Transforms;
	}

protected:
	TArray<TObjectPtr<UObject>> Assets;
	TArray<FTransform> Transforms;
};