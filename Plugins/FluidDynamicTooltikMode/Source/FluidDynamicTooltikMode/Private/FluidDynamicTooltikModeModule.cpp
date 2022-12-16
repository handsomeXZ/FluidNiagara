// Copyright Epic Games, Inc. All Rights Reserved.

#include "FluidDynamicTooltikModeModule.h"
#include "FluidDynamicTooltikModeEditorModeCommands.h"

#define LOCTEXT_NAMESPACE "FluidDynamicTooltikModeModule"

void FFluidDynamicTooltikModeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FFluidDynamicTooltikModeEditorModeCommands::Register();
}

void FFluidDynamicTooltikModeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FFluidDynamicTooltikModeEditorModeCommands::Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFluidDynamicTooltikModeModule, FluidDynamicTooltikModeEditorMode)