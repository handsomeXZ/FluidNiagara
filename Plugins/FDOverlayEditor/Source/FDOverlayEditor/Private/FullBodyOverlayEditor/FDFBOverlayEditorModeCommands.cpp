// Copyright Epic Games, Inc. All Rights Reserved.

#include "FullBodyOverlayEditor/FDFBOverlayEditorModeCommands.h"
//#include "FullBodyOverlayEditor/FDFBOverlayEditorMode.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "FDFBOverlayEditorModeCommands"

FFDFBOverlayEditorModeCommands::FFDFBOverlayEditorModeCommands()
	: TCommands<FFDFBOverlayEditorModeCommands>("FDFBOverlayEditorMode",
		NSLOCTEXT("FDFBOverlayEditorMode", "FDFBOverlayEditorModeCommands", "FDFBOverlay Editor Mode"),
		NAME_None,
		FEditorStyle::GetStyleSetName())
{
}

void FFDFBOverlayEditorModeCommands::RegisterCommands()
{
	TArray <TSharedPtr<FUICommandInfo>>& ToolCommands = Commands.FindOrAdd(NAME_Default);


	// These are part of the asset editor UI
	UI_COMMAND(ApplyChanges, "Apply", "Apply changes to original meshes", EUserInterfaceActionType::Button, FInputChord());



}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FFDFBOverlayEditorModeCommands::GetCommands()
{
	return FFDFBOverlayEditorModeCommands::Get().Commands;
}

#undef LOCTEXT_NAMESPACE
