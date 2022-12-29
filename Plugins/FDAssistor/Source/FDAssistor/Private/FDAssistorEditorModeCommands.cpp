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
	// ������ñ���ʱ������ɣ���ΪUI_COMMAND��չΪLOCTEXT��
#define REGISTER_MODELING_TOOL_COMMAND(ToolCommandInfo, ToolName, ToolTip) \
		UI_COMMAND(ToolCommandInfo, ToolName, ToolTip, EUserInterfaceActionType::Button, FInputChord()); \
		RegisteredTools.Add(ToolCommandInfo);

	// ����ֱ����ɵģ�������ʹ�� REGISTER_�꣬��Ϊ���ǲ�ϣ������ӵ������б��ʹ���л���ť
	//UI_COMMAND(LaunchFDFBOverlayEditor, "FDFBOverlayEditor", "Launch FDFBOverlay asset editor", EUserInterfaceActionType::Button, FInputChord());

	// ֱ�Ӽ��� Commands�б��Ҳ���Ҫ�Լ�ʵ�� BuildToolPalette����Ϊ���Ѿ�ȷ���˰�ť���͵�һ������������֮ǰ����
	REGISTER_MODELING_TOOL_COMMAND(LaunchFDFBOverlayEditor, "FDFBOverlayEditor", "Launch FDFBOverlay asset editor");


}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FFDAssistorEditorModeCommands::GetCommands()
{
	return FFDAssistorEditorModeCommands::Get().Commands;
}

#undef LOCTEXT_NAMESPACE
