// Copyright Epic Games, Inc. All Rights Reserved.

#include "FDOverlayEditorModeCommands.h"
//#include "FDOverlayEditorMode.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "FDOverlayEditorModeCommands"

FFDOverlayEditorModeCommands::FFDOverlayEditorModeCommands()
	: TCommands<FFDOverlayEditorModeCommands>("FDOverlayEditorMode",
		NSLOCTEXT("FDOverlayEditorMode", "FDOverlayEditorModeCommands", "FDOverlay Editor Mode"),
		NAME_None,
		FEditorStyle::GetStyleSetName())
{
}

void FFDOverlayEditorModeCommands::RegisterCommands()
{
	TArray <TSharedPtr<FUICommandInfo>>& RegisteredTools = Commands.FindOrAdd(NAME_Default);

#define REGISTER_MODELING_TOOL_COMMAND(ToolCommandInfo, ToolName, ToolTip) \
		UI_COMMAND(ToolCommandInfo, ToolName, ToolTip, EUserInterfaceActionType::Button, FInputChord()); \
		RegisteredTools.Add(ToolCommandInfo);

	// These are part of the asset editor UI
	UI_COMMAND(ApplyChanges, "Apply", "Apply changes to original meshes", EUserInterfaceActionType::Button, FInputChord());
	
	REGISTER_MODELING_TOOL_COMMAND(AutoCalTool, "AutoCalTool", "Auto Calulate Overlay");


}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FFDOverlayEditorModeCommands::GetCommands()
{
	return FFDOverlayEditorModeCommands::Get().Commands;
}

#undef LOCTEXT_NAMESPACE
