// Copyright HandsomeCheese. All Rights Reserved.

#include "FDOverlayEditorModule.h"

#include "FDOverlayEditorModeCommands.h"
#include "Tools/FDOverlayStyle.h"

#define LOCTEXT_NAMESPACE "FDOverlayEditorModule"

void FFDOverlayEditorModule::StartupModule()
{
	FFDOverlayStyle::Get();
	FFDOverlayEditorModeCommands::Register();
}

void FFDOverlayEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FFDOverlayEditorModeCommands::Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFDOverlayEditorModule, FDOverlayEditor)