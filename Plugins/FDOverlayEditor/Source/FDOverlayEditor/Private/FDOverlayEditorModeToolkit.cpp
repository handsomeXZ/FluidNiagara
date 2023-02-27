// Copyright Epic Games, Inc. All Rights Reserved.

#include "FDOverlayEditorModeToolkit.h"

#include "Engine/Selection.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"

#include "FDOverlayEditorMode.h"

#define LOCTEXT_NAMESPACE "FDOverlayEditoModeToolkit"

FFDOverlayEditorModeToolkit::FFDOverlayEditorModeToolkit()
{
}

void FFDOverlayEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);

	// Set up the overlay. Largely copied from ModelingToolsEditorModeToolkit.
	// TODO: We could put some of the shared code in some common place.
	//SAssignNew(ViewportOverlayWidget, SHorizontalBox)


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

void FFDOverlayEditorModeToolkit::OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	FModeToolkit::OnToolStarted(Manager, Tool);

	UFDOverlayEditorMode* Mode = Cast<UFDOverlayEditorMode>(GetScriptableEditorMode());
	//GetToolkitHost()->AddViewportOverlayWidget(ViewportOverlayWidget.ToSharedRef());
}

void FFDOverlayEditorModeToolkit::OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	FModeToolkit::OnToolEnded(Manager, Tool);
	//GetToolkitHost()->RemoveViewportOverlayWidget(ViewportOverlayWidget.ToSharedRef());

}

#undef LOCTEXT_NAMESPACE
