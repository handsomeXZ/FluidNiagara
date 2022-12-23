// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FDAssistorContextObject.h"

#include "FDAssistorInitializationContext.generated.h"

class UEditorInteractiveToolsContext;

/**
 * An editor-only context used specifically to pass data from the asset editor to the
 * mode when that data is not meant to be used by the tools. This allows a mode
 * to be prepped before its first Enter() call even when it doesn't exist yet.
 * 
 * Data/api's that are meant to be used by the tools would usually be placed into
 * other, non-editor-only context objects (UUVToolLivePreviewAPI, etc).
 */
UCLASS()
class UVEDITOR_API UFDAssistorInitializationContext : public UFDAssistorContextObject
{
	GENERATED_BODY()
public:
	TWeakObjectPtr<UEditorInteractiveToolsContext> LivePreviewITC;
};