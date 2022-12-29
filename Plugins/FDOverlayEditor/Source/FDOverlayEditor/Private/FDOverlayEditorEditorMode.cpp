// Copyright Epic Games, Inc. All Rights Reserved.

#include "FDOverlayEditorEditorMode.h"
#include "FDOverlayEditorEditorModeToolkit.h"
#include "EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "FDOverlayEditorEditorModeCommands.h"


//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
// AddYourTool Step 1 - include the header file for your Tools here
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
#include "Tools/FDOverlayEditorSimpleTool.h"
#include "Tools/FDOverlayEditorInteractiveTool.h"

// step 2: register a ToolBuilder in FFDOverlayEditorEditorMode::Enter() below


#define LOCTEXT_NAMESPACE "FDOverlayEditorEditorMode"

const FEditorModeID UFDOverlayEditorEditorMode::EM_FDOverlayEditorEditorModeId = TEXT("EM_FDOverlayEditorEditorMode");

FString UFDOverlayEditorEditorMode::SimpleToolName = TEXT("FDOverlayEditor_ActorInfoTool");
FString UFDOverlayEditorEditorMode::InteractiveToolName = TEXT("FDOverlayEditor_MeasureDistanceTool");


UFDOverlayEditorEditorMode::UFDOverlayEditorEditorMode()
{
	FModuleManager::Get().LoadModule("EditorStyle");

	// appearance and icon in the editing mode ribbon can be customized here
	Info = FEditorModeInfo(UFDOverlayEditorEditorMode::EM_FDOverlayEditorEditorModeId,
		LOCTEXT("ModeName", "FDOverlayEditor"),
		FSlateIcon(),
		true);
}


UFDOverlayEditorEditorMode::~UFDOverlayEditorEditorMode()
{
}


void UFDOverlayEditorEditorMode::ActorSelectionChangeNotify()
{
}

void UFDOverlayEditorEditorMode::Enter()
{
	UEdMode::Enter();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// AddYourTool Step 2 - register the ToolBuilders for your Tools here.
	// The string name you pass to the ToolManager is used to select/activate your ToolBuilder later.
	//////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////// 
	const FFDOverlayEditorEditorModeCommands& SampleToolCommands = FFDOverlayEditorEditorModeCommands::Get();

	RegisterTool(SampleToolCommands.SimpleTool, SimpleToolName, NewObject<UFDOverlayEditorSimpleToolBuilder>(this));
	RegisterTool(SampleToolCommands.InteractiveTool, InteractiveToolName, NewObject<UFDOverlayEditorInteractiveToolBuilder>(this));

	// active tool type is not relevant here, we just set to default
	GetToolManager()->SelectActiveToolType(EToolSide::Left, SimpleToolName);
}

void UFDOverlayEditorEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FFDOverlayEditorEditorModeToolkit);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UFDOverlayEditorEditorMode::GetModeCommands() const
{
	return FFDOverlayEditorEditorModeCommands::Get().GetCommands();
}

#undef LOCTEXT_NAMESPACE
