// Copyright Epic Games, Inc. All Rights Reserved.

#include "FluidNiagaraSimulationStageCustomization.h"

#include "Modules/ModuleManager.h"

//Customization
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
//Widgets
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/SBoxPanel.h"
//#include "Widgets/SNiagaraDebugger.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBorder.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
///Niagara
#include "NiagaraEditorModule.h"

#define LOCTEXT_NAMESPACE "FluidNiagaraSimulationStageCustomization"

FFluidNiagaraSimulationStageGenericCustomization::~FFluidNiagaraSimulationStageGenericCustomization()
{
	if (UFluidNiagaraSimulationStageGeneric* SimStage = WeakSimStage.Get())
	{
		SimStage->OnChanged().RemoveAll(this);
	}
}

TSharedRef<IDetailCustomization> FFluidNiagaraSimulationStageGenericCustomization::MakeInstance()
{
	return MakeShareable(new FFluidNiagaraSimulationStageGenericCustomization());
}

void FFluidNiagaraSimulationStageGenericCustomization::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder)
{
	WeakDetailBuilder = DetailBuilder;
	CustomizeDetails(*DetailBuilder);
}

void FFluidNiagaraSimulationStageGenericCustomization::CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder)
{
	// We only support customization on 1 object
	TArray<TWeakObjectPtr<UObject>> ObjectsCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsCustomized);
	if (ObjectsCustomized.Num() != 1 || !ObjectsCustomized[0]->IsA<UFluidNiagaraSimulationStageGeneric>())
	{
		return;
	}

	UFluidNiagaraSimulationStageGeneric* SimStage = CastChecked<UFluidNiagaraSimulationStageGeneric>(ObjectsCustomized[0]);
	WeakSimStage = SimStage;

	SimStage->OnChanged().AddRaw(this, &FFluidNiagaraSimulationStageGenericCustomization::OnPropertyChanged);

	static const FName CategoryName = TEXT("Simulation Stage");
	static const FName ParticleStageName = TEXT("Particle Parameters");
	static const FName DataInterfaceStageName = TEXT("DataInterface Parameters");
	IDetailCategoryBuilder& SimStageCategory = DetailBuilder.EditCategory(CategoryName);
	IDetailCategoryBuilder& ParticleStageCategory = DetailBuilder.EditCategory(ParticleStageName);
	IDetailCategoryBuilder& DataInterfaceStageCategory = DetailBuilder.EditCategory(DataInterfaceStageName);

	// Hide all
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, EnabledBinding));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, ElementCountXBinding));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, ElementCountYBinding));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, ElementCountZBinding));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, IterationSource));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, Iterations));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, NumIterationsBinding));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, ExecuteBehavior));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, bDisablePartialParticleUpdate));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, DataInterface));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, bParticleIterationStateEnabled));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, ParticleIterationStateBinding));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, ParticleIterationStateRange));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, bGpuDispatchForceLinear));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, bOverrideGpuDispatchType));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, OverrideGpuDispatchType));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, bOverrideGpuDispatchNumThreads));
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, OverrideGpuDispatchNumThreads));

	// Show properties in the order we want them to appear
	SimStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, EnabledBinding)));
	SimStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, Iterations)));
	SimStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, NumIterationsBinding)));
	SimStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, IterationSource)));

	DataInterfaceStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, bOverrideGpuDispatchType)));
	if (SimStage->bOverrideGpuDispatchType)
	{
		DataInterfaceStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, OverrideGpuDispatchType)));
		// Always true as we always dispatch across at least 1 dimension
		//if (SimStage->OverrideGpuDispatchType >= ENiagaraGpuDispatchType::OneD)
		{
			DataInterfaceStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, ElementCountXBinding)));
		}
		if (SimStage->OverrideGpuDispatchType >= ENiagaraGpuDispatchType::TwoD)
		{
			DataInterfaceStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, ElementCountYBinding)));
		}
		if (SimStage->OverrideGpuDispatchType >= ENiagaraGpuDispatchType::ThreeD)
		{
			DataInterfaceStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, ElementCountZBinding)));
		}
	}

	if (SimStage->IterationSource == ENiagaraIterationSource::Particles)
	{
		ParticleStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, bDisablePartialParticleUpdate)));
		ParticleStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, bParticleIterationStateEnabled)));
		if (SimStage->bParticleIterationStateEnabled)
		{
			ParticleStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, ParticleIterationStateBinding)));
			ParticleStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, ParticleIterationStateRange)));
		}
	}
	else
	{
		DataInterfaceStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, DataInterface)));
		DataInterfaceStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, ExecuteBehavior)));

		DataInterfaceStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, bGpuDispatchForceLinear)));

		DataInterfaceStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, bOverrideGpuDispatchNumThreads)));
		if (SimStage->bOverrideGpuDispatchNumThreads)
		{
			DataInterfaceStageCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFluidNiagaraSimulationStageGeneric, OverrideGpuDispatchNumThreads)));
		}
	}
}

void FFluidNiagaraSimulationStageGenericCustomization::OnPropertyChanged()
{
	if (IDetailLayoutBuilder* DetailBuilder = WeakDetailBuilder.Pin().Get())
	{
		DetailBuilder->ForceRefreshDetails();
	}
}

#undef LOCTEXT_NAMESPACE
