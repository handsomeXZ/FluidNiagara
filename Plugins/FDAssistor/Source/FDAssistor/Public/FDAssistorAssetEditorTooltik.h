#pragma once

#include "CoreMinimal.h"

#include "Tools/BaseAssetToolkit.h"



class FDASSISTOR_API FFDAssistorAssetEditorToolkit : public FBaseAssetToolkit
{
public:
	FFDAssistorAssetEditorToolkit(UAssetEditor* InOwningAssetEditor);
	virtual ~FFDAssistorAssetEditorToolkit();



protected:


	// FAssetEditorToolkit
	virtual void PostInitAssetEditor() override;

protected:

	// ��Щ���� 3D "live preview" viewport �йء�2d unwrap viewport �Ķ����洢��FBaseAssetToolkit::ViewportTabContent,
	// ViewportDelegate, ViewportClient ��
	TSharedPtr<FEditorViewportClient> Live3DPreviewViewportClient;

	UFDAssistorViewportButtonsAPI* ViewportButtonsAPI = nullptr;
	UFDAssistorLive2DViewportAPI* FDAssistorLive2DViewportAPI = nullptr;
	
	TSharedPtr<FWorkspaceItem> FDAssistorEditorMenuCategory;

};