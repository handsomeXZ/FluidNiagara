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
	* ���Թ�������Ŀ�꣬ΪFD���߹����ṩ�����塣
	*/
	virtual void BuildTargets(const TArray<TObjectPtr<UObject>>& ObjectsIn,
		const FToolTargetTypeRequirements& TargetRequirements,
		TArray<TObjectPtr<UToolTarget>>& TargetsOut);


	/**
	* ��Ҫ�ڹر� FD�༭��ʵ��ʱ���ã��Ա���ϵͳ֪��
	* Ϊ��Щ���󴴽�һ���µĶ�����������ٴδ򿪡�
	*/
	virtual void NotifyThatFDEditorClosed(TArray<TObjectPtr<UObject>> ObjectsItWasEditing);


	virtual void LaunchFDOverlayEditor(const TArray<TObjectPtr<UObject>>& ObjectsToEdit);
	virtual bool CanLaunchFDOverlayEditor(const TArray<TObjectPtr<UObject>>& ObjectsIn);

protected:
	void ConvertInputArgsToValidTargets(const TArray<TObjectPtr<UObject>>& ObjectsIn, TArray<TObjectPtr<UObject>>& ObjectsOut) const;


protected:
	/**
	 * ��������ϵͳ�ж�Ŀ���Ƿ���Ч���ڿ����¹���ʱ��Ӧ����Initialize()��������ǡ�
	 */
	UPROPERTY()
	TObjectPtr<UToolTargetManager> ToolTargetManager = nullptr;
	
	TMap<TObjectPtr<UObject>, TObjectPtr<UFDOverlayAssetEditor>> OpenedEditorInstances;
};
