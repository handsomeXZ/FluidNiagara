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

	// ��ȡ��ģʽ�Ĺ���Ŀ�����󡣲�����Ŀ�꾭����һ���Ĵ���������ת��Ϊ���߻�õ��������(��Ϊ��Щ��ҪԤ�������)��
	static const FToolTargetTypeRequirements& GetToolTargetRequirements();

	// ���������ｫ���Ŀɼ��Ը���Ϊpublic���Ա����ǿ����ڵ�������/ȡ����ťʱ�ӹ��߰��е�������
	// ���ǲ���ӹ�����Ϊ���ѣ���Ϊ���ǲ�ϣ�����Ժ�(�����)��ģʽ�ڲ���������һ��
	// ͬʱ�����ǲ�ȷ���Ƿ�Ҫ��UEdMode��Ϊpublic�������Ӱ����С�ĵ�����
	virtual void ActivateDefaultTool() override;
	// ��Ŀǰ�������ǻ����һ���֡���Ӧ����?
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
	* �洢ԭʼ�����������UStaticMeshָ�롣������������ϵ��ʲ�id�Ǹ����������(�Լ��������1:1������)
	*/
	UPROPERTY()
	TArray<TObjectPtr<UObject>> OriginalObjectsToEdit;

	/**
	* ת��Ӧ������3dԤ����1:1��OriginalObjectsToEdit��ToolTargets��
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

	// ������Ҫ��Ϊ�˷��㣬�����ں���֮�䴫������
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
