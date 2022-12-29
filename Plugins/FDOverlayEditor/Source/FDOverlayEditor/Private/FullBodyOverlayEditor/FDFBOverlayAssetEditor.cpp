// Fill out your copyright notice in the Description page of Project Settings.


#include "FullBodyOverlayEditor/FDFBOverlayAssetEditor.h"

#include "FullBodyOverlayEditor/FDFBOverlayAssetEditorTooltik.h"
#include "FullBodyOverlayEditor/FDFBOverlayEditorSubsystem.h"


void UFDFBOverlayAssetEditor::Initialize(const TArray<TObjectPtr<UObject>>& InObjects)
{
	// Make sure we have valid targets.
	UFDFBOverlayEditorSubsystem* UVSubsystem = GEditor->GetEditorSubsystem<UFDFBOverlayEditorSubsystem>();
	check(UVSubsystem && UVSubsystem->AreObjectsValidTargets(InObjects));

	OriginalObjectsToEdit = InObjects;

	// This will do a variety of things including registration of the asset editor, creation of the toolkit
	// (via CreateToolkit()), and creation of the editor mode manager within the toolkit.
	// The asset editor toolkit will do the rest of the necessary initialization inside its PostInitAssetEditor.
	UAssetEditor::Initialize();
}

IAssetEditorInstance* UFDFBOverlayAssetEditor::GetInstanceInterface()
{
	return ToolkitInstance;
}

TSharedPtr<FBaseAssetToolkit> UFDFBOverlayAssetEditor::CreateToolkit()
{
	TSharedPtr<FFDFBOverlayAssetEditorToolkit> Toolkit = MakeShared<FFDFBOverlayAssetEditorToolkit>(this);

	return Toolkit;
}