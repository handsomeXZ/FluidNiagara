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
* ��UV�༭�����߲������ʲ�������Ϣ�İ���������һ��UVչ������һ��Ӧ����UV��������Լ�ÿ������ı�����������Ԥ���������ṩ�˷���ķ��������ڴ�����һ����ʾ�и������б�ʾ���ڿ��ܵ������ʹ�á����ٸ��¡�����·����
* �������Ŀ����ͨ�����е㲻ͬ����Ϊ�������ɹ���Ŀ������������ģ����û�и����Ĺ�����
* �෴��������ģʽ�����ģ���Ϊģʽ���Է�����Ҫ����Ԥ�������硣
* �Ƿ�Ӧ�ü̳�UToolTarget��������ġ�
 */
UCLASS()
class FDOVERLAYEDITOR_API UFDOverlayMeshInput : public UToolTarget
{
	GENERATED_BODY()

public:

	/** 
	 * �����ʾδ��װ��UV�㡣��� UnwrapPreview ��ͨ����̨�����ı�ģ���ô�����������ڲ����ı�ʱ��������һ��������
	 * һ��������ɣ��������Ӧ�ñ�����(�������ǡ�canonical��չ�����񣬾������յ�UV���������� AppliedCanonical ��UV��)��
	 */
	TSharedPtr<UE::Geometry::FDynamicMesh3> UnwrapCanonical;

	/**
	 * Ԥ��δ��װ��UV�㣬�ʺ��ɺ�̨����������
	 */
	UPROPERTY()
	TObjectPtr<UMeshOpPreviewWithBackgroundCompute> UnwrapPreview = nullptr;

	// ע�⣬������ UnwrapCanonical �� UnwrapPreview�������� vert ��λ�ã�����һ��UV�㣬Ҳ��һ����UV���ǣ�����ͬһ�㣬ʹ���п�����һ�������չ����
	// UV������ AppliedCanonical �� AppliedPreview �е�UV����ֻ��Ԫ�صĸ�ָ����������ͬ����Ϊû���κ�Ԫ��ָ��ͬһ�����㡣Ԫ��id����������һ���ġ�

	/** 
	 * һ����ά������UV��Ӧ�á����ǽ��ڸ���Ӧ�ó����з��ص�Canonical�������Ҳ�����������ú�̨���������ܻ���� AppliedPreview��
	 */
	TSharedPtr<UE::Geometry::FDynamicMesh3> AppliedCanonical;

	/**
	 * �ʲ���3dԤ����UV����£��ʺ��뱳������ʹ�á�
	 */
	UPROPERTY()
	TObjectPtr<UMeshOpPreviewWithBackgroundCompute> AppliedPreview;

	/**
	 * ��ѡ: �߿���� unwrap mesh preview����������ˣ������������չ��Ԥ��ʱ�õ����£�����Shutdown()�����١�
	 * TODO: ����Ӧ����һ�������߿�Ŀ���·����
	 */
	UPROPERTY()
	TObjectPtr<UMeshElementsVisualizer> WireframeDisplay = nullptr;

	// OnCanonicalModified �㲥��Ϣ�������һ��������Ҫ�Ļ���������Ӹ������Ϣ��
	struct FCanonicalModifiedInfo
	{
		// Ŀǰ������������Ƿ���Ҫ�����κ�һ����
		// bapplieoverlaychanged �ƺ��ڷ�������ʱ����Ϊ�档��������� applied canonical �б༭һЩ�����������ʾ����bUnwrapMeshShapeChanged�ƺ�ֻ��Ϊ�١�����������ټ����ڴ��������£�ֻ��һ�������ĸ��¿��ܻ����ȫ(����Ȼ����unwrap�����˱仯)��
		// �����������ǲ�ʹ�����ǡ�
		// bool bUnwrapMeshShapeChanged = true;
		// bool bapplieoverlaychanged = true;

		// ��ֻ�����������ⲿ�༭�����Żᷢ������ΪUV�༭������ı��������״��
		bool bAppliedMeshShapeChanged = false;
	};

