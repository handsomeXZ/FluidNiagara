

#pragma once

#include "CoreMinimal.h"

#include "EditorViewportClient.h"

class UFDAssistorViewportButtonsAPI;

// Types of camera motion for the UV Editor 3D viewport
enum EFDAssistor3DViewportClientCameraMode {
	Orbit,
	Fly
};

/**
 * Viewport client for the 3d live preview in the UV editor. Currently same as editor viewport
 * client but doesn't allow editor gizmos/widgets, and alters orbit camera control.
 */
class FDASSISTOR_API FFDAssistor3DViewportClient : public FEditorViewportClient
{
public:

	FFDAssistor3DViewportClient(FEditorModeTools* InModeTools, FPreviewScene* InPreviewScene = nullptr,
		const TWeakPtr<SEditorViewport>& InEditorViewportWidget = nullptr, UFDAssistorViewportButtonsAPI* ViewportButtonsAPI = nullptr);

	virtual ~FFDAssistor3DViewportClient() {}

	// FEditorViewportClient
	virtual bool ShouldOrbitCamera() const override {

		// Including some additional checks to prevent the orbit mode from being on all the time,
		// which ultimately causes weirdness in how the camera transform matrices are handled by
		// the viewport internally.
		const bool bLeftMouseButtonDown = Viewport->KeyState(EKeys::LeftMouseButton) && !bLockFlightCamera;
		const bool bMiddleMouseButtonDown = Viewport->KeyState(EKeys::MiddleMouseButton);
		const bool bRightMouseButtonDown = Viewport->KeyState(EKeys::RightMouseButton);
		const bool bIsOnlyAltPressed = IsAltPressed() && !IsCtrlPressed() && !IsShiftPressed();

		switch (CameraMode) {
		case EFDAssistor3DViewportClientCameraMode::Orbit:
			return bIsOnlyAltPressed || bLeftMouseButtonDown || bMiddleMouseButtonDown || bRightMouseButtonDown;
		case EFDAssistor3DViewportClientCameraMode::Fly:
			return FEditorViewportClient::ShouldOrbitCamera();
		default:
			ensure(false);
			return FEditorViewportClient::ShouldOrbitCamera();
		}
	}
	bool CanSetWidgetMode(UE::Widget::EWidgetMode NewMode) const override {	return false; }
	void SetWidgetMode(UE::Widget::EWidgetMode NewMode) override {}
	UE::Widget::EWidgetMode GetWidgetMode() const override { return UE::Widget::EWidgetMode::WM_None; }
	void FocusCameraOnSelection();
public:

	void SetCameraMode(EFDAssistor3DViewportClientCameraMode CameraModeIn) { CameraMode = CameraModeIn; };
	EFDAssistor3DViewportClientCameraMode GetCameraMode() const { return CameraMode; };

protected:

	// Enforce Orbit camera for UV editor live preview viewport. Use this instead of the base class orbit camera flag
	// to allow for expected behaviors of the base class when in fly camera mode.
	EFDAssistor3DViewportClientCameraMode CameraMode = EFDAssistor3DViewportClientCameraMode::Orbit;
	UFDAssistorViewportButtonsAPI* ViewportButtonsAPI;

};