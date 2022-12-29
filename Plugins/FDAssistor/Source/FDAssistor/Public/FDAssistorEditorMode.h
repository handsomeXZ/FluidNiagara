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
	// ԭ��Ӧ��ʹ�� RegisterModularFeature() �� GetModularFeatureImplementations() �ķ��������
	// ������Ҫ֪��Ǳ�����������ڲ������ʱ�ɹ����á����������ҽ�����ȫ������ SubSystem�ˣ�
	// ��ô����Ҫ�� .uplugin ��ȷ�������Ͳ���ɹ����غ���ܳɹ����á�
	// �������ڽ����Ǻϳ���һ����������Բ�����Ҫȷ�����������
	// ���� Subsystem��ʹ��
	void RegisterFDEditor();
};