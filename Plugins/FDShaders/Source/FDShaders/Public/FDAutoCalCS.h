#pragma once
#include "VectorTypes.h"
#include "GeometryBase.h"


PREDECLARE_GEOMETRY(class FDynamicMesh3);

struct FAppliedVertex
{
	FVector3f Position;
	FVector3f Normal;
	FVector2f UV;
};
struct FTriangle
{
	int32 A;
	int32 B;
	int32 C;
};
struct FParams
{
	FVector3f GradientOrigin;
	FVector3f GradientDir;
	float GradientMax;
	FVector3f UVCurveOrigin;
	float CurveRange;
	int32 KeyNum;
	int32 VertexNum;
	int32 TriangleNum;
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
	UTextureRenderTarget2D * RTOutput;
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
		TFunction<void(FExtraParams& ExtraParams)> CallBack,
		std::atomic<bool>& bDidGPUFinish
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void Dispatch_GameThread(
		TArray<FAppliedVertex>& Vertices,
		TArray<FTriangle>& Triangles,
		FExtraParams& ExtraParams,
		TFunction<void(FExtraParams& ExtraParams)> CallBack
	);

	// Dispatches this shader. Can be called from any thread
	static void Dispatch(
		const TSharedPtr<UE::Geometry::FDynamicMesh3> AppliedCanonical,
		FExtraParams& ExtraParams,
		TFunction<void(FExtraParams& ExtraParams)> CallBack
	);
};