// Fill out your copyright notice in the Description page of Project Settings.


#include "FullBodyOverlayEditor/FDFBOverlayEditorSubsystem.h"

#include "ToolTargetManager.h"
#include "ToolTargets/DynamicMeshComponentToolTarget.h"
#include "ToolTargets/StaticMeshToolTarget.h"
#include "ToolTargets/SkeletalMeshToolTarget.h"

#include "FullBodyOverlayEditor/FDFBOverlayAssetEditor.h"
//#include "FullBodyOverlayEditor/FDFBOverlayEditorMode.h"

void UFDFBOverlayEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// The subsystem has its own tool target manager because it must exist before any UV editors exist,
	// to see if a FD editor can be started.
	ToolTargetManager = NewObject<UToolTargetManager>(this);
	ToolTargetManager->Initialize();

	ToolTargetManager->AddTargetFactory(NewObject<UStaticMeshToolTargetFactory>(ToolTargetManager));
	ToolTargetManager->AddTargetFactory(NewObject<USkeletalMeshToolTargetFactory>(ToolTargetManager));
	ToolTargetManager->AddTargetFactory(NewObject<UDynamicMeshComponentToolTargetFactory>(ToolTargetManager));
}
void UFDFBOverlayEditorSubsystem::Deinitialize()
{
	ToolTargetManager->Shutdown();
	ToolTargetManager = nullptr;
}
bool UFDFBOverlayEditorSubsystem::AreObjectsValidTargets(const TArray<UObject*>& InObjects) const
{
	if (InObjects.IsEmpty())
	{
		return false;
	}

	for (UObject* Object : InObjects)
	{
		/*if (!ToolTargetManager->CanBuildTarget(Object, UFDFBOverlayEditorMode::GetToolTargetRequirements()))
		{
			return false;
		}*/
		return false;
	}

	return true;
}
void UFDFBOverlayEditorSubsystem::BuildTargets(const TArray<TObjectPtr<UObject>>& ObjectsIn,
	const FToolTargetTypeRequirements& TargetRequirements, TArray<TObjectPtr<UToolTarget>>& TargetsOut)
{
	TargetsOut.Reset();

	for (UObject* Object : ObjectsIn)
	{
		UToolTarget* Target = ToolTargetManager->BuildTarget(Object, TargetRequirements);
		if (Target)
		{
			TargetsOut.Add(Target);
		}
	}
}

void UFDFBOverlayEditorSubsystem::NotifyThatFDEditorClosed(TArray<TObjectPtr<UObject>> ObjectsItWasEditing)
{
	for (TObjectPtr<UObject>& Object : ObjectsItWasEditing)
	{
		OpenedEditorInstances.Remove(Object);
	}
}
void UFDFBOverlayEditorSubsystem::LaunchFDFBOverlayEditor(const TArray<TObjectPtr<UObject>>& ObjectsToEdit)
{
	TArray<TObjectPtr<UObject>> ProcessedObjects;
	ConvertInputArgsToValidTargets(ObjectsToEdit, ProcessedObjects);
	
	// �������ʵ�����Ѿ������κζ������������ʵ�����෴�����ǰ�����һ��ʵ������ǰ�档
	// ��ע�⣬�ʲ��༭����ϵͳ���� ��primary�� �ʲ��༭������˫���ʲ���ѡ�񡰱༭��ʱ�򿪵ı༭����
	// ����FD�༭�������κ��ʲ����͵� ��primary�� �ʲ��༭������������Լ����й���
	for (TObjectPtr<UObject>& Object : ProcessedObjects)
	{
		if (OpenedEditorInstances.Contains(Object))
		{
			OpenedEditorInstances[Object]->GetInstanceInterface()->FocusWindow(Object);
			return;
		}
	}
	// If we got here, there's not an instance already opened.

	UFDFBOverlayAssetEditor* FDFBOverlayEditor = NewObject<UFDFBOverlayAssetEditor>(this);

	// �����������У�������ý� FD�༭��ע�ᵽ�ʲ��༭����ϵͳ���⽫��ֹ���������ռ���
	FDFBOverlayEditor->Initialize(ProcessedObjects);

	for (TObjectPtr<UObject>& Object : ProcessedObjects)
	{
		OpenedEditorInstances.Add(Object, FDFBOverlayEditor);
	}
}

bool UFDFBOverlayEditorSubsystem::CanLaunchFDFBOverlayEditor(const TArray<TObjectPtr<UObject>>& ObjectsIn)
{
	TArray<TObjectPtr<UObject>> ProcessedObjects;
	ConvertInputArgsToValidTargets(ObjectsIn, ProcessedObjects);
	return AreObjectsValidTargets(ProcessedObjects);
}

void UFDFBOverlayEditorSubsystem::ConvertInputArgsToValidTargets(const TArray<TObjectPtr<UObject>>& ObjectsIn, TArray<TObjectPtr<UObject>>& ObjectsOut) const
{
	for (const TObjectPtr<UObject>& Object : ObjectsIn)
	{
		const AActor* Actor = Cast<const AActor>(Object);
		if (Actor)
		{
			TArray<UObject*> ActorAssets;
			Actor->GetReferencedContentObjects(ActorAssets);

			if (ActorAssets.Num() > 0)
			{
				for (UObject* Asset : ActorAssets)
				{
					ObjectsOut.AddUnique(Asset);
				}
			}
			else {
				// TODO: What is this actually used for?
				// Need to transform actors to components here because that's what the FDEditor expects to have
				TInlineComponentArray<UActorComponent*> ActorComponents;
				Actor->GetComponents(ActorComponents);
				ObjectsOut.Append(ActorComponents);
			}
		}
		else
		{
			ObjectsOut.AddUnique(Object);
		}
	}
}