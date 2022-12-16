// Copyright Epic Games, Inc. All Rights Reserved.

#include "FluidDynamicTooltikModeEditorModeToolkit.h"
#include "FluidDynamicTooltikModeEditorMode.h"
#include "Engine/Selection.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "FluidDynamicTooltikModeEditorModeToolkit"

FFluidDynamicTooltikModeEditorModeToolkit::FFluidDynamicTooltikModeEditorModeToolkit()
{
}

void FFluidDynamicTooltikModeEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);
}

void FFluidDynamicTooltikModeEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add(NAME_Default);
}


FName FFluidDynamicTooltikModeEditorModeToolkit::GetToolkitFName() const
{
	return FName("FluidDynamicTooltikModeEditorMode");
}

FText FFluidDynamicTooltikModeEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "FluidDynamicTooltikModeEditorMode Toolkit");
}

#undef LOCTEXT_NAMESPACE
