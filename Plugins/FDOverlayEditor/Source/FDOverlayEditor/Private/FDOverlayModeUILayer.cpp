// Copyright HandsomeCheese. All Rights Reserved.

#include "FDOverlayModeUILayer.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Toolkits/IToolkit.h"
#include "FDOverlayAssetEditorTooltik.h"
#include "FDOverlayEditorModule.h"


void UFDOverlayEditorUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	FFDOverlayEditorModule& FDOverlayModule = FModuleManager::GetModuleChecked<FFDOverlayEditorModule>("FDOverlayEditor");
	FDOverlayModule.OnRegisterLayoutExtensions().AddUObject(this, &UFDOverlayEditorUISubsystem::RegisterLayoutExtensions);
}
void UFDOverlayEditorUISubsystem::Deinitialize()
{
	FFDOverlayEditorModule& FDOverlayModule = FModuleManager::GetModuleChecked<FFDOverlayEditorModule>("FDOverlayEditor");
	FDOverlayModule.OnRegisterLayoutExtensions().RemoveAll(this);
}
void UFDOverlayEditorUISubsystem::RegisterLayoutExtensions(FLayoutExtender& Extender)
{
	FTabManager::FTab NewTab(FTabId(UAssetEditorUISubsystem::TopLeftTabID), ETabState::ClosedTab);
	Extender.ExtendStack("EditorSidePanelArea", ELayoutExtensionPosition::After, NewTab);
}

FFDOverlayEditorModeUILayer::FFDOverlayEditorModeUILayer(const IToolkitHost* InToolkitHost) :
	FAssetEditorModeUILayer(InToolkitHost)
{}

void FFDOverlayEditorModeUILayer::OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit)
{
	// ��Ϊ���Ǵ� FDAssistorģʽ�л����� FDOverlayEditorģʽ�� Tooltik ҲҪ�����л�
	// �ú�����Ŀ�ģ�
	//		1. �����幤�ߵ� tab spawner
	//		2. �����µ���幤���Լ���������
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

void FFDOverlayEditorModeUILayer::OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit)
{
	if (HostedToolkit.IsValid() && HostedToolkit.Pin() == Toolkit)
	{
		FAssetEditorModeUILayer::OnToolkitHostingFinished(Toolkit);
	}
}

TSharedPtr<FWorkspaceItem> FFDOverlayEditorModeUILayer::GetModeMenuCategory() const
{
	check(FDOverlayEditorMenuCategory);
	return FDOverlayEditorMenuCategory;
}

void FFDOverlayEditorModeUILayer::SetModeMenuCategory(TSharedPtr<FWorkspaceItem> MenuCategoryIn)
{
	FDOverlayEditorMenuCategory = MenuCategoryIn;
}

