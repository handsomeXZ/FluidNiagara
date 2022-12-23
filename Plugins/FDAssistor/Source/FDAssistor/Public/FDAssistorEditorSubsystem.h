// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "FDAssistorEditorSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class FDASSISTOR_API UFDAssistorEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Checks that all of the objects are valid targets for a FD editor session. */
	virtual bool AreObjectsValidTargets(const TArray<UObject*>& InObjects) const;
};
