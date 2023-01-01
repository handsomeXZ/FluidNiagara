// Fill out your copyright notice in the Description page of Project Settings.


#include "FDOverlayAssetEditor.h"

#include "FDOverlayAssetEditorTooltik.h"
#include "FDOverlayEditorSubsystem.h"


void UFDOverlayAssetEditor::Initialize(const TArray<TObjectPtr<UObject>>& InObjects)
{
	// Make sure we have valid targets.
	UFDOverlayEditorSubsystem* UVSubsystem = GEditor->GetEditorSubsystem<UFDOverlayEditorSubsystem>();
	check(UVSubsystem && UVSubsystem->AreObjectsValidTargets(InObjects));

	OriginalObjectsToEdit = InObjects;

	// This will do a variety of things including registration of the asset editor, creation of the toolkit
	// (via CreateToolkit()), and creation of the editor mode manager within the toolkit.
	// The asset editor toolkit will do the rest of the necessary initialization inside its PostInitAssetEditor.
	UAssetEditor::Initialize();
}

IAssetEditorInstance* UFDOverlayAssetEditor::GetInstanceInterface()
{
	return ToolkitInstance;
}

void UFDOverlayAssetEditor::GetObjectsToEdit(TArray<UObject*>& OutObjects)
{
	OutObjects.Append(OriginalObjectsToEdit);
	check(OutObjects.Num() > 0);
}

TSharedPtr<FBaseAssetToolkit> UFDOverlayAssetEditor::CreateToolkit()
{
	TSharedPtr<FFDOverlayAssetEditorToolkit> Toolkit = MakeShared<FFDOverlayAssetEditorToolkit>(this);

	return Toolkit;
}