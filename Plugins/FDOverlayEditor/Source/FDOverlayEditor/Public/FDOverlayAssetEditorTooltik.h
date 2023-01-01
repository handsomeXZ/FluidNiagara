#pragma once

#include "CoreMinimal.h"

#include "Tools/BaseAssetToolkit.h"

class FAdvancedPreviewScene;
class UInputRouter;
class SDockTab;
class SWidget;
class UFDOverlayLive2DViewportAPI;
class UFDOverlayViewportButtonsAPI;
class FFDOverlayEditorModeUILayer;


class FDOVERLAYEDITOR_API FFDOverlayAssetEditorToolkit : public FBaseAssetToolkit
{
public:
	FFDOverlayAssetEditorToolkit(UAssetEditor* InOwningAssetEditor);
	virtual ~FFDOverlayAssetEditorToolkit();

	static const FName Live3DPreviewTabID;

	// FBaseAssetToolkit
	virtual void CreateWidgets() override;

	// FAssetEditorToolkit
	virtual FText GetToolkitName() const override;	/** Returns the localized name of this toolkit */
	virtual FName GetToolkitFName() const override;	/** Returns the invariant name of this toolkit type */
	virtual FText GetBaseToolkitName() const override;	/** Returns the localized name of this toolkit type (typically just "<ClassName> editor") */
	virtual FText GetToolkitToolTipText() const override;
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;	/** Register tabs that this toolkit can spawn with the TabManager */
	virtual void OnClose() override;
	// �����������󽫸���С������ӵ��ӿ�ʱ���á���û���ӿڵ�����²���ء�
	virtual void AddViewportOverlayWidget(TSharedRef<SWidget> InViewportOverlayWidget) override;
	virtual void RemoveViewportOverlayWidget(TSharedRef<SWidget> InViewportOverlayWidget) override;
	void OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit) override;
	void OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit) override;

protected:
	TSharedRef<SDockTab> SpawnTab_LivePreview(const FSpawnTabArgs& Args);

	// FBaseAssetToolkit
	virtual TSharedPtr<FEditorViewportClient> CreateEditorViewportClient() const override; // ��FBaseAssetToolkit::CreateWidgets���������ViewportClient��������ֻ�������Լ���viewportί����ʹ�á�
	virtual AssetEditorViewportFactoryFunction GetViewportDelegate() override; // ��FBaseAssetToolkit::CreateWidgets���á�ί�е���·������FAssetEditorToolkit::InitAssetEditor��FBaseAssetToolkit::SpawnTab_Viewport��
	// FAssetEditorToolkit
	virtual void CreateEditorModeManager() override; // ��FBaseAssetToolkit::CreateWidgets���á�
	virtual void PostInitAssetEditor() override;
	//virtual const FSlateBrush* GetDefaultTabIcon() const override;
	virtual FLinearColor GetDefaultTabColor() const override;

protected:

	/** Scene in which the 2D unwrapped uv meshes live. */
	TUniquePtr<FPreviewScene> UnwrapScene;

	/** Scene in which the 3D preview meshes of the assets live. */
	TUniquePtr<FAdvancedPreviewScene> Live3DPreviewScene;

	// ��Щ���� 3D "live preview" viewport �йء�2d unwrap viewport �Ķ����洢��FBaseAssetToolkit::ViewportTabContent,
	// ViewportDelegate, ViewportClient ��
	TSharedPtr<class FEditorViewportTabContent> LivePreviewTabContent;
	AssetEditorViewportFactoryFunction Live3DPreviewViewportDelegate;
	TSharedPtr<FEditorViewportClient> Live3DPreviewViewportClient;

	TSharedPtr<FAssetEditorModeManager> Live3DPreviewEditorModeManager;
	TObjectPtr<UInputRouter> Live3DPreviewInputRouter = nullptr;

	TWeakPtr<SEditorViewport> FDOverlay2DViewport;
	UFDOverlayViewportButtonsAPI* ViewportButtonsAPI = nullptr;

	UFDOverlayLive2DViewportAPI* FDOverlayLive2DViewportAPI = nullptr;

	TSharedPtr<FFDOverlayEditorModeUILayer> ModeUILayer;
	TSharedPtr<FWorkspaceItem> FDOverlayMenuCategory;
};