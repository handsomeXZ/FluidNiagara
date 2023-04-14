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
	int32 RenderMode = 0;

	// �������ɺͺ濾չ����ӳ�䡣
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