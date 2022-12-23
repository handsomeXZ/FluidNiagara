// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tools/UAssetEditor.h"
#include "FDAssistorAssetEditor.generated.h"

/**
 * 
 */
UCLASS()
class FDASSISTOR_API UFDAssistorAssetEditor : public UAssetEditor
{
	GENERATED_BODY()
public:
	void Initialize(const TArray<TObjectPtr<UObject>>& InObjects);

	/** Returns the asset editor instance interface, so that its window can be focused, for example. */
	IAssetEditorInstance* GetInstanceInterface();

	virtual TSharedPtr<FBaseAssetToolkit> CreateToolkit() override;


protected:

	UPROPERTY()
		TArray<TObjectPtr<UObject>> OriginalObjectsToEdit;


};
