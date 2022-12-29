// Copyright Epic Games, Inc. All Rights Reserved.

#include "FDOverlayEditorEditorModeToolkit.h"
#include "FDOverlayEditorEditorMode.h"
#include "Engine/Selection.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "FDOverlayEditorEditorModeToolkit"

FFDOverlayEditorEditorModeToolkit::FFDOverlayEditorEditorModeToolkit()
{
}

void FFDOverlayEditorEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);
}

void FFDOverlayEditorEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add(NAME_Default);
}


FName FFDOverlayEditorEditorModeToolkit::GetToolkitFName() const
{
	return FName("FDOverlayEditorEditorMode");
}

FText FFDOverlayEditorEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "FDOverlayEditorEditorMode Toolkit");
}

#undef LOCTEXT_NAMESPACE
