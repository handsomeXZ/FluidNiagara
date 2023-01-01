// Copyright Epic Games, Inc. All Rights Reserved.

#include "FDOverlayEditorModeToolkit.h"
#include "Engine/Selection.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "FDOverlayEditoModeToolkit"

FFDOverlayEditorModeToolkit::FFDOverlayEditorModeToolkit()
{
}

void FFDOverlayEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);
}

void FFDOverlayEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add(NAME_Default);
}

FName FFDOverlayEditorModeToolkit::GetToolkitFName() const
{
	return FName("FDOverlayEditorMode");
}

FText FFDOverlayEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "FDOverlayEditorMode Toolkit");
}

#undef LOCTEXT_NAMESPACE
