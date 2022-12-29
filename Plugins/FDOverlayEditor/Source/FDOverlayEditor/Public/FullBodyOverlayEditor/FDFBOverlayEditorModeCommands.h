// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

/**
 * This class contains info about the full set of commands used in this editor mode.
 */
class FFDFBOverlayEditorModeCommands : public TCommands<FFDFBOverlayEditorModeCommands>
{
public:
	FFDFBOverlayEditorModeCommands();

	virtual void RegisterCommands() override;
	static TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetCommands();

public:

	// TODO: 还未在 Mode里为下面这些CommandInfo 调用 RegisterTool
	TSharedPtr<FUICommandInfo> ApplyChanges;

protected:
	TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> Commands;

};
