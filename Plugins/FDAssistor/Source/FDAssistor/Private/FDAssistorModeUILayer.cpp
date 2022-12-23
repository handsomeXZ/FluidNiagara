// Copyright Epic Games, Inc. All Rights Reserved.

#include "FDAssistorModeUILayer.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Toolkits/IToolkit.h"
#include "FDAssistorAssetEditorTooltik.h"
#include "FDAssistorModule.h"


void UFDAssistorEditorUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	FFDAssistorModule& FDAssistorModule = FModuleManager::GetModuleChecked<FFDAssistorModule>("FDAssistor");
	FDAssistorModule.OnRegisterLayoutExtensions().AddUObject(this, &UFDAssistorEditorUISubsystem::RegisterLayoutExtensions);
}
void UFDAssistorEditorUISubsystem::Deinitialize()
{
	FFDAssistorModule& FDAssistorModule = FModuleManager::GetModuleChecked<FFDAssistorModule>("FDAssistor");
	FDAssistorModule.OnRegisterLayoutExtensions().RemoveAll(this);
}
void UFDAssistorEditorUISubsystem::RegisterLayoutExtensions(FLayoutExtender& Extender)
{
	FTabManager::FTab NewTab(FTabId(UAssetEditorUISubsystem::TopLeftTabID), ETabState::ClosedTab);
	Extender.ExtendStack("EditorSidePanelArea", ELayoutExtensionPosition::After, NewTab);
}

FFDAssistorEditorModeUILayer::FFDAssistorEditorModeUILayer(const IToolkitHost* InToolkitHost) :
	FAssetEditorModeUILayer(InToolkitHost)
{}

void FFDAssistorEditorModeUILayer::OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit)
{
	if (!Toolkit->IsAssetEditor())
	{
		FAssetEditorModeUILayer::OnToolkitHostingStarted(Toolkit);
		HostedToolkit = Toolkit;
		Toolkit->SetModeUILayer(SharedThis(this));
		Toolkit->RegisterTabSpawners(ToolkitHost->GetTabManager().ToSharedRef());
		RegisterModeTabSpawners();
		OnToolkitHostReadyForUI.ExecuteIfBound();
	}
}

void FFDAssistorEditorModeUILayer::OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit)
{
	if (HostedToolkit.IsValid() && HostedToolkit.Pin() == Toolkit)
	{
		FAssetEditorModeUILayer::OnToolkitHostingFinished(Toolkit);
	}
}

TSharedPtr<FWorkspaceItem> FFDAssistorEditorModeUILayer::GetModeMenuCategory() const
{
	check(FDAssistorEditorMenuCategory);
	return FDAssistorEditorMenuCategory;
}

void FFDAssistorEditorModeUILayer::SetModeMenuCategory(TSharedPtr<FWorkspaceItem> MenuCategoryIn)
{
	FDAssistorEditorMenuCategory = MenuCategoryIn;
}

