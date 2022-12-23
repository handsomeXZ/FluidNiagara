// Fill out your copyright notice in the Description page of Project Settings.


#include "FDAssistorAssetEditor.h"

#include "FDAssistorAssetEditorTooltik.h"
#include "FDAssistorEditorSubsystem.h"


void UFDAssistorAssetEditor::Initialize(const TArray<TObjectPtr<UObject>>& InObjects)
{
	// Make sure we have valid targets.
	UFDAssistorEditorSubsystem* UVSubsystem = GEditor->GetEditorSubsystem<UFDAssistorEditorSubsystem>();
	check(UVSubsystem && UVSubsystem->AreObjectsValidTargets(InObjects));

	OriginalObjectsToEdit = InObjects;

	// This will do a variety of things including registration of the asset editor, creation of the toolkit
	// (via CreateToolkit()), and creation of the editor mode manager within the toolkit.
	// The asset editor toolkit will do the rest of the necessary initialization inside its PostInitAssetEditor.
	UAssetEditor::Initialize();
}

IAssetEditorInstance* UFDAssistorAssetEditor::GetInstanceInterface()
{
	return ToolkitInstance;
}

TSharedPtr<FBaseAssetToolkit> UFDAssistorAssetEditor::CreateToolkit()
{
	TSharedPtr<FFDAssistorAssetEditorToolkit> Toolkit = MakeShared<FFDAssistorAssetEditorToolkit>(this);

	return Toolkit;
}