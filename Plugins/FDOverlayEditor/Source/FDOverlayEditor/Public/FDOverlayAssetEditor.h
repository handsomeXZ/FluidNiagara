// Copyright HandsomeCheese. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tools/UAssetEditor.h"
#include "FDOverlayAssetEditor.generated.h"

/**
 * 
 */
UCLASS()
class FDOVERLAYEDITOR_API UFDOverlayAssetEditor : public UAssetEditor
{
	GENERATED_BODY()
public:
	void Initialize(const TArray<TObjectPtr<UObject>>& InObjects);

	/** 返回资产编辑器的实例界面，这样它的窗口就可以被聚焦，比如说。*/
	IAssetEditorInstance* GetInstanceInterface();

	// UAssetEditor overrides
	void GetObjectsToEdit(TArray<UObject*>& OutObjects) override; // 必须覆写，原有实现为空函数，这会触发后面的断言
	virtual TSharedPtr<FBaseAssetToolkit> CreateToolkit() override;


protected:

	UPROPERTY()
		TArray<TObjectPtr<UObject>> OriginalObjectsToEdit;


};
