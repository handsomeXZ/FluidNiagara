#pragma once
#include "VectorTypes.h"
#include "GeometryBase.h"


PREDECLARE_GEOMETRY(class FDynamicMesh3);

struct FAppliedVertex
{
	FVector3d Position;
	FVector3f Normal;
	FVector2f UV;
	FAppliedVertex(FVector3d PositionIn, FVector3f NormalIn, FVector2f UVIn) : Position(PositionIn), Normal(NormalIn), UV(UVIn) {}
};
struct FTriangle
{
	int A;
	int B;
	int C;
	FTriangle(int A, int B, int C) : A(A), B(B), C(C) {}
};
struct FParams
{
	FVector3f GradientOrigin;
	FVector3f GradientDir;
	float GradientMax;
	FVector3f UVCurveOrigin;
	float CurveRange;
};
struct FCurveKey
{
	float Time;
	float Value;
	float ArriveTangent;
	float LeaveTangent;
	FCurveKey(float TimeIn,
	float ValueIn,
	float ArriveTangentIn,
	float LeaveTangentIn) : Time(TimeIn), Value(ValueIn), ArriveTangent(ArriveTangentIn), LeaveTangent(LeaveTangentIn){}
};
struct FExtraParams
{
	FParams Params;
	TArray<FCurveKey> CurveKeys;
	FIntPoint Size;
	UTexture2D* OutputTexture;
	int MaterialID;
};


// This is a public interface that we define so outside code can invoke our compute shader.
class FDSHADERS_API FFDAutoCalCSInterface {
public:
	// Executes this shader on the render thread
	static void Dispatch_RenderThread(
		FRHICommandListImmediate& RHICmdList,
		TArray<FAppliedVertex> Vertices,
		TArray<FTriangle> Triangles,
		FExtraParams ExtraParams,
		TFunction<void(UTexture2D* OutputTexture)> CallBack
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void Dispatch_GameThread(
		TArray<FAppliedVertex>& Vertices,
		TArray<FTriangle>& Triangles,
		FExtraParams& ExtraParams,
		TFunction<void(UTexture2D* OutputTexture)> CallBack
	);

	// Dispatches this shader. Can be called from any thread
	static void Dispatch(
		const TSharedPtr<UE::Geometry::FDynamicMesh3> AppliedCanonical,
		FExtraParams& ExtraParams,
		TFunction<void(UTexture2D* OutputTexture)> CallBack
	);
};