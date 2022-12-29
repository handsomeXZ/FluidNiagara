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
	TArray <TSharedPtr<FUICommandInfo>>& RegisteredTools = Commands.FindOrAdd(NAME_Default);
	// 这必须用编译时宏来完成，因为UI_COMMAND扩展为LOCTEXT宏
#define REGISTER_MODELING_TOOL_COMMAND(ToolCommandInfo, ToolName, ToolTip) \
		UI_COMMAND(ToolCommandInfo, ToolName, ToolTip, EUserInterfaceActionType::Button, FInputChord()); \
		RegisteredTools.Add(ToolCommandInfo);

	// 这是直接完成的，而不是使用 REGISTER_宏，因为我们不希望它添加到工具列表或使用切换按钮
	//UI_COMMAND(LaunchFDFBOverlayEditor, "FDFBOverlayEditor", "Launch FDFBOverlay asset editor", EUserInterfaceActionType::Button, FInputChord());

	// 直接加入 Commands列表，我不需要自己实现 BuildToolPalette，因为我已经确保了按钮类型单一且依赖已在这之前加载
	REGISTER_MODELING_TOOL_COMMAND(LaunchFDFBOverlayEditor, "FDFBOverlayEditor", "Launch FDFBOverlay asset editor");


}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FFDAssistorEditorModeCommands::GetCommands()
{
	return FFDAssistorEditorModeCommands::Get().Commands;
}

#undef LOCTEXT_NAMESPACE
