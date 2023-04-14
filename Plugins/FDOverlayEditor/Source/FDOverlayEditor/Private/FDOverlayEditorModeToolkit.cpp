// Copyright HandsomeCheese. All Rights Reserved.

#include "FDOverlayEditorModeToolkit.h"

#include "Engine/Selection.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"

#include "FDOverlayEditorMode.h"

#define LOCTEXT_NAMESPACE "FDOverlayEditoModeToolkit"

FFDOverlayEditorModeToolkit::FFDOverlayEditorModeToolkit()
{
}

void FFDOverlayEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);

	// Set up the overlay. Largely copied from ModelingToolsEditorModeToolkit.
	// TODO: We could put some of the shared code in some common place.
	//SAssignNew(ViewportOverlayWidget, SHorizontalBox)


}

void FFDOverlayEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add(NAME_Default);
}

FName FFDOverlayEditorModeToolkit::GetToolkitFName() const
{
	return FName("FDOverlayEditorMode");
}

FText FFDOverlayEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "FDOverlayEditorMode Toolkit");
}

void FFDOverlayEditorModeToolkit::OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	FModeToolkit::OnToolStarted(Manager, Tool);

	UFDOverlayEditorMode* Mode = Cast<UFDOverlayEditorMode>(GetScriptableEditorMode());
	//GetToolkitHost()->AddViewportOverlayWidget(ViewportOverlayWidget.ToSharedRef());
}

void FFDOverlayEditorModeToolkit::OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	FModeToolkit::OnToolEnded(Manager, Tool);
	//GetToolkitHost()->RemoveViewportOverlayWidget(ViewportOverlayWidget.ToSharedRef());

}

TSharedRef<SWidget> FFDOverlayEditorModeToolkit::CreateSettingsWidget()
{
	UFDOverlayEditorMode* Mode = Cast<UFDOverlayEditorMode>(GetScriptableEditorMode());
	return CreateSettingsWidget(Mode->GetSettingsObject());
}

TSharedRef<SWidget> FFDOverlayEditorModeToolkit::CreateSettingsWidget(UObject* SettingsObject) const
{
	TSharedRef<SBorder> GridDetailsContainer =
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("NoBorder"));

	TSharedRef<SWidget> Widget = SNew(SBorder)
		.HAlign(HAlign_Fill)
		.Padding(4)
		[
			SNew(SBox)
			.MinDesiredWidth(500)
		[
			GridDetailsContainer
		]
		];

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs GridDetailsViewArgs;
	GridDetailsViewArgs.bAllowSearch = false;
	GridDetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	GridDetailsViewArgs.bHideSelectionTip = true;
	GridDetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;
	GridDetailsViewArgs.bShowOptions = false;
	GridDetailsViewArgs.bAllowMultipleTopLevelObjects = false;

	TSharedRef<IDetailsView> GridDetailsView = PropertyEditorModule.CreateDetailView(GridDetailsViewArgs);
	GridDetailsView->SetObject(SettingsObject);
	GridDetailsContainer->SetContent(GridDetailsView);

	return Widget;

}

#undef LOCTEXT_NAMESPACE
