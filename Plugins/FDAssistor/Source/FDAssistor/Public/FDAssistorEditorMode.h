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
UCLASS(Transient)
class FDASSISTOR_API UFDAssistorEditorMode : public UEdMode
{
	GENERATED_BODY()

public:
	const static FEditorModeID EM_FDAssistorEditorModeId;

	static FString ToolName;

	UFDAssistorEditorMode();
	virtual ~UFDAssistorEditorMode();

	/** UEdMode interface */
	virtual void Enter() override;
	virtual void ActorSelectionChangeNotify() override;
	virtual void CreateToolkit() override;

	virtual TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetModeCommands() const override;
protected:
	// 原本应该使用 RegisterModularFeature() 和 GetModularFeatureImplementations() 的方案来解耦，
	// 即不需要知道潜在依赖即可在插件可用时成功调用。但是现在我将功能全部丢给 SubSystem了，
	// 那么就需要在 .uplugin 内确保依赖和插件成功加载后才能成功调用。
	// 但我现在将他们合成了一个插件，所以并不需要确保插件依赖。
	// 保留 Subsystem的使用
	void RegisterFDEditor();
};
