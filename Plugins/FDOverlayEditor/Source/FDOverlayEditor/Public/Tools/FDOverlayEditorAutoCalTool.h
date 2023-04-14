// Copyright HandsomeCheese. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "InteractiveTool.h"
#include "InteractiveToolBuilder.h"
#include "GeometryBase.h"
#include "FDAutoCalCS.h"
#include "Context\FDOverlayAutoCalToolAPI.h"
#include "Context\FDOverlayLive3DPreviewAPI.h"

#include "FDOverlayEditorAutoCalTool.generated.h"


class UFDOverlayMeshInput;
class UFDOverlayEditorAutoCalProperties;
class UFDOverlayRT2DArray;
class UMeshOpPreviewWithBackgroundCompute;
class FMeshDescriptionToDynamicMesh;
enum class EFDOverlayEditorAutoCalType : uint8;

PREDECLARE_GEOMETRY(class FDynamicMesh3);


struct FAuxiliaryMeshPreview
{
	EFDOverlayEditorAutoCalType type;
	TObjectPtr<UMeshOpPreviewWithBackgroundCompute> Preview;
};

/**
 * Builder for UFDOverlayEditorAutoCalTool
 */
 UCLASS()
class FDOVERLAYEDITOR_API UFDOverlayEditorAutoCalToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;

	// This is a pointer so that it can be updated under the builder without
	// having to set it in the mode after initializing targets.
	const TArray<TObjectPtr<UFDOverlayMeshInput>>* Targets = nullptr;
	TObjectPtr<UWorld>* LivePreviewWorld = nullptr;
};

// 在该框架中，你不需要自己创建UInteractiveTool的实例。
// 你只需提供一个UInteractiveToolBuilder的实现，它可以正确地构造一个你的工具实例，这就是例如默认参数被设置的地方。
UCLASS()
class FDOVERLAYEDITOR_API UFDOverlayEditorAutoCalTool : public UInteractiveTool
{
	GENERATED_BODY()

public: 
	UFDOverlayEditorAutoCalTool();
	void Initialize(const TArray<TObjectPtr<UFDOverlayMeshInput>>& TargetsIn, TObjectPtr<UWorld> LivePreviewWorldIn);
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	
	virtual void OnTick(float DeltaTime) override;

	virtual bool HasCancel() const override { return true; }
	virtual bool HasAccept() const override { return true; }
	virtual bool CanAccept() const override;

	virtual void OnPropertyModified(UObject* PropertySet, FProperty* Property) override;

	void UpdateOutputTexture(UTextureRenderTarget2DArray* BakeBuffer, int TargetID);
	UTexture2D* ConstructTexture2D(UTextureRenderTarget2DArray* BakeBufferIn, UObject* ObjOuter, const FString& NewTexName, EObjectFlags InFlags, int32 SliceId);
	void ExecuteBakePass();

	// Setting Property Set
	void OnChangeOutputType(EAutoCalToolOutputType type)
	{
		OutputType = type;
	}


	DECLARE_DELEGATE(FOnFinishCS);
	FOnFinishCS OnFinishCS;

protected:

	void InitializeCurve();
	void InitializeMeshResource();
	void InitializeBakePass(int32 MIDNum, int TargetID);
	void InitializeBakeParams(struct FExtraParams& ExtraParams, int TargetID, FVector3f Origin, FVector3f Direction, const TArray<FCurveKey>& CurveKeysIn);
	template<typename T>
	void InitializeBakeParams(FMultiExtraParams& ExtraParams, int TargetID, const TArray<T>& DataList, const TArray<float>& MultiCurveRange, const TArray<FCurveKey>& MultiCurveKeys, const TArray<int>& MultiCurveKeysNum);
	template<typename T>
	void AddBakePass(T ExtraParams, int TargetID, EFDOverlayEditorAutoCalType Type);
	void UpdatedAuxiliaryMeshPreview();
protected:

	UPROPERTY()
	TArray<TObjectPtr<UFDOverlayMeshInput>> Targets;

	UPROPERTY()
	TObjectPtr<UFDOverlayEditorAutoCalProperties> Settings = nullptr;

	TObjectPtr<UWorld> LivePreviewWorld = nullptr;

private:

	FString GetAssetPath(FString PathFormat, FString Name, int32 MaterialID) const;
	UTexture2D* FindOrCreate(const FString& AssetPath);

	FVector GetUVOffsetOrigin(const float& UVOffset, const FVector& LineOrigin, const FVector& LineDirection);
	float GetDistanceAlongLine(const FVector& Point, const FVector& LineOrigin, const FVector& LineDirection);

private:
	// Single
	TArray<FCurveKey> CurveKeys;
	float CurveRange;

	// MultiLine
	TArray<FCurveKey> MultiLineCurveKeys;
	TArray<float> MultiLineCurveRange;
	TArray<int> MultiLineCurveKeysNum;

	// MultiPoint
	TArray<FCurveKey> MultiPointCurveKeys;
	TArray<float> MultiPointCurveRange;
	TArray<int> MultiPointCurveKeysNum; 

	TArray<UTextureRenderTarget2DArray*> BakeBuffer;

	TArray<TArray<FUVVertex>> Vertices;
	TArray<TArray<FTriangle>> Triangles;

	TArray<FAuxiliaryMeshPreview> AuxiliaryMeshPreviews;

	TObjectPtr<UStaticMesh> ArrowMesh_Static;
	TObjectPtr<UStaticMesh> SphereMesh_Static;
	TSharedPtr<UE::Geometry::FDynamicMesh3> ArrowMesh_Dynamic;
	TSharedPtr<UE::Geometry::FDynamicMesh3> SphereMesh_Dynamic;
	TObjectPtr<UMaterialInterface> DefaultArrow_MI;
	TObjectPtr<UMaterialInterface> DefaultSphere_MI;
	TObjectPtr<UMaterialInstanceDynamic> DefaultArrow_DMI;
	TObjectPtr<UMaterialInstanceDynamic> DefaultSphere_DMI;

	// Setting Property Set
	EAutoCalToolOutputType OutputType = EAutoCalToolOutputType::Texture2DArray;
};