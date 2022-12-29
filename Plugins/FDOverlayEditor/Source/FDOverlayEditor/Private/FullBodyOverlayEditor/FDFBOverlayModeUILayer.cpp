// Copyright Epic Games, Inc. All Rights Reserved.

#include "FullBodyOverlayEditor/FDFBOverlayModeUILayer.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Toolkits/IToolkit.h"
#include "FullBodyOverlayEditor/FDFBOverlayAssetEditorTooltik.h"
#include "FDAssistorModule.h"


void UFDFBOverlayEditorUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	FFDAssistorModule& FDAssistorModule = FModuleManager::GetModuleChecked<FFDAssistorModule>("FDAssistor");
	FDAssistorModule.OnRegisterLayoutExtensions().AddUObject(this, &UFDFBOverlayEditorUISubsystem::RegisterLayoutExtensions);
}
void UFDFBOverlayEditorUISubsystem::Deinitialize()
{
	FFDAssistorModule& FDAssistorModule = FModuleManager::GetModuleChecked<FFDAssistorModule>("FDAssistor");
	FDAssistorModule.OnRegisterLayoutExtensions().RemoveAll(this);
}
void UFDFBOverlayEditorUISubsystem::RegisterLayoutExtensions(FLayoutExtender& Extender)
{
	FTabManager::FTab NewTab(FTabId(UAssetEditorUISubsystem::TopLeftTabID), ETabState::ClosedTab);
	Extender.ExtendStack("EditorSidePanelArea", ELayoutExtensionPosition::After, NewTab);
}

FFDFBOverlayEditorModeUILayer::FFDFBOverlayEditorModeUILayer(const IToolkitHost* InToolkitHost) :
	FAssetEditorModeUILayer(InToolkitHost)
{}

void FFDFBOverlayEditorModeUILayer::OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit)
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

void FFDFBOverlayEditorModeUILayer::OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit)
{
	if (HostedToolkit.IsValid() && HostedToolkit.Pin() == Toolkit)
	{
		FAssetEditorModeUILayer::OnToolkitHostingFinished(Toolkit);
	}
}

TSharedPtr<FWorkspaceItem> FFDFBOverlayEditorModeUILayer::GetModeMenuCategory() const
{
	check(FDFBOverlayEditorMenuCategory);
	return FDFBOverlayEditorMenuCategory;
}

void FFDFBOverlayEditorModeUILayer::SetModeMenuCategory(TSharedPtr<FWorkspaceItem> MenuCategoryIn)
{
	FDFBOverlayEditorMenuCategory = MenuCategoryIn;
}

