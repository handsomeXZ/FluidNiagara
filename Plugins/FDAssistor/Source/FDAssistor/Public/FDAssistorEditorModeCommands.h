// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

/**
 * This class contains info about the full set of commands used in this editor mode.
 */
class FFDAssistorEditorModeCommands : public TCommands<FFDAssistorEditorModeCommands>
{
public:
	FFDAssistorEditorModeCommands();

	virtual void RegisterCommands() override;
	static TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetCommands();
public:
	// For connecting ModelingMode with FDEditor
	TSharedPtr<FUICommandInfo> LaunchFDFBOverlayEditor;
protected:
	TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> Commands;
};