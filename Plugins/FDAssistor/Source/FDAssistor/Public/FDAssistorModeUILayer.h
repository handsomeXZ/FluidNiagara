
#pragma once

#include "Toolkits/AssetEditorModeUILayer.h"
#include "FDAssistorModeUILayer.generated.h"

/** Interchange layer to manage built in tab locations within the editor's layout. **/
UCLASS()
class UFDAssistorEditorUISubsystem : public UAssetEditorUISubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void RegisterLayoutExtensions(FLayoutExtender& Extender) override;
};

/** Handles the hosting of additional toolkits, such as the mode toolkit, within the UVEditor's toolkit. **/
class FFDAssistorEditorModeUILayer : public FAssetEditorModeUILayer
{
public:
	FFDAssistorEditorModeUILayer(const IToolkitHost* InToolkitHost);
	void OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit) override;
	void OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit) override;

	void SetModeMenuCategory(TSharedPtr<FWorkspaceItem> MenuCategoryIn);
	TSharedPtr<FWorkspaceItem> GetModeMenuCategory() const override;

protected:
	TSharedPtr<FWorkspaceItem> FDAssistorEditorMenuCategory;

};
