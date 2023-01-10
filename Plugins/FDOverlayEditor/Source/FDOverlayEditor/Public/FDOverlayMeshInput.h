// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "DynamicMesh/DynamicMeshAttributeSet.h" // FDynamicMeshUVOverlay
#include "ToolTargets/ToolTarget.h"
#include "VectorTypes.h"

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

	// 用于生成和烘烤展开的映射。
	TFunction<FVector3d(const FVector2f&)> UVToVertPosition;
	TFunction<FVector2f(const FVector3d&)> VertPositionToUV;

	bool InitializeMeshes(UToolTarget* Target, TSharedPtr<UE::Geometry::FDynamicMesh3> AppliedCanonicalIn,
		UMeshOpPreviewWithBackgroundCompute* AppliedPreviewIn, int32 AssetIDIn, int32 UVLayerIndexIn, 
		UWorld* UnwrapWorld, UWorld* LivePreviewWorld, UMaterialInterface* WorkingMaterialIn, 
		TFunction<FVector3d(const FVector2f&)> UVToVertPositionFuncIn, 
		TFunction<FVector2f(const FVector3d&)> VertPositionToUVFuncIn);

	void Shutdown() {};

	//注意以下便利函数:
	// 1。为ChangedVids / changnedelementids / ChangedConnectivityTids 传递 nullptr 意味着所有的 vids / elements / tids 都需要分别更新。
	//		传递 UFDOverlayMeshInput::NONE_CHANGED_ARG 相当于传递一个指向空数组的指针，意味着没有任何变化。否则，只有指向数组中的 vids / elements / tids 在更新的网格中迭代。
	//		虽然传递这些可以使更新更快，但请注意，未能包含相关的 tids / vids / elements 会使输入对象处于无效状态或命中检查，例如，如果一个三角形指向一个未添加的新vid。
	// 
	// 2。ChangedVids / ChangedElementIDs 允许有新的元素，因为分割uv是很自然的。然而，ChangedConnectivityTids 必须没有新的Tids，因为Tids形成了我们的展开网格和原始层之间的对应关系。
	//		还要注意的是，如果你正在收集基于三角形的更改的 vids / elements，你应该从 post-change mesh/overlay 中收集它们，这样你就不会错过任何添加的元素，因为删除的元素是通过更改的三角连接捕获的。
	// 
	// 3。FastRenderUpdateTids是一个可选的三角形列表，它的渲染数据需要更新，如果渲染缓冲区被正确分割，它可以允许更快的预览更新。如果提供，它应该是ChangedConnectivityTids的 superset，并且应该包含ChangedVids的 one-ring triangles。
	// 
	// 4。如果更新预览对象，请注意函数不会尝试取消任何活动计算，因此活动计算完成后可能会重置。

	//// 可以作为 ChangedVids/ChangedElements/ChangedConnectivityTids 参数传入，相当于传递一个指向空数组的指针。
	//static const TArray<int32>* const NONE_CHANGED_ARG;

	///**
	// * 更新 UnwrapPreview UV Overlay 从 UnwrapPreview vert 位置。为 positions 和 UVs 发出一个 NotifyDeferredEditCompleted。
	// */
	//void UpdateUnwrapPreviewOverlayFromPositions(const TArray<int32>* ChangedVids = nullptr, 
	//	const TArray<int32>* ChangedConnectivityTids = nullptr, const TArray<int32>* FastRenderUpdateTids = nullptr) {};

	///**
	// * 从 UnwrapCanonical vert 位置 更新 UnwrapCanonical UV Overlay。为 positions 和 UVs 发出一个 NotifyDeferredEditCompleted。
	// * 不广播，因为预期这个调用之后会有另一个调用来更新输入对象的其余部分。
	// */
	//void UpdateUnwrapCanonicalOverlayFromPositions(const TArray<int32>* ChangedVids = nullptr, 
	//	const TArray<int32>* ChangedConnectivityTids = nullptr) {};

	///**
	// * 从UnwrapPreview更新AppliedPreview，不更新非预览网格。当我们只关心更新可见项时，对于在拖拽期间进行更新非常有用。
	// * 假设UnwrapPreview中的覆盖层已经更新。
	// */
	//void UpdateAppliedPreviewFromUnwrapPreview(const TArray<int32>* ChangedVids = nullptr, 
	//	const TArray<int32>* ChangedConnectivityTids = nullptr, const TArray<int32>* FastRenderUpdateTids = nullptr) {};

	///**
	// * 只更新来自AppliedPreview的UnwrapPreview，而不更新非预览网格。当我们只关心更新可见项时，对于临时更新非常有用。
	// */
	//void UpdateUnwrapPreviewFromAppliedPreview(
	//	const TArray<int32>* ChangedElementIDs = nullptr, const TArray<int32>* ChangedConnectivityTids = nullptr, 
	//	const TArray<int32>* FastRenderUpdateTids = nullptr) {};

	///**
	// * 更新非预览网格。例如，在完成拖拽后更新 canonical objects 时很有用。
	// */
	//void UpdateCanonicalFromPreviews(const TArray<int32>* ChangedVids = nullptr, 
	//	const TArray<int32>* ChangedConnectivityTids = nullptr, bool bBroadcast = true) {};

	///**
	// * 从它们的 canonical对应物更新预览网格。主要用于重置预览。
	// */
	//void UpdatePreviewsFromCanonical(const TArray<int32>* ChangedVids = nullptr, const TArray<int32>* ChangedConnectivityTids = nullptr,
	//	const TArray<int32>* FastRenderUpdateTids = nullptr) {};

	///**
	// * 使用UnwrapPreview中的网格更新其他网格。假设UnwrapPreview中的覆盖层已更新。
	// * 用于在 applied preview 未与 unwrap preview 同步更新时更新所有内容(否则将使用UpdateCanonicalFromPreviews)。
	// */
	//void UpdateAllFromUnwrapPreview(const TArray<int32>* ChangedVids = nullptr, const TArray<int32>* ChangedConnectivityTids = nullptr, 
	//	const TArray<int32>* FastRenderUpdateTids = nullptr, bool bBroadcast = true) {};

	///**
	// * 使用UnwrapCanonical中的网格更新其他网格。在立即应用直接对 unwrap mesh 进行操作时非常有用。
	// */
	//void UpdateAllFromUnwrapCanonical(const TArray<int32>* ChangedVids = nullptr, const TArray<int32>* ChangedConnectivityTids = nullptr, 
	//	const TArray<int32>* FastRenderUpdateTids = nullptr, bool bBroadcast = true) {};

	///**
	// * 使用AppliedCanonical中的网格更新其他网格。从“original”数据重置集合。
	// */
	//void UpdateAllFromAppliedCanonical(const TArray<int32>* ChangedVids = nullptr, const TArray<int32>* ChangedConnectivityTids = nullptr, 
	//	const TArray<int32>* FastRenderUpdateTids = nullptr, bool bBroadcast = true) {};

	///**
	// * 在实时预览中使用UV覆盖更新其他网格。
	// */
	//void UpdateAllFromAppliedPreview(const TArray<int32>* ChangedElementIDs = nullptr, 
	//	const TArray<int32>* ChangedConnectivityTids = nullptr, const TArray<int32>* FastRenderUpdateTids = nullptr, bool bBroadcast = true) {};

	///**
	// * 使用网格更改中存储的 三角形/顶点 来更新 canonical unwrap 的所有内容。
	// * 假设更改已经应用到规范的展开。
	// */
	//void UpdateFromCanonicalUnwrapUsingMeshChange(const UE::Geometry::FDynamicMeshChange& UnwrapCanonicalMeshChange, bool bBroadcast = true) {};

	///**
	// * 将展开网格中的vid转换为应用网格中的对应vid(即，与展开顶点对应的元素的父顶点)。
	// */
	//int32 UnwrapVidToAppliedVid(int32 UnwrapVid) { return 0; };

	///**
	// * 获得对应于给定应用vid的展开vid。如果顶点是一个接缝顶点，这些将是多个，因此有多个与之关联的UV元素。
	// */
	//void AppliedVidToUnwrapVids(int32 AppliedVid, TArray<int32>& UnwrapVidsOut) {};

	///**
	// * 返回底层源ToolTarget是否仍然有效。
	// * 这是分开的IsValid，它也检查ToolTarget，
	// * 以防我们只对ToolTarget的状态感兴趣。
	//*/
	//virtual bool IsToolTargetValid() const { return false; };

	///**
	// * 返回规范网格和预览网格是否仍然有效。
	// * 这是从IsValid中分离出来的，以防我们只对规范和预览网格的状态感兴趣。
	// */
	//virtual bool AreMeshesValid() const { return false; };

	//// UToolTarget
	//virtual bool IsValid() const override {return false;};

private:
	void GenerateUVUnwrapMesh(const UE::Geometry::FDynamicMeshUVOverlay& UVOverlay, UE::Geometry::FDynamicMesh3& UnwrapMeshOut,
		TFunctionRef<FVector3d(const FVector2f&)> UVToVertPosition);
};