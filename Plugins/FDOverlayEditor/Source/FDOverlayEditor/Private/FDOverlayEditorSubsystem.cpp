// Fill out your copyright notice in the Description page of Project Settings.


#include "FDOverlayEditorSubsystem.h"

#include "ToolTargetManager.h"
#include "ToolTargets/DynamicMeshComponentToolTarget.h"
#include "ToolTargets/StaticMeshToolTarget.h"
#include "ToolTargets/SkeletalMeshToolTarget.h"

#include "FDOverlayAssetEditor.h"
#include "FDOverlayEditorMode.h"

void UFDOverlayEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// The subsystem has its own tool target manager because it must exist before any UV editors exist,
	// to see if a FD editor can be started.
	ToolTargetManager = NewObject<UToolTargetManager>(this);
	ToolTargetManager->Initialize();

	ToolTargetManager->AddTargetFactory(NewObject<UStaticMeshToolTargetFactory>(ToolTargetManager));
	ToolTargetManager->AddTargetFactory(NewObject<USkeletalMeshToolTargetFactory>(ToolTargetManager));
	ToolTargetManager->AddTargetFactory(NewObject<UDynamicMeshComponentToolTargetFactory>(ToolTargetManager));
}
void UFDOverlayEditorSubsystem::Deinitialize()
{
	ToolTargetManager->Shutdown();
	ToolTargetManager = nullptr;
}
bool UFDOverlayEditorSubsystem::AreObjectsValidTargets(const TArray<UObject*>& InObjects) const
{
	if (InObjects.IsEmpty())
	{
		return false;
	}

	for (UObject* Object : InObjects)
	{
		if (!ToolTargetManager->CanBuildTarget(Object, UFDOverlayEditorMode::GetToolTargetRequirements()))
		{
			return false;
		}

	}

	return true;
}
void UFDOverlayEditorSubsystem::BuildTargets(const TArray<TObjectPtr<UObject>>& ObjectsIn,
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

void UFDOverlayEditorSubsystem::NotifyThatFDEditorClosed(TArray<TObjectPtr<UObject>> ObjectsItWasEditing)
{
	for (TObjectPtr<UObject>& Object : ObjectsItWasEditing)
	{
		OpenedEditorInstances.Remove(Object);
	}
}
void UFDOverlayEditorSubsystem::LaunchFDOverlayEditor(const TArray<TObjectPtr<UObject>>& ObjectsToEdit)
{
	TArray<TObjectPtr<UObject>> ProcessedObjects;
	ConvertInputArgsToValidTargets(ObjectsToEdit, ProcessedObjects);
	
	// 如果已有实例中已经打开了任何对象，则不允许打开新实例。相反，我们把这样一个实例放在前面。
	// 请注意，资产编辑器子系统负责 “primary” 资产编辑器，即双击资产或选择“编辑”时打开的编辑器。
	// 由于FD编辑器不是任何资产类型的 “primary” 资产编辑器，因此我们自己进行管理。
	for (TObjectPtr<UObject>& Object : ProcessedObjects)
	{
		if (OpenedEditorInstances.Contains(Object))
		{
			OpenedEditorInstances[Object]->GetInstanceInterface()->FocusWindow(Object);
			return;
		}
	}
	// If we got here, there's not an instance already opened.

	UFDOverlayAssetEditor* FDOverlayEditor = NewObject<UFDOverlayAssetEditor>(this);

	// 在其他事情中，这个调用将 FD编辑器注册到资产编辑器子系统，这将防止它被垃圾收集。
	FDOverlayEditor->Initialize(ProcessedObjects);
	TObjectPtr<UObject>& Object0 = ProcessedObjects[0];

	for (TObjectPtr<UObject>& Object : ProcessedObjects)
	{
		OpenedEditorInstances.Add(Object, FDOverlayEditor);
	}
}

bool UFDOverlayEditorSubsystem::CanLaunchFDOverlayEditor(const TArray<TObjectPtr<UObject>>& ObjectsIn)
{
	TArray<TObjectPtr<UObject>> ProcessedObjects;
	ConvertInputArgsToValidTargets(ObjectsIn, ProcessedObjects);
	return AreObjectsValidTargets(ProcessedObjects);
}

void UFDOverlayEditorSubsystem::ConvertInputArgsToValidTargets(const TArray<TObjectPtr<UObject>>& ObjectsIn, TArray<TObjectPtr<UObject>>& ObjectsOut) const
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
