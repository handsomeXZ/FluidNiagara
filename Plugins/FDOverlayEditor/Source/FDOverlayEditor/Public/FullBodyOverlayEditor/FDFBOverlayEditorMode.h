

#pragma once

#include "CoreMinimal.h"
#include "Tools/UEdMode.h"
#include "ToolTargets/ToolTarget.h" // FToolTargetTypeRequirements
#include "FDFBOverlayEditorMode.generated.h"


class UFDAssistorLive2DViewportAPI;
class UFDAssistorViewportButtonsAPI;
class UFDAssistorSelectionAPI;
class UContextObjectStore;
class UFDAssistorContextObject
/**
 * This class provides an example of how to extend a UEdMode to add some simple tools
 * using the InteractiveTools framework. The various UEdMode input event handlers (see UEdMode.h)
 * forward events to a UEdModeInteractiveToolsContext instance, which
 * has all the logic for interacting with the InputRouter, ToolManager, etc.
 * The functions provided here are the minimum to get started inserting some custom behavior.
 * Take a look at the UEdMode markup for more extensibility options.
 */
UCLASS()
class FDASSISTOR_API UFDFBOverlayEditorMode : public UEdMode
{
	GENERATED_BODY()

public:
	const static FEditorModeID EM_FDFBOverlayEditorModeId;

	const static FString ToolName;

	UFDFBOverlayEditorMode();

	/** UEdMode interface */
	virtual void Enter() override;
	virtual void ActorSelectionChangeNotify() override;
	virtual void CreateToolkit() override;
	virtual TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetModeCommands() const override;


	/**
	 * Called by an asset editor so that a created instance of the mode has all the data it needs on Enter() to initialize itself.
	 */
	static void InitializeAssetEditorContexts(UContextObjectStore& ContextStore,
		const TArray<TObjectPtr<UObject>>& AssetsIn, const TArray<FTransform>& TransformsIn,
		FEditorViewportClient& LivePreviewViewportClient, FAssetEditorModeManager& LivePreviewModeManager,
		UFDAssistorViewportButtonsAPI& ViewportButtonsAPI, UFDAssistorLive2DViewportAPI& FDFBOverlayLive2DViewportAPI);


	//virtual void Render(IToolsContextRenderAPI* RenderAPI);
	//virtual void DrawHUD(FCanvas* Canvas, IToolsContextRenderAPI* RenderAPI);

	// Asset management
	// TODO: 还未实现
	bool CanApplyChanges() const;
	void ApplyChanges();

	// TODO: 还未实现
	void FocusLivePreviewCameraOnSelection();

	// 获取该模式的工具目标需求。产生的目标经过进一步的处理，将它们转化为工具获得的输入对象(因为这些需要预览网格等)。
	static const FToolTargetTypeRequirements& GetToolTargetRequirements();

	// 我们在这里将它的可见性更改为public，以便我们可以在单击接受/取消按钮时从工具包中调用它。
	// 我们不想加工具箱为好友，因为我们不希望它以后(意外地)与模式内部更纠缠在一起。
	// 同时，我们不确定是否要将UEdMode设为public。这就是影响最小的调整。
	virtual void ActivateDefaultTool() override;
	// 它目前根本不是基类的一部分……应该吗?
	virtual bool IsDefaultToolActive();
	bool IsActive() { return bIsActive; }

protected:
	void InitializeModeContexts();
	void InitializeTargets();

protected:
	
	bool bIsActive = false;
	

	/**
	* 存储原始输入对象，例如UStaticMesh指针。工具输入对象上的资产id是该数组的索引(以及与该数组1:1的索引)
	*/
	UPROPERTY()
	TArray<TObjectPtr<UObject>> OriginalObjectsToEdit;

	/**
	* 转换应该用于3d预览，1:1与OriginalObjectsToEdit和ToolTargets。
	*/
	TArray<FTransform> Transforms;

	// 这里主要是为了方便，避免在函数之间传递它。
	UPROPERTY()
	TObjectPtr<UWorld> LivePreviewWorld = nullptr;


	//// 用于将实时预览中的Render/ DrawHUD 调用转发给api对象。
	//TWeakObjectPtr<UEditorInteractiveToolsContext> LivePreviewITC;

	UPROPERTY()
	TObjectPtr<UFDAssistorSelectionAPI> SelectionAPI = nullptr;

	TArray<TWeakObjectPtr<UFDAssistorContextObject>> ContextsToUpdateOnToolEnd;

	// Holds references to PIE callbacks to handle logic when the PIE session starts & shuts down
	bool bPIEModeActive;
	FDelegateHandle BeginPIEDelegateHandle;
	FDelegateHandle EndPIEDelegateHandle;
	FDelegateHandle CancelPIEDelegateHandle;
	
};
