// Copyright Epic Games, Inc. All Rights Reserved.

#include "FDOverlayEditorModule.h"
#include "FDOverlayEditorEditorModeCommands.h"

#define LOCTEXT_NAMESPACE "FDOverlayEditorModule"

void FFDOverlayEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FFDOverlayEditorEditorModeCommands::Register();
}

void FFDOverlayEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FFDOverlayEditorEditorModeCommands::Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFDOverlayEditorModule, FDOverlayEditor)