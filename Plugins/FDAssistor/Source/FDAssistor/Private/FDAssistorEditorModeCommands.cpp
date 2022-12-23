// Copyright Epic Games, Inc. All Rights Reserved.

#include "FDAssistorEditorModeCommands.h"
#include "FDAssistorEditorMode.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "FDAssistorEditorModeCommands"

FFDAssistorEditorModeCommands::FFDAssistorEditorModeCommands()
	: TCommands<FFDAssistorEditorModeCommands>("FDAssistorEditorMode",
		NSLOCTEXT("FDAssistorEditorMode", "FDAssistorEditorModeCommands", "FDAssistor Editor Mode"),
		NAME_None,
		FEditorStyle::GetStyleSetName())
{
}

void FFDAssistorEditorModeCommands::RegisterCommands()
{
	TArray <TSharedPtr<FUICommandInfo>>& ToolCommands = Commands.FindOrAdd(NAME_Default);

	UI_COMMAND(SimpleTool, "Show Actor Info", "Opens message box with info about a clicked actor", EUserInterfaceActionType::Button, FInputChord());
	ToolCommands.Add(SimpleTool);

	UI_COMMAND(InteractiveTool, "Measure Distance", "Measures distance between 2 points (click to set origin, shift-click to set end point)", EUserInterfaceActionType::ToggleButton, FInputChord());
	ToolCommands.Add(InteractiveTool);

	// These are part of the asset editor UI
	UI_COMMAND(ApplyChanges, "Apply", "Apply changes to original meshes", EUserInterfaceActionType::Button, FInputChord());



}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FFDAssistorEditorModeCommands::GetCommands()
{
	return FFDAssistorEditorModeCommands::Get().Commands;
}

#undef LOCTEXT_NAMESPACE
