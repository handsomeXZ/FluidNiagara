#pragma once

#include "CoreMinimal.h"

#include "InteractiveTool.h"
#include "InteractiveToolBuilder.h"
#include "FDAutoCalCS.h"
#include "GeometryBase.h"

#include "FDOverlayEditorAutoCalTool.generated.h"


class UFDOverlayMeshInput;
class UFDOverlayEditorAutoCalProperties;
class UTextureRenderTarget2DArray;
enum class EFDOverlayEditorAutoCalType : uint8;
PREDECLARE_GEOMETRY(class FDynamicMesh3);

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
};

// 在该框架中，你不需要自己创建UInteractiveTool的实例。
// 你只需提供一个UInteractiveToolBuilder的实现，它可以正确地构造一个你的工具实例，这就是例如默认参数被设置的地方。
UCLASS()
class FDOVERLAYEDITOR_API UFDOverlayEditorAutoCalTool : public UInteractiveTool
{
	GENERATED_BODY()

public: 

	void SetTarget(const TArray<TObjectPtr<UFDOverlayMeshInput>>& TargetsIn);
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	
	virtual void OnTick(float DeltaTime) override;

	virtual bool HasCancel() const override { return true; }
	virtual bool HasAccept() const override { return true; }
	virtual bool CanAccept() const override;

	virtual void OnPropertyModified(UObject* PropertySet, FProperty* Property) override;

	void UpdateOutputTexture(FExtraParams ExtraParams);
	
	DECLARE_DELEGATE_OneParam(FOnFinishCS, FExtraParams ExtraParams);
	FOnFinishCS OnFinishCS;

protected:

	void InitializeCurve();
	void InitializeMeshResource();
	void InitializeBakePass(int32 MIDNum, int TargetID);
	void InitializeBakeParams(FExtraParams& ExtraParams, int TargetID, FVector3f Origin, FVector3f Direction, const TArray<FCurveKey>& CurveKeysIn);
	template<typename T>
	void InitializeBakeParams(FMultiExtraParams& ExtraParams, int TargetID, const TArray<T>& DataList, const TArray<float>& MultiCurveRange, const TArray<TArray<FCurveKey>>& MultiCurveKeys);
	template<typename T>
	void AddBakePass(T ExtraParams, int TargetID, EFDOverlayEditorAutoCalType Type);
	
protected:

	UPROPERTY()
	TArray<TObjectPtr<UFDOverlayMeshInput>> Targets;

	UPROPERTY()
	TObjectPtr<UFDOverlayEditorAutoCalProperties> Settings = nullptr;

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
	TArray<TArray<FCurveKey>> MultiLineCurveKeys;
	TArray<float> MultiLineCurveRange;

	// MultiPoint
	TArray<TArray<FCurveKey>> MultiPointCurveKeys;
	TArray<float> MultiPointCurveRange;

	TArray<UTextureRenderTarget2DArray*> BakeBuffer;

	TArray<TArray<FUVVertex>> Vertices;
	TArray<TArray<FTriangle>> Triangles;
};