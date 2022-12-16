// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InteractiveToolBuilder.h"
#include "BaseTools/SingleClickTool.h"
#include "FluidDynamicTooltikModeSimpleTool.generated.h"

/**
 * Builder for UFluidDynamicTooltikModeSimpleTool
 */
UCLASS()
class FLUIDDYNAMICTOOLTIKMODE_API UFluidDynamicTooltikModeSimpleToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override { return true; }
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};



/**
 * Settings UObject for UFluidDynamicTooltikModeSimpleTool. This UClass inherits from UInteractiveToolPropertySet,
 * which provides an OnModified delegate that the Tool will listen to for changes in property values.
 */
UCLASS(Transient)
class FLUIDDYNAMICTOOLTIKMODE_API UFluidDynamicTooltikModeSimpleToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()
public:
	UFluidDynamicTooltikModeSimpleToolProperties();

	/** If enabled, dialog should display extended information about the actor clicked on. Otherwise, only basic info will be shown. */
	UPROPERTY(EditAnywhere, Category = Options, meta = (DisplayName = "Show Extended Info"))
	bool ShowExtendedInfo;
};




/**
 * UFluidDynamicTooltikModeSimpleTool is an example Tool that opens a message box displaying info about an actor that the user
 * clicks left mouse button. All the action is in the ::OnClicked handler.
 */
UCLASS()
class FLUIDDYNAMICTOOLTIKMODE_API UFluidDynamicTooltikModeSimpleTool : public USingleClickTool
{
	GENERATED_BODY()

public:
	UFluidDynamicTooltikModeSimpleTool();

	virtual void SetWorld(UWorld* World);

	virtual void Setup() override;

	virtual void OnClicked(const FInputDeviceRay& ClickPos);


protected:
	UPROPERTY()
	TObjectPtr<UFluidDynamicTooltikModeSimpleToolProperties> Properties;


protected:
	/** target World we will raycast into to find actors */
	UWorld* TargetWorld;
};