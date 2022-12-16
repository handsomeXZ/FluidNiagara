// Copyright Epic Games, Inc. All Rights Reserved.

#include "FluidDynamicTooltikModeEditorMode.h"
#include "FluidDynamicTooltikModeEditorModeToolkit.h"
#include "EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "FluidDynamicTooltikModeEditorModeCommands.h"


//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
// AddYourTool Step 1 - include the header file for your Tools here
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
#include "Tools/FluidDynamicTooltikModeSimpleTool.h"
#include "Tools/FluidDynamicTooltikModeInteractiveTool.h"

// step 2: register a ToolBuilder in FFluidDynamicTooltikModeEditorMode::Enter() below


#define LOCTEXT_NAMESPACE "FluidDynamicTooltikModeEditorMode"

const FEditorModeID UFluidDynamicTooltikModeEditorMode::EM_FluidDynamicTooltikModeEditorModeId = TEXT("EM_FluidDynamicTooltikModeEditorMode");

FString UFluidDynamicTooltikModeEditorMode::SimpleToolName = TEXT("FluidDynamicTooltikMode_ActorInfoTool");
FString UFluidDynamicTooltikModeEditorMode::InteractiveToolName = TEXT("FluidDynamicTooltikMode_MeasureDistanceTool");


UFluidDynamicTooltikModeEditorMode::UFluidDynamicTooltikModeEditorMode()
{
	FModuleManager::Get().LoadModule("EditorStyle");

	// appearance and icon in the editing mode ribbon can be customized here
	Info = FEditorModeInfo(UFluidDynamicTooltikModeEditorMode::EM_FluidDynamicTooltikModeEditorModeId,
		LOCTEXT("ModeName", "FluidDynamicTooltikMode"),
		FSlateIcon(),
		true);
}


UFluidDynamicTooltikModeEditorMode::~UFluidDynamicTooltikModeEditorMode()
{
}


void UFluidDynamicTooltikModeEditorMode::ActorSelectionChangeNotify()
{
}

void UFluidDynamicTooltikModeEditorMode::Enter()
{
	UEdMode::Enter();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// AddYourTool Step 2 - register the ToolBuilders for your Tools here.
	// The string name you pass to the ToolManager is used to select/activate your ToolBuilder later.
	//////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////// 
	const FFluidDynamicTooltikModeEditorModeCommands& SampleToolCommands = FFluidDynamicTooltikModeEditorModeCommands::Get();

	RegisterTool(SampleToolCommands.SimpleTool, SimpleToolName, NewObject<UFluidDynamicTooltikModeSimpleToolBuilder>(this));
	RegisterTool(SampleToolCommands.InteractiveTool, InteractiveToolName, NewObject<UFluidDynamicTooltikModeInteractiveToolBuilder>(this));

	// active tool type is not relevant here, we just set to default
	GetToolManager()->SelectActiveToolType(EToolSide::Left, SimpleToolName);
}

void UFluidDynamicTooltikModeEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FFluidDynamicTooltikModeEditorModeToolkit);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UFluidDynamicTooltikModeEditorMode::GetModeCommands() const
{
	return FFluidDynamicTooltikModeEditorModeCommands::Get().GetCommands();
}

#undef LOCTEXT_NAMESPACE
