// Copyright HandsomeCheese. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "DynamicMesh/DynamicMeshAttributeSet.h" // FDynamicMeshUVOverlay
#include "ToolTargets/ToolTarget.h"
#include "VectorTypes.h"
#include "TargetInterfaces\MaterialProvider.h"
#include "GeometryBase.h"

#include "FDOverlayMeshInput.generated.h"

class UMaterialInterface;
class UMeshOpPreviewWithBackgroundCompute;
class UMeshElementsVisualizer;
PREDECLARE_GEOMETRY(class FDynamicMesh3);

/**
* 由UV编辑器工具操作的资产所需信息的包。它包括一个UV展开网格，一个应用了UV层的网格，以及每个网格的背景操作兼容预览。它还提供了方便的方法，用于从其中一个表示中更新所有表示，在可能的情况下使用“快速更新”代码路径。
* 这个工具目标与通常的有点不同，因为它不是由工具目标管理器创建的，因此没有附带的工厂。
* 相反，它是由模式创建的，因为模式可以访问需要创建预览的世界。
* 是否应该继承UToolTarget是有争议的。
 */
UCLASS()
class FDOVERLAYEDITOR_API UFDOverlayMeshInput : public UToolTarget
{
	GENERATED_BODY()

public:

	/** 
	 * 网格表示未包装的UV层。如果 UnwrapPreview 是通过后台操作改变的，那么这个网格可以在参数改变时重新启动一个操作。
	 * 一旦更改完成，这个网格应该被更新(即，它是“canonical”展开网格，尽管最终的UV层真相是在 AppliedCanonical 的UV中)。
	 */
	TSharedPtr<UE::Geometry::FDynamicMesh3> UnwrapCanonical;

	/**
	 * 预览未包装的UV层，适合由后台操作操作。
	 */
	UPROPERTY()
	TObjectPtr<UMeshOpPreviewWithBackgroundCompute> UnwrapPreview = nullptr;

	// 注意，无论是 UnwrapCanonical 和 UnwrapPreview，除了有 vert 的位置，代表一个UV层，也有一个主UV覆盖，代表同一层，使它有可能有一天纹理的展开。
	// UV叠加与 AppliedCanonical 和 AppliedPreview 中的UV叠加只在元素的父指针上有所不同，因为没有任何元素指向同一个顶点。元素id和三角形是一样的。

	/** 
	 * 一个三维网格与UV层应用。这是将在更改应用程序中返回的Canonical结果。它也可以用来重置后台操作，可能会操作 AppliedPreview。
	 */
	TSharedPtr<UE::Geometry::FDynamicMesh3> AppliedCanonical;

	/**
	 * 资产的3d预览与UV层更新，适合与背景操作使用。
	 */
	UPROPERTY()
	TObjectPtr<UMeshOpPreviewWithBackgroundCompute> AppliedPreview;

	/**
	 * 可选: 线框跟踪 unwrap mesh preview。如果设置了，它将在类更新展开预览时得到更新，并在Shutdown()上销毁。
	 * TODO: 我们应该有一个更新线框的快速路径…
	 */
	UPROPERTY()
	TObjectPtr<UMeshElementsVisualizer> WireframeDisplay = nullptr;


	// OnCanonicalModified 广播信息。如果有一天我们需要的话，这会增加更多的信息。
	struct FCanonicalModifiedInfo
	{
		// 目前还不清楚我们是否需要其中任何一个。
		// bapplieoverlaychanged 似乎在发出更改时总是为真。如果我们在 applied canonical 中编辑一些其他层而不显示它，bUnwrapMeshShapeChanged似乎只会为假。这种情况很少见，在大多数情况下，只做一个完整的更新可能会更安全(即仍然假设unwrap发生了变化)。
		// 所以现在我们不使用它们。
		// bool bUnwrapMeshShapeChanged = true;
		// bool bapplieoverlaychanged = true;

