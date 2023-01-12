#pragma once

#include "CoreMinimal.h"

#include "InteractiveTool.h"
#include "InteractiveToolBuilder.h"
#include "FDOverlayEditorAutoCalTool.generated.h"

class UFDOverlayMeshInput;
class UFDOverlayEditorAutoCalProperties;



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

	void UpdateOutputTexture(UTexture2D* OutputTexture);
	
	DECLARE_DELEGATE_OneParam(FOnFinishCS, UTexture2D*);
	FOnFinishCS OnFinishCS;

protected:
	UPROPERTY()
	TArray<TObjectPtr<UFDOverlayMeshInput>> Targets;

	UPROPERTY()
	TObjectPtr<UFDOverlayEditorAutoCalProperties> Settings = nullptr;

private:
	FString GetAssetPath(FString PathFormat, FString Name, int32 MaterialID) const;

	UTexture2D* FindOrCreate(const FString& AssetPath);

};