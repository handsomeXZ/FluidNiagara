// Copyright Epic Games, Inc. All Rights Reserved.

#include "FluidDynamic.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"
#include "PropertyEditorModule.h"
#include "FluidNiagaraSimStage.h"
#include "FluidNiagaraSimulationStageCustomization.h"

#define LOCTEXT_NAMESPACE "FFluidDynamicModule"

void FFluidDynamicModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("FluidDynamic"))->GetBaseDir(), TEXT("Shader"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/FluidDynamic"), PluginShaderDir);
	/*FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(
		UFluidNiagaraSimulationStageGeneric::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FFluidNiagaraSimulationStageGenericCustomization::MakeInstance));*/
}

void FFluidDynamicModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFluidDynamicModule, FluidDynamic)