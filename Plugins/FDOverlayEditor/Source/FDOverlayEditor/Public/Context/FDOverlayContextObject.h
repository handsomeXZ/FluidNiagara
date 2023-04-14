// Copyright HandsomeCheese. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "DynamicMesh/DynamicMeshAABBTree3.h"
#include "DynamicMesh/DynamicMeshChangeTracker.h" // FDynamicMeshChange for TUniquePtr
#include "InteractiveToolManager.h"
#include "GeometryBase.h"

#include "FDOverlayContextObject.generated.h"

// TODO: This should be spread out across multiple files

PREDECLARE_GEOMETRY(class FDynamicMesh3);
class FToolCommandChange;
struct FViewCameraState;
class UInputRouter;
class UWorld;
class UUVEditorToolMeshInput;

/**
 * Base class for context objects used in the FDOverlay.
 */
UCLASS()
class FDOVERLAYEDITOR_API UFDOverlayContextObject : public UObject
{
	GENERATED_BODY()
public:
	/**
	 * Called by the mode when shutting context objects down, allowing them to do any cleanup.
	 * Initialization, on the other hand is usually done by some class-specific Initialize() method.
	 */
	virtual void Shutdown() {}

	/**
	 * Called whenever a tool is ended, for instance to let a context object remove listeners associated
	 * with that tool (it shouldn't have to do so, but may choose to for robustness).
	 */
	virtual void OnToolEnded(UInteractiveTool* DeadTool) {}
};