	/**
	 * �� canonical unwrap �� applied meshes �ı�ʱ���й㲥���⽫ͨ�������е�ʵ�ó��������й㲥���Ӷ�������Щ����( UpdateUnwrapCanonicalOverlayFromPositions ���⣬����ͨ�����������и������ĸ���)��
	 * �����Щʵ�ú���û��ʹ�ã����ͻ��˸���������һ�����񣬿ͻ���Ӧ�ù㲥���ı����Ա������Ҫ�������Ϣ(���磬�Ա�ģʽ���Խ���Щ������Ϊ���޸�)��
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnObjectModified, UFDOverlayMeshInput* InputObject, const FCanonicalModifiedInfo&);
	FOnObjectModified OnCanonicalModified;

	// Additional needed information
	TObjectPtr<UToolTarget> SourceTarget = nullptr;
	int32 AssetID = -1;
	int32 UVLayerIndex = 0;
	int32 MaterialIndex = -1;
	int32 MaxMaterialIndex = -1;

	// �������ɺͺ濾չ����ӳ�䡣
	TFunction<FVector3d(const FVector2f&)> UVToVertPosition;
	TFunction<FVector2f(const FVector3d&)> VertPositionToUV;

	bool InitializeMeshes(UToolTarget* Target, TSharedPtr<UE::Geometry::FDynamicMesh3> AppliedCanonicalIn,
		UMeshOpPreviewWithBackgroundCompute* AppliedPreviewIn, int32 AssetIDIn, int32 UVLayerIndexIn, 
		UWorld* UnwrapWorld, UWorld* LivePreviewWorld, UMaterialInterface* WorkingMaterialIn, 
		TFunction<FVector3d(const FVector2f&)> UVToVertPositionFuncIn, 
		TFunction<FVector2f(const FVector3d&)> VertPositionToUVFuncIn);

	void Shutdown() {};

	//ע�����±�������:
	// 1��ΪChangedVids / changnedelementids / ChangedConnectivityTids ���� nullptr ��ζ�����е� vids / elements / tids ����Ҫ�ֱ���¡�
	//		���� UFDOverlayMeshInput::NONE_CHANGED_ARG �൱�ڴ���һ��ָ��������ָ�룬��ζ��û���κα仯������ֻ��ָ�������е� vids / elements / tids �ڸ��µ������е�����
	//		��Ȼ������Щ����ʹ���¸��죬����ע�⣬δ�ܰ�����ص� tids / vids / elements ��ʹ�����������Ч״̬�����м�飬���磬���һ��������ָ��һ��δ��ӵ���vid��
	// 
	// 2��ChangedVids / ChangedElementIDs �������µ�Ԫ�أ���Ϊ�ָ�uv�Ǻ���Ȼ�ġ�Ȼ����ChangedConnectivityTids ����û���µ�Tids����ΪTids�γ������ǵ�չ�������ԭʼ��֮��Ķ�Ӧ��ϵ��
	//		��Ҫע����ǣ�����������ռ����������εĸ��ĵ� vids / elements����Ӧ�ô� post-change mesh/overlay ���ռ����ǣ�������Ͳ������κ���ӵ�Ԫ�أ���Ϊɾ����Ԫ����ͨ�����ĵ��������Ӳ���ġ�
	// 
	// 3��FastRenderUpdateTids��һ����ѡ���������б�������Ⱦ������Ҫ���£������Ⱦ����������ȷ�ָ��������������Ԥ�����¡�����ṩ����Ӧ����ChangedConnectivityTids�� superset������Ӧ�ð���ChangedVids�� one-ring triangles��
	// 
	// 4���������Ԥ��������ע�⺯�����᳢��ȡ���κλ���㣬��˻������ɺ���ܻ����á�

	//// ������Ϊ ChangedVids/ChangedElements/ChangedConnectivityTids �������룬�൱�ڴ���һ��ָ��������ָ�롣
	//static const TArray<int32>* const NONE_CHANGED_ARG;

	///**
	// * ���� UnwrapPreview UV Overlay �� UnwrapPreview vert λ�á�Ϊ positions �� UVs ����һ�� NotifyDeferredEditCompleted��
	// */
	//void UpdateUnwrapPreviewOverlayFromPositions(const TArray<int32>* ChangedVids = nullptr, 
	//	const TArray<int32>* ChangedConnectivityTids = nullptr, const TArray<int32>* FastRenderUpdateTids = nullptr) {};

	///**
	// * �� UnwrapCanonical vert λ�� ���� UnwrapCanonical UV Overlay��Ϊ positions �� UVs ����һ�� NotifyDeferredEditCompleted��
	// * ���㲥����ΪԤ���������֮�������һ�����������������������ಿ�֡�
	// */
	//void UpdateUnwrapCanonicalOverlayFromPositions(const TArray<int32>* ChangedVids = nullptr, 
	//	const TArray<int32>* ChangedConnectivityTids = nullptr) {};

	///**
	// * ��UnwrapPreview����AppliedPreview�������·�Ԥ�����񡣵�����ֻ���ĸ��¿ɼ���ʱ����������ק�ڼ���и��·ǳ����á�
	// * ����UnwrapPreview�еĸ��ǲ��Ѿ����¡�
	// */
	//void UpdateAppliedPreviewFromUnwrapPreview(const TArray<int32>* ChangedVids = nullptr, 
	//	const TArray<int32>* ChangedConnectivityTids = nullptr, const TArray<int32>* FastRenderUpdateTids = nullptr) {};

	///**
	// * ֻ��������AppliedPreview��UnwrapPreview���������·�Ԥ�����񡣵�����ֻ���ĸ��¿ɼ���ʱ��������ʱ���·ǳ����á�
	// */
	//void UpdateUnwrapPreviewFromAppliedPreview(
	//	const TArray<int32>* ChangedElementIDs = nullptr, const TArray<int32>* ChangedConnectivityTids = nullptr, 
	//	const TArray<int32>* FastRenderUpdateTids = nullptr) {};

	///**
	// * ���·�Ԥ���������磬�������ק����� canonical objects ʱ�����á�
	// */
	//void UpdateCanonicalFromPreviews(const TArray<int32>* ChangedVids = nullptr, 
	//	const TArray<int32>* ChangedConnectivityTids = nullptr, bool bBroadcast = true) {};

	///**
	// * �����ǵ� canonical��Ӧ�����Ԥ��������Ҫ��������Ԥ����
	// */
	//void UpdatePreviewsFromCanonical(const TArray<int32>* ChangedVids = nullptr, const TArray<int32>* ChangedConnectivityTids = nullptr,
	//	const TArray<int32>* FastRenderUpdateTids = nullptr) {};

	///**
	// * ʹ��UnwrapPreview�е���������������񡣼���UnwrapPreview�еĸ��ǲ��Ѹ��¡�
	// * ������ applied preview δ�� unwrap preview ͬ������ʱ������������(����ʹ��UpdateCanonicalFromPreviews)��
	// */
	//void UpdateAllFromUnwrapPreview(const TArray<int32>* ChangedVids = nullptr, const TArray<int32>* ChangedConnectivityTids = nullptr, 
	//	const TArray<int32>* FastRenderUpdateTids = nullptr, bool bBroadcast = true) {};

	///**
	// * ʹ��UnwrapCanonical�е����������������������Ӧ��ֱ�Ӷ� unwrap mesh ���в���ʱ�ǳ����á�
	// */
	//void UpdateAllFromUnwrapCanonical(const TArray<int32>* ChangedVids = nullptr, const TArray<int32>* ChangedConnectivityTids = nullptr, 
	//	const TArray<int32>* FastRenderUpdateTids = nullptr, bool bBroadcast = true) {};

	///**
	// * ʹ��AppliedCanonical�е���������������񡣴ӡ�original���������ü��ϡ�
	// */
	//void UpdateAllFromAppliedCanonical(const TArray<int32>* ChangedVids = nullptr, const TArray<int32>* ChangedConnectivityTids = nullptr, 
	//	const TArray<int32>* FastRenderUpdateTids = nullptr, bool bBroadcast = true) {};

	///**
	// * ��ʵʱԤ����ʹ��UV���Ǹ�����������
	// */
	//void UpdateAllFromAppliedPreview(const TArray<int32>* ChangedElementIDs = nullptr, 
	//	const TArray<int32>* ChangedConnectivityTids = nullptr, const TArray<int32>* FastRenderUpdateTids = nullptr, bool bBroadcast = true) {};

	///**
	// * ʹ����������д洢�� ������/���� ������ canonical unwrap ���������ݡ�
	// * ��������Ѿ�Ӧ�õ��淶��չ����
	// */
	//void UpdateFromCanonicalUnwrapUsingMeshChange(const UE::Geometry::FDynamicMeshChange& UnwrapCanonicalMeshChange, bool bBroadcast = true) {};

	///**
	// * ��չ�������е�vidת��ΪӦ�������еĶ�Ӧvid(������չ�������Ӧ��Ԫ�صĸ�����)��
	// */
	//int32 UnwrapVidToAppliedVid(int32 UnwrapVid) { return 0; };

	///**
	// * ��ö�Ӧ�ڸ���Ӧ��vid��չ��vid�����������һ���ӷ춥�㣬��Щ���Ƕ��������ж����֮������UVԪ�ء�
	// */
	//void AppliedVidToUnwrapVids(int32 AppliedVid, TArray<int32>& UnwrapVidsOut) {};

	///**
	// * ���صײ�ԴToolTarget�Ƿ���Ȼ��Ч��
	// * ���Ƿֿ���IsValid����Ҳ���ToolTarget��
	// * �Է�����ֻ��ToolTarget��״̬����Ȥ��
	//*/
	//virtual bool IsToolTargetValid() const { return false; };

	///**
	// * ���ع淶�����Ԥ�������Ƿ���Ȼ��Ч��
	// * ���Ǵ�IsValid�з�������ģ��Է�����ֻ�Թ淶��Ԥ�������״̬����Ȥ��
	// */
	//virtual bool AreMeshesValid() const { return false; };

	//// UToolTarget
	//virtual bool IsValid() const override {return false;};

private:
	void GenerateUVUnwrapMesh(const UE::Geometry::FDynamicMeshUVOverlay& UVOverlay, UE::Geometry::FDynamicMesh3& UnwrapMeshOut,
		TFunctionRef<FVector3d(const FVector2f&)> UVToVertPosition);
};