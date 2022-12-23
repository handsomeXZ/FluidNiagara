// Copyright Epic Games, Inc. All Rights Reserved.

#include "FDAssistorModule.h"
#include "FDAssistorEditorModeCommands.h"

#define LOCTEXT_NAMESPACE "FDAssistorModule"

void FFDAssistorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FFDAssistorEditorModeCommands::Register();
}

void FFDAssistorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FFDAssistorEditorModeCommands::Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFDAssistorModule, FDAssistorEditorMode)