

#pragma once

#include "CoreMinimal.h"

#include "EditorViewportClient.h"
#include "InputBehaviorSet.h"
//#include "UVEditor2DViewportBehaviorTargets.h" // FUVEditor2DScrollBehaviorTarget, FUVEditor2DMouseWheelZoomBehaviorTarget
#include "Context/FDAssistorViewportButtonsAPI.h" // UFDAssistorViewportButtonsAPI::ESelectionMode

class UCanvas;
class UFDAssistorLive2DViewportAPI;

/**
 * Client used to display a 2D view of the UV's, implemented by using a perspective viewport with a locked
 * camera.
 */
class FDASSISTOR_API FFDAssistor2DViewportClient : public FEditorViewportClient/*, public IInputBehaviorSource*/
{
public:
	FFDAssistor2DViewportClient(FEditorModeTools* InModeTools, FPreviewScene* InPreviewScene,
		const TWeakPtr<SEditorViewport>& InEditorViewportWidget, UFDAssistorViewportButtonsAPI* ViewportButtonsAPI, UFDAssistorLive2DViewportAPI* live2DViewportAPI);

	virtual ~FFDAssistor2DViewportClient() {}

	bool AreSelectionButtonsEnabled() const;
	void SetSelectionMode(UFDAssistorViewportButtonsAPI::ESelectionMode NewMode);
	UFDAssistorViewportButtonsAPI::ESelectionMode GetSelectionMode() const;
	bool AreWidgetButtonsEnabled() const;

	void SetLocationGridSnapEnabled(bool bEnabled);
	bool GetLocationGridSnapEnabled();
	void SetLocationGridSnapValue(float SnapValue);
	float GetLocationGridSnapValue();
	void SetRotationGridSnapEnabled(bool bEnabled);
	bool GetRotationGridSnapEnabled();
	void SetRotationGridSnapValue(float SnapValue);
	float GetRotationGridSnapValue();
	void SetScaleGridSnapEnabled(bool bEnabled);
	bool GetScaleGridSnapEnabled();
	void SetScaleGridSnapValue(float SnapValue);
	float GetScaleGridSnapValue();

	// FEditorViewportClient
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;

	//virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual bool ShouldOrbitCamera() const override;
	bool CanSetWidgetMode(UE::Widget::EWidgetMode NewMode) const override;
	void SetWidgetMode(UE::Widget::EWidgetMode NewMode) override;
	UE::Widget::EWidgetMode GetWidgetMode() const override;	
	//void DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas) override;


	// Overriding base class visibility
	using FEditorViewportClient::OverrideNearClipPlane;

	// IInputBehaviorSource
	//virtual const UInputBehaviorSet* GetInputBehaviors() const override;

	// FGCObject
	//virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:
	//void DrawGrid(const FSceneView* View, FPrimitiveDrawInterface* PDI);
	//void DrawGridRulers(FViewport& InViewport, FSceneView& View, UCanvas& Canvas);
	//void DrawUDIMLabels(FViewport& InViewport, FSceneView& View, UCanvas& Canvas);

	// These get added in AddReferencedObjects for memory management
	UInputBehaviorSet* BehaviorSet;
	UFDAssistorViewportButtonsAPI* ViewportButtonsAPI;
	UFDAssistorLive2DViewportAPI* Live2DViewportAPI;

	bool bDrawGridRulers = true;
	bool bDrawGrid = true;
	UCanvas* CanvasObject;

	// Note that it's generally less hassle if the unique ptr types are complete here,
	// not forward declared, else we get compile errors if their destruction shows up
	// anywhere in the header.
	//TUniquePtr<FUVEditor2DScrollBehaviorTarget> ScrollBehaviorTarget;
	//TUniquePtr<FUVEditor2DMouseWheelZoomBehaviorTarget> ZoomBehaviorTarget;
};