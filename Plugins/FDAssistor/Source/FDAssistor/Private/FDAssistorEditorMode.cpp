// Copyright Epic Games, Inc. All Rights Reserved.

#include "FDAssistorEditorMode.h"
#include "EdModeInteractiveToolsContext.h"
#include "InputState.h"
#include "InteractiveToolManager.h"
#include "ModelingToolsActions.h"
#include "Framework/Commands/UICommandList.h"

#include "FDAssistorEditorModeCommands.h"
#include "FDAssistorEditorModeToolkit.h"

#include "EditorModeManager.h"
#include "Selection.h"

#include "FDOverlayEditorSubsystem.h"


// step 2: register a ToolBuilder in FFDAssistorEditorMode::Enter() below

#define LOCTEXT_NAMESPACE "FDAssistorEditorMode"

const FEditorModeID UFDAssistorEditorMode::EM_FDAssistorEditorModeId = TEXT("EM_FDAssistorEditorMode");

FString UFDAssistorEditorMode::ToolName = TEXT("FDAssistor_Tools");


UFDAssistorEditorMode::UFDAssistorEditorMode()
{
	FModuleManager::Get().LoadModule("EditorStyle");

	// appearance and icon in the editing mode ribbon can be customized here
	Info = FEditorModeInfo(UFDAssistorEditorMode::EM_FDAssistorEditorModeId,
		LOCTEXT("ModeName", "FDAssistor"),
		FSlateIcon(),
		true);
}


UFDAssistorEditorMode::~UFDAssistorEditorMode()
{
}


void UFDAssistorEditorMode::ActorSelectionChangeNotify()
{
}

void UFDAssistorEditorMode::Enter()
{
	UEdMode::Enter();

	/*const FFDAssistorEditorModeCommands& SampleToolCommands = FFDAssistorEditorModeCommands::Get();

	RegisterTool(SampleToolCommands.SimpleTool, SimpleToolName, NewObject<UFDAssistorSimpleToolBuilder>(this));
	RegisterTool(SampleToolCommands.InteractiveTool, InteractiveToolName, NewObject<UFDAssistorInteractiveToolBuilder>(this));*/
	RegisterFDEditor();

	// active tool type is not relevant here, we just set to default
	GetToolManager()->SelectActiveToolType(EToolSide::Left, ToolName);
}

void UFDAssistorEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FFDAssistorEditorModeToolkit);
}


void UFDAssistorEditorMode::RegisterFDEditor()
{
	
	const FFDAssistorEditorModeCommands& ToolManagerCommands = FFDAssistorEditorModeCommands::Get();
	UFDOverlayEditorSubsystem* FDOverlaySubsystem = GEditor->GetEditorSubsystem<UFDOverlayEditorSubsystem>();

	check(FDOverlaySubsystem);

	const TSharedRef<FUICommandList>& CommandList = Toolkit->GetToolkitCommands();
	CommandList->MapAction(ToolManagerCommands.LaunchFDOverlayEditor,
		FExecuteAction::CreateLambda([this, FDOverlaySubsystem]()
			{
				EToolsContextScope ToolScope = GetDefaultToolScope();
				UEditorInteractiveToolsContext* UseToolsContext = GetInteractiveToolsContext(ToolScope);
				if (ensure(UseToolsContext != nullptr) == false)
				{
					return;
				}

				TArray<UObject*> SelectedActors, SelectedComponents;
				TArray<TObjectPtr<UObject>> SelectedObjects;
				UseToolsContext->GetParentEditorModeManager()->GetSelectedActors()->GetSelectedObjects(SelectedActors);
				UseToolsContext->GetParentEditorModeManager()->GetSelectedComponents()->GetSelectedObjects(SelectedComponents);
				SelectedObjects.Append(SelectedActors);
				SelectedObjects.Append(SelectedComponents);
				FDOverlaySubsystem->LaunchFDOverlayEditor(SelectedObjects);
			}),
		FCanExecuteAction::CreateLambda([this, FDOverlaySubsystem]()
			{
				EToolsContextScope ToolScope = GetDefaultToolScope();
				UEditorInteractiveToolsContext* UseToolsContext = GetInteractiveToolsContext(ToolScope);
				if (ensure(UseToolsContext != nullptr) == false)
				{
					return false;
				}

				TArray<UObject*> SelectedActors, SelectedComponents;
				TArray<TObjectPtr<UObject>> SelectedObjects;
				UseToolsContext->GetParentEditorModeManager()->GetSelectedActors()->GetSelectedObjects(SelectedActors);
				UseToolsContext->GetParentEditorModeManager()->GetSelectedComponents()->GetSelectedObjects(SelectedComponents);
				SelectedObjects.Append(SelectedActors);
				SelectedObjects.Append(SelectedComponents);
				return FDOverlaySubsystem->CanLaunchFDOverlayEditor(SelectedObjects);
			})
		);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UFDAssistorEditorMode::GetModeCommands() const
{
	return FFDAssistorEditorModeCommands::Get().GetCommands();
}

#undef LOCTEXT_NAMESPACE
