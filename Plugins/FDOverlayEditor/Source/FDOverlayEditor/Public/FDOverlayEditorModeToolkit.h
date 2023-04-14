// Copyright HandsomeCheese. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"

/**
 * This FModeToolkit just creates a basic UI panel that allows various InteractiveTools to
 * be initialized, and a DetailsView used to show properties of the active Tool.
 */
class FFDOverlayEditorModeToolkit : public FModeToolkit
{
public:
	FFDOverlayEditorModeToolkit();

	/** FModeToolkit interface */
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode) override;
	/** Returns the Mode specific tabs in the mode toolbar **/
	virtual void GetToolPaletteNames(TArray<FName>& InPaletteName) const override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;

	TSharedRef<SWidget> CreateSettingsWidget();

protected:
	// FModeToolkit
	virtual void OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool) override;
	virtual void OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool) override;

	
	TSharedRef<SWidget> CreateSettingsWidget(UObject* SettingsObject) const;
	
protected:
	TSharedPtr<SWidget> ViewportOverlayWidget;
};