		// 这只有在我们在外部编辑网格后才会发生，因为UV编辑器不会改变网格的形状。
		bool bAppliedMeshShapeChanged = false;
	};

	/**
	 * 当 canonical unwrap 或 applied meshes 改变时进行广播。这将通过该类中的实用程序函数进行广播，从而更新这些网格( UpdateUnwrapCanonicalOverlayFromPositions 除外，后者通常紧随其后进行更完整的更新)。
	 * 如果这些实用函数没有使用，而客户端更新了其中一个网格，客户端应该广播更改本身，以便更新重要的相关信息(例如，以便模式可以将这些对象标记为已修改)。
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnObjectModified, UFDOverlayMeshInput* InputObject, const FCanonicalModifiedInfo&);
	FOnObjectModified OnCanonicalModified;

	// Additional needed information
	TObjectPtr<UToolTarget> SourceTarget = nullptr;
	int32 AssetID = -1;
	int32 UVLayerIndex = 0;
	int32 MaterialIndex = -1;
	int32 MaxMaterialIndex = -1;
	int32 RenderMode = 0;

	// 用于生成和烘烤展开的映射。
	TFunction<FVector3d(const FVector2f&)> UVToVertPosition;
	TFunction<FVector2f(const FVector3d&)> VertPositionToUV;

	bool InitializeMeshes(UToolTarget* Target, TSharedPtr<UE::Geometry::FDynamicMesh3> AppliedCanonicalIn,
		UMeshOpPreviewWithBackgroundCompute* AppliedPreviewIn, int32 AssetIDIn, int32 UVLayerIndexIn, UWorld* UnwrapWorld,
		UWorld* LivePreviewWorldIn, UMaterialInterface* DefaultBakeAppliedMaterialInterfaceIn, UMaterialInterface* EmissiveBakeAppliedMaterialInterfaceIn,
		UMaterialInterface* TranslucencyBakeUnwrapMaterialInterfaceIn, UMaterialInterface* TransitionBakeAppliedMaterialInterfaceIn, UMaterialInterface* DefaultBakeUnwrapMaterialInterfaceIn, TFunction<FVector3d(const FVector2f&)> UVToVertPositionFuncIn,
		TFunction<FVector2f(const FVector3d&)> VertPositionToUVFuncIn);

	void ShowToMesh(const UTexture2DArray* BakedSource);

	void Shutdown();

	void ChangeDynamicMaterialDisplayChannel(uint8 Channel, bool bIsDisplay);
	void ChangeDynamicMaterialDisplayRender(uint8 RenderMode);
	void SwitchDynamicMaterialDisplayIDMode(uint8 style = 0);
	void AddDynamicMaterialDisplayID();
	void SubDynamicMaterialDisplayID();

	void FindCurrentRenderMaterial(UMaterialInstanceDynamic*& MI, FComponentMaterialSet*& MSet);

	void UpdateOpacity();
	
private:
	void GenerateUVUnwrapMesh(const UE::Geometry::FDynamicMeshUVOverlay& UVOverlay, UE::Geometry::FDynamicMesh3& UnwrapMeshOut,
		TFunctionRef<FVector3d(const FVector2f&)> UVToVertPosition);

private:
	// Material
	UMaterialInterface* DefaultBakeAppliedMaterialInterface;
	UMaterialInterface* EmissiveBakeAppliedMaterialInterface;
	UMaterialInterface* TranslucencyBakeUnwrapMaterialInterface;
	UMaterialInterface* TransitionBakeAppliedMaterialInterface;
	UMaterialInterface* DefaultBakeUnwrapMaterialInterface;
	FComponentMaterialSet PrevMaterialSet;
	FComponentMaterialSet DefaultBakeAppliedMaterialSet;
	FComponentMaterialSet EmissiveBakeAppliedMaterialSet;
	FComponentMaterialSet TranslucencyBakeAppliedMaterialSet;
	FComponentMaterialSet TransitionBakeAppliedMaterialSet;
	FComponentMaterialSet BakeUnwrapMaterialSet;
};