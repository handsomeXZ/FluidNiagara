// Copyright HandsomeCheese. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "FDOverlayContextObject.h"
#include "FDOverlayAutoCalToolAPI.generated.h"

UENUM()
enum class EAutoCalToolOutputType
{
	Texture2DArray,
	Texture2Ds
};
/**
 * Allows tools to interact with the AutoCalTool
 */
UCLASS()
class FDOVERLAYEDITOR_API UFDOverlayAutoCalToolAPI : public UFDOverlayContextObject
{
	GENERATED_BODY()
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnOutputTypeChange, EAutoCalToolOutputType type);
	FOnOutputTypeChange OnOutputTypeChange;
	void SetOutputType(EAutoCalToolOutputType type)
	{
		OnOutputTypeChange.Broadcast(type);
	}

};
