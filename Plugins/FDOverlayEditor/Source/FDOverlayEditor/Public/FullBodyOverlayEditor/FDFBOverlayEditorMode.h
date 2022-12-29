

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
	// TODO: ��δʵ��
	bool CanApplyChanges() const;
	void ApplyChanges();

	// TODO: ��δʵ��
	void FocusLivePreviewCameraOnSelection();

	// ��ȡ��ģʽ�Ĺ���Ŀ�����󡣲�����Ŀ�꾭����һ���Ĵ���������ת��Ϊ���߻�õ��������(��Ϊ��Щ��ҪԤ�������)��
	static const FToolTargetTypeRequirements& GetToolTargetRequirements();

	// ���������ｫ���Ŀɼ��Ը���Ϊpublic���Ա����ǿ����ڵ�������/ȡ����ťʱ�ӹ��߰��е�������
	// ���ǲ���ӹ�����Ϊ���ѣ���Ϊ���ǲ�ϣ�����Ժ�(�����)��ģʽ�ڲ���������һ��
	// ͬʱ�����ǲ�ȷ���Ƿ�Ҫ��UEdMode��Ϊpublic�������Ӱ����С�ĵ�����
	virtual void ActivateDefaultTool() override;
	// ��Ŀǰ�������ǻ����һ���֡���Ӧ����?
	virtual bool IsDefaultToolActive();
	bool IsActive() { return bIsActive; }

protected:
	void InitializeModeContexts();
	void InitializeTargets();

protected:
	
	bool bIsActive = false;
	

	/**
	* �洢ԭʼ�����������UStaticMeshָ�롣������������ϵ��ʲ�id�Ǹ����������(�Լ��������1:1������)
	*/
	UPROPERTY()
	TArray<TObjectPtr<UObject>> OriginalObjectsToEdit;

	/**
	* ת��Ӧ������3dԤ����1:1��OriginalObjectsToEdit��ToolTargets��
	*/
	TArray<FTransform> Transforms;

	// ������Ҫ��Ϊ�˷��㣬�����ں���֮�䴫������
	UPROPERTY()
	TObjectPtr<UWorld> LivePreviewWorld = nullptr;


	//// ���ڽ�ʵʱԤ���е�Render/ DrawHUD ����ת����api����
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
