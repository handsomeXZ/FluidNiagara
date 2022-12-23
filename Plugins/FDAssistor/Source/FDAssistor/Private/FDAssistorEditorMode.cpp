// Copyright Epic Games, Inc. All Rights Reserved.

#include "FDAssistorEditorMode.h"
#include "FDAssistorEditorModeToolkit.h"
#include "EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "FDAssistorEditorModeCommands.h"


//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
// AddYourTool Step 1 - include the header file for your Tools here
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
#include "Tools/FDAssistorSimpleTool.h"
#include "Tools/FDAssistorInteractiveTool.h"

// step 2: register a ToolBuilder in FFDAssistorEditorMode::Enter() below

const FEditorModeID UFDAssistorEditorMode::EM_FDAssistorModeId = TEXT("EM_FDAssistorModeId");

#define LOCTEXT_NAMESPACE "FDAssistorEditorMode"

const FEditorModeID UFDAssistorEditorMode::EM_FDAssistorEditorModeId = TEXT("EM_FDAssistorEditorMode");

FString UFDAssistorEditorMode::SimpleToolName = TEXT("FDAssistor_ActorInfoTool");
FString UFDAssistorEditorMode::InteractiveToolName = TEXT("FDAssistor_MeasureDistanceTool");


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

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// AddYourTool Step 2 - register the ToolBuilders for your Tools here.
	// The string name you pass to the ToolManager is used to select/activate your ToolBuilder later.
	//////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////// 
	const FFDAssistorEditorModeCommands& SampleToolCommands = FFDAssistorEditorModeCommands::Get();

	RegisterTool(SampleToolCommands.SimpleTool, SimpleToolName, NewObject<UFDAssistorSimpleToolBuilder>(this));
	RegisterTool(SampleToolCommands.InteractiveTool, InteractiveToolName, NewObject<UFDAssistorInteractiveToolBuilder>(this));

	// active tool type is not relevant here, we just set to default
	GetToolManager()->SelectActiveToolType(EToolSide::Left, SimpleToolName);
}

void UFDAssistorEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FFDAssistorEditorModeToolkit);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UFDAssistorEditorMode::GetModeCommands() const
{
	return FFDAssistorEditorModeCommands::Get().GetCommands();
}

void UFDAssistorEditorMode::InitializeAssetEditorContexts(UContextObjectStore& ContextStore,
	const TArray<TObjectPtr<UObject>>& AssetsIn, const TArray<FTransform>& TransformsIn, 
	FEditorViewportClient& LivePreviewViewportClient, FAssetEditorModeManager& LivePreviewModeManager, 
	UFDAssistorViewportButtonsAPI& ViewportButtonsAPI, UFDAssistorLive2DViewportAPI& FDAssistorLive2DViewportAPI)
{
	using namespace UVEditorModeLocals;

	UFDAssistorAssetInputsContext* AssetInputsContext = ContextStore.FindContext<UFDAssistorAssetInputsContext>();
	if (!AssetInputsContext)
	{
		AssetInputsContext = NewObject<UFDAssistorAssetInputsContext>();
		AssetInputsContext->Initialize(AssetsIn, TransformsIn);
		ContextStore.AddContextObject(AssetInputsContext);
	}

	UFDAssistorLive3DPreviewAPI* Live3DPreviewAPI = ContextStore.FindContext<UFDAssistorLive3DPreviewAPI>();
	if (!Live3DPreviewAPI)
	{
		Live3DPreviewAPI = NewObject<UFDAssistorLive3DPreviewAPI>();
		Live3DPreviewAPI->Initialize(
			LivePreviewModeManager.GetPreviewScene()->GetWorld(),
			LivePreviewModeManager.GetInteractiveToolsContext()->InputRouter,
			[LivePreviewViewportClientPtr = &LivePreviewViewportClient](FViewCameraState& CameraStateOut) {
				GetCameraState(*LivePreviewViewportClientPtr, CameraStateOut);
			},
			[LivePreviewViewportClientPtr = &LivePreviewViewportClient](const FAxisAlignedBox3d& BoundingBox) {
				// We check for the Viewport here because it might not be open at the time this
				// method is called, e.g. during startup with an initially closed tab. And since
				// the FocusViewportOnBox method doesn't check internally that the Viewport is
				// available, this can crash.
				if (LivePreviewViewportClientPtr && LivePreviewViewportClientPtr->Viewport)
				{
					LivePreviewViewportClientPtr->FocusViewportOnBox((FBox)BoundingBox, true);
				}
			}
			);
		ContextStore.AddContextObject(LivePreviewAPI);
	}

	// Prep the editor-only context that we use to pass things to the mode.
	if (!ContextStore.FindContext<UFDAssistorInitializationContext>())
	{
		UFDAssistorInitializationContext* InitContext = NewObject<UFDAssistorInitializationContext>();
		InitContext->LivePreviewITC = Cast<UFDAssistorInitializationContext>(LivePreviewModeManager.GetInteractiveToolsContext());
		ContextStore.AddContextObject(InitContext);
	}

	if (!ContextStore.FindContext<UFDAssistorViewportButtonsAPI>())
	{
		ContextStore.AddContextObject(&ViewportButtonsAPI);
	}

	if (!ContextStore.FindContext<UFDAssistorLive2DViewportAPI>())
	{
		ContextStore.AddContextObject(&FDAssistorLive2DViewportAPI);
	}


}



#undef LOCTEXT_NAMESPACE
