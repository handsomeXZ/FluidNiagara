// Copyright HandsomeCheese. All Rights Reserved.

#include "FDOverlayEditorModeCommands.h"
#include "Tools/FDOverlayStyle.h"
//#include "FDOverlayEditorMode.h"

#include "EditorStyleSet.h"


#define LOCTEXT_NAMESPACE "FDOverlayEditorModeCommands"

FFDOverlayEditorModeCommands::FFDOverlayEditorModeCommands()
	: TCommands<FFDOverlayEditorModeCommands>("FDOverlayEditorMode",
		NSLOCTEXT("FDOverlayEditorMode", "FDOverlayEditorModeCommands", "FDOverlay Editor Mode"),
		NAME_None,
		FFDOverlayStyle::Get().GetStyleSetName())
{
}

void FFDOverlayEditorModeCommands::RegisterCommands()
{
	TArray <TSharedPtr<FUICommandInfo>>& RegisteredTools = Commands.FindOrAdd(NAME_Default);

#define REGISTER_MODELING_TOOL_COMMAND(ToolCommandInfo, ToolName, ToolTip) \
		UI_COMMAND(ToolCommandInfo, ToolName, ToolTip, EUserInterfaceActionType::Button, FInputChord()); \
		RegisteredTools.Add(ToolCommandInfo);

	// These part of the asset editor UI
	UI_COMMAND(ApplyChanges, "Apply", "Apply changes to output", EUserInterfaceActionType::Button, FInputChord());
	
	// These get linked to various tool buttons.
	REGISTER_MODELING_TOOL_COMMAND(AutoCalTool, "AutoCalTool", "Auto Calulate Overlay");

	// These allow us to link up to pressed keys

	// These get used in viewport buttons
	UI_COMMAND(XChannel, "XChannel", "Display X channel", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(YChannel, "YChannel", "Display Y channel", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(ZChannel, "ZChannel", "Display Z channel", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(WChannel, "WChannel", "Display W channel", EUserInterfaceActionType::ToggleButton, FInputChord());

	UI_COMMAND(Compact,  "Compact",  "Display UV in a compact way", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(Iterable, "Iterable", "Display UV in a Iterable way", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(Exploded, "Exploded", "Display UV in a Exploded way", EUserInterfaceActionType::ToggleButton, FInputChord());

	UI_COMMAND(DefaultLight, "DefaultLight", "Use default lighting to display", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(Emissive, "Emissive", "Use Emissive to display", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(Translucency, "Translucency", "Use Translucency to display", EUserInterfaceActionType::ToggleButton, FInputChord()); 
	UI_COMMAND(Transition, "Transition", "Use Transition to display", EUserInterfaceActionType::ToggleButton, FInputChord());
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FFDOverlayEditorModeCommands::GetCommands()
{
	return FFDOverlayEditorModeCommands::Get().Commands;
}

#undef LOCTEXT_NAMESPACE
