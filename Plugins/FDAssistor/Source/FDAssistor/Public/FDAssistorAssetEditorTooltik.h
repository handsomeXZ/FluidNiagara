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

	// 这些都与 3D "live preview" viewport 有关。2d unwrap viewport 的东西存储在FBaseAssetToolkit::ViewportTabContent,
	// ViewportDelegate, ViewportClient 中
	TSharedPtr<FEditorViewportClient> Live3DPreviewViewportClient;

	UFDAssistorViewportButtonsAPI* ViewportButtonsAPI = nullptr;
	UFDAssistorLive2DViewportAPI* FDAssistorLive2DViewportAPI = nullptr;
	
	TSharedPtr<FWorkspaceItem> FDAssistorEditorMenuCategory;

};