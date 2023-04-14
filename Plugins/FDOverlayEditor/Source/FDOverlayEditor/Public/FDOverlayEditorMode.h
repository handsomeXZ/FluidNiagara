// Copyright HandsomeCheese. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "Tools/UEdMode.h"
#include "ToolTargets/ToolTarget.h" // FToolTargetTypeRequirements
#include "GeometryBase.h"
#include "Context/FDOverlayAutoCalToolAPI.h"

#include "FDOverlayEditorMode.generated.h"

class UFDOverlayLive2DViewportAPI;
class UFDOverlayViewportButtonsAPI;
class UFDOverlaySelectionAPI;
class UFDOverlayMeshInput;
class UFDOverlayContextObject;
class UContextObjectStore;
class UWorld;
class UMeshOpPreviewWithBackgroundCompute;

PREDECLARE_GEOMETRY(class FDynamicMesh3);


UCLASS()
class FDOVERLAYEDITOR_API UFDOverlaySettingProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, Category = "Setting", meta = (DisplayName = "Output Type"))
	EAutoCalToolOutputType type = EAutoCalToolOutputType::Texture2DArray;

};

UCLASS(Transient)
class FDOVERLAYEDITOR_API UFDOverlayEditorMode : public UEdMode
{
	GENERATED_BODY()

public:

	const static FEditorModeID EM_FDOverlayEditorModeId;

	const static FString ToolName;

	UFDOverlayEditorMode();

	void RegisterTools();

	/** UEdMode interface */
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void CreateToolkit() override;
	virtual void ModeTick(float DeltaTime) override;
	virtual TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetModeCommands() const override;
	/**
	 * Called by an asset editor so that a created instance of the mode has all the data it needs on Enter() to initialize itself.
	 */
	static void InitializeAssetEditorContexts(UContextObjectStore& ContextStore,
		const TArray<TObjectPtr<UObject>>& AssetsIn, const TArray<FTransform>& TransformsIn,
		FEditorViewportClient& LivePreviewViewportClient, FEditorViewportClient& Live2DViewportClient,
		FAssetEditorModeManager& LivePreviewModeManager, UFDOverlayViewportButtonsAPI& ViewportButtonsAPI);

	// Asset management
	bool CanApplyChanges() const;
	void ApplyChanges();

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
public:

	UPROPERTY()
	TObjectPtr<UFDOverlaySettingProperties> SettingProperties = nullptr;
	
	UObject* GetSettingsObject();


protected:
	// Not sure whether we need these yet
	//virtual void BindCommands() override;
	virtual void OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool) override;
	virtual void OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool) override;

	void InitializeModeContexts();
	void InitializeTargets();
protected:
	
	bool bIsActive = false;
	FString DefaultToolIdentifier;

	/**
	* 存储原始输入对象，例如UStaticMesh指针。工具输入对象上的资产id是该数组的索引(以及与该数组1:1的索引)
	*/
	UPROPERTY()
	TArray<TObjectPtr<UObject>> OriginalObjectsToEdit;

	/**
	* 转换应该用于3d预览，1:1与OriginalObjectsToEdit和ToolTargets。
	*/
	TArray<FTransform> Transforms;

	TArray<TSharedPtr<UE::Geometry::FDynamicMesh3>> AppliedCanonicalMeshes;

	/**
	* Tool targets created from OriginalObjectsToEdit (and 1:1 with that array) that provide
	* us with dynamic meshes whose UV layers we unwrap.
	*/
	UPROPERTY()
	TArray<TObjectPtr<UToolTarget>> ToolTargets;

	TArray<TObjectPtr<UMeshOpPreviewWithBackgroundCompute>> AppliedPreviews;

	/**
	* Input objects we give to the tools, one per displayed UV layer. This includes pointers
	* to the applied meshes, but also contains the unwrapped mesh and preview. These should
	* not be assumed to be the same length as the asset arrays in case we someday do not
	* display exactly a single layer per asset.
	*/
	UPROPERTY()
	TArray<TObjectPtr<UFDOverlayMeshInput>> ToolInputObjects;

	// 这里主要是为了方便，避免在函数之间传递它。
	UPROPERTY()
	TObjectPtr<UWorld> LivePreviewWorld = nullptr;

	UPROPERTY()
	TObjectPtr<UFDOverlaySelectionAPI> SelectionAPI = nullptr;

	TArray<TWeakObjectPtr<UFDOverlayContextObject>> ContextsToUpdateOnToolEnd;
	TArray<TWeakObjectPtr<UFDOverlayContextObject>> ContextsToShutdown;

	// Holds references to PIE callbacks to handle logic when the PIE session starts & shuts down
	bool bPIEModeActive;
	FDelegateHandle BeginPIEDelegateHandle;
	FDelegateHandle EndPIEDelegateHandle;
	FDelegateHandle CancelPIEDelegateHandle;

	UMaterialInterface* DefaultBakeAppliedMaterialInterface = nullptr;
	UMaterialInterface* EmissiveBakeAppliedMaterialInterface = nullptr;
	UMaterialInterface* TranslucencyBakeAppliedMaterialInterface = nullptr;
	UMaterialInterface* TransitionBakeAppliedMaterialInterface = nullptr;
	UMaterialInterface* DefaultBakeUnwrapMaterialInterface = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<UInteractiveToolPropertySet>> PropertyObjectsToTick;
};
