// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tools/UEdMode.h"
#include "FDAssistorEditorMode.generated.h"

/**
 * This class provides an example of how to extend a UEdMode to add some simple tools
 * using the InteractiveTools framework. The various UEdMode input event handlers (see UEdMode.h)
 * forward events to a UEdModeInteractiveToolsContext instance, which
 * has all the logic for interacting with the InputRouter, ToolManager, etc.
 * The functions provided here are the minimum to get started inserting some custom behavior.
 * Take a look at the UEdMode markup for more extensibility options.
 */
UCLASS()
class UFDAssistorEditorMode : public UEdMode
{
	GENERATED_BODY()

public:
	const static FEditorModeID EM_FDAssistorModeId;

	static FString SimpleToolName;
	static FString InteractiveToolName;

	UFDAssistorEditorMode();
	virtual ~UFDAssistorEditorMode();

	/** UEdMode interface */
	virtual void Enter() override;
	virtual void ActorSelectionChangeNotify() override;
	virtual void CreateToolkit() override;
	virtual TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetModeCommands() const override;


	/**
	 * Called by an asset editor so that a created instance of the mode has all the data it needs on Enter() to initialize itself.
	 */
	static void InitializeAssetEditorContexts(UContextObjectStore& ContextStore,
		const TArray<TObjectPtr<UObject>>& AssetsIn, const TArray<FTransform>& TransformsIn,
		FEditorViewportClient& LivePreviewViewportClient, FAssetEditorModeManager& LivePreviewModeManager,
		UFDAssistorViewportButtonsAPI& ViewportButtonsAPI, UFDAssistorLive2DViewportAPI& FDAssistorLive2DViewportAPI);

	// Asset management
	// TODO: 还未实现
	bool CanApplyChanges() const;
	void ApplyChanges();

	// TODO: 还未实现
	void FocusLivePreviewCameraOnSelection();
protected:
	
};
