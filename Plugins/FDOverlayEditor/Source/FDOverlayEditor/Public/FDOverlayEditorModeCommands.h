// Copyright HandsomeCheese. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

/**
 * This class contains info about the full set of commands used in this editor mode.
 */
class FFDOverlayEditorModeCommands : public TCommands<FFDOverlayEditorModeCommands>
{
public:
	FFDOverlayEditorModeCommands();

	virtual void RegisterCommands() override;
	static TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetCommands();

public:

	// TODO: 还未在 Mode里为下面这些CommandInfo 调用 RegisterTool
	TSharedPtr<FUICommandInfo> ApplyChanges;

	// 这些被链接到各种工具按钮。
	TSharedPtr<FUICommandInfo> AutoCalTool;

	// 这些在视口按钮中使用
	TSharedPtr<FUICommandInfo> XChannel;
	TSharedPtr<FUICommandInfo> YChannel;
	TSharedPtr<FUICommandInfo> ZChannel;
	TSharedPtr<FUICommandInfo> WChannel;

	TSharedPtr<FUICommandInfo> Compact;
	TSharedPtr<FUICommandInfo> Iterable;
	TSharedPtr<FUICommandInfo> Exploded;

	TSharedPtr<FUICommandInfo> DefaultLight;
	TSharedPtr<FUICommandInfo> Emissive;
	TSharedPtr<FUICommandInfo> Translucency;
	TSharedPtr<FUICommandInfo> Transition;
protected:
	TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> Commands;

};
