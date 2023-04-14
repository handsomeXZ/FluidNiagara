// Copyright HandsomeCheese. All Rights Reserved.
#pragma once

#include "Toolkits/AssetEditorModeUILayer.h"
#include "FDOverlayModeUILayer.generated.h"

/** Interchange layer to manage built in tab locations within the editor's layout. **/
UCLASS()
class UFDOverlayEditorUISubsystem : public UAssetEditorUISubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void RegisterLayoutExtensions(FLayoutExtender& Extender) override;
};

/** Handles the hosting of additional toolkits, such as the mode toolkit, within the UVEditor's toolkit. **/
class FFDOverlayEditorModeUILayer : public FAssetEditorModeUILayer
{
public:
	FFDOverlayEditorModeUILayer(const IToolkitHost* InToolkitHost);

	//  由SLevelEditor调用，通知工具箱正在托管一个新的工具包。
	void OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit) override;
	void OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit) override;

	 void SetModeMenuCategory(TSharedPtr<FWorkspaceItem> MenuCategoryIn);
	 // TODO: What is this actually used for?
	 TSharedPtr<FWorkspaceItem> GetModeMenuCategory() const override;

protected:
	 TSharedPtr<FWorkspaceItem> FDOverlayEditorMenuCategory;

};
