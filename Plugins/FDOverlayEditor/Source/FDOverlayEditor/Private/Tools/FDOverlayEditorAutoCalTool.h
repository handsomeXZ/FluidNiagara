#pragma once

#include "CoreMinimal.h"

#include "InteractiveTool.h"
#include "InteractiveToolBuilder.h"
#include "FDOverlayEditorAutoCalTool.generated.h"

class UFDOverlayMeshInput;

struct FAppliedVertex
{
	FVector3f Position;
	FVector3f Normal;
	FVector3f UV;
};
struct FTriangle
{
	int A;
	int B;
	int C;
};
struct ExtraParams
{
	FIntPoint Size;
	TRefCountPtr<IPooledRenderTarget> ExtractedTexture;
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
	const TObjectPtr<UFDOverlayMeshInput>* Target = nullptr;
};

// 在该框架中，你不需要自己创建UInteractiveTool的实例。
// 你只需提供一个UInteractiveToolBuilder的实现，它可以正确地构造一个你的工具实例，这就是例如默认参数被设置的地方。
UCLASS()
class FDOVERLAYEDITOR_API UFDOverlayEditorAutoCalTool : public UInteractiveTool
{
	GENERATED_BODY()

public: 
	void SetTarget(const TObjectPtr<UFDOverlayMeshInput>& Target);
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	
protected:
	UPROPERTY()
	TObjectPtr<UFDOverlayMeshInput> Target;

};