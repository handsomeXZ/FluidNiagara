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

	/** �����ʲ��༭����ʵ�����棬�������Ĵ��ھͿ��Ա��۽�������˵��*/
	IAssetEditorInstance* GetInstanceInterface();

	// UAssetEditor overrides
	void GetObjectsToEdit(TArray<UObject*>& OutObjects) override; // ���븲д��ԭ��ʵ��Ϊ�պ�������ᴥ������Ķ���
	virtual TSharedPtr<FBaseAssetToolkit> CreateToolkit() override;


protected:

	UPROPERTY()
		TArray<TObjectPtr<UObject>> OriginalObjectsToEdit;


};
