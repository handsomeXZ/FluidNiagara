// Copyright Epic Games, Inc. All Rights Reserved.

#include "FDAssistorEditorModeToolkit.h"
#include "FDAssistorEditorMode.h"
#include "Engine/Selection.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "FDAssistorEditorModeToolkit"

FFDAssistorEditorModeToolkit::FFDAssistorEditorModeToolkit()
{
}

void FFDAssistorEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);
}

void FFDAssistorEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add(NAME_Default);
}


FName FFDAssistorEditorModeToolkit::GetToolkitFName() const
{
	return FName("FDAssistorEditorMode");
}

FText FFDAssistorEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "FDAssistorEditorMode Toolkit");
}

#undef LOCTEXT_NAMESPACE
