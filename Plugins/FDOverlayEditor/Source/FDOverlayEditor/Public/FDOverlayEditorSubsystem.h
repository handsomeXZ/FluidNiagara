// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "FDOverlayEditorSubsystem.generated.h"

class UFDOverlayAssetEditor;
class UToolTarget;
class UToolTargetManager;
class FToolTargetTypeRequirements;

/**
 * 
 */
UCLASS()
class FDOVERLAYEDITOR_API UFDOverlayEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Checks that all of the objects are valid targets for a FD editor session. */
	virtual bool AreObjectsValidTargets(const TArray<UObject*>& InObjects) const;

	/**
	* 尝试构建核心目标，为FD工具工作提供网格体。
	*/
	virtual void BuildTargets(const TArray<TObjectPtr<UObject>>& ObjectsIn,
		const FToolTargetTypeRequirements& TargetRequirements,
		TArray<TObjectPtr<UToolTarget>>& TargetsOut);


	/**
	* 需要在关闭 FD编辑器实例时调用，以便子系统知道
	* 为这些对象创建一个新的对象，如果它们再次打开。
	*/
	virtual void NotifyThatFDEditorClosed(TArray<TObjectPtr<UObject>> ObjectsItWasEditing);


	virtual void LaunchFDOverlayEditor(const TArray<TObjectPtr<UObject>>& ObjectsToEdit);
	virtual bool CanLaunchFDOverlayEditor(const TArray<TObjectPtr<UObject>>& ObjectsIn);

protected:
	void ConvertInputArgsToValidTargets(const TArray<TObjectPtr<UObject>>& ObjectsIn, TArray<TObjectPtr<UObject>>& ObjectsOut) const;


protected:
	/**
	 * 用于让子系统判断目标是否有效。在开发新工厂时，应该在Initialize()中添加它们。
	 */
	UPROPERTY()
	TObjectPtr<UToolTargetManager> ToolTargetManager = nullptr;
	
	TMap<TObjectPtr<UObject>, TObjectPtr<UFDOverlayAssetEditor>> OpenedEditorInstances;
};
