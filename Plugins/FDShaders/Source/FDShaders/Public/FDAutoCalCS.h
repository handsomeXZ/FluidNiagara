#pragma once
#include "VectorTypes.h"
#include "IndexTypes.h"

class UTexture2D;
class UTextureRenderTarget2DArray;

struct FUVVertex
{
	FVector3f Position;
	FVector3f Normal;
	FVector2f UV;
	FUVVertex(FVector3f PositionIn, FVector3f NormalIn, FVector2f UVIn)
		: Position(PositionIn), Normal(NormalIn), UV(UVIn){}
};
struct FTriangle
{
	int32 A;
	int32 B;
	int32 C;
	int32 MID;
	FTriangle(int32 a, int32 b, int32 c, int32 mid)
		: A(a), B(b), C(c), MID(mid) {}
	FTriangle(UE::Geometry::FIndex3i abc, int32 mid)
		: A(abc.A), B(abc.B), C(abc.C), MID(mid){}
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
struct FMultiParams
{
	int32 VertexNum;
	int32 TriangleNum;
};
struct FMultiParamsArr
{
	Vector3f GradientOrigin;
	FVector3f GradientDir;
	float GradientMax;
	FVector3f UVCurveOrigin;
	float CurveRange;
	int32 KeyNum;
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
	FIntPoint Size;
	UTextureRenderTarget2DArray* BakeBuffer;
	int32 MIDNum;
	int TargetID;
	TArray<FCurveKey> CurveKeys;
};
struct FMultiExtraParams
{
	FMultiParams Params;
	TArray<FMultiParamsArr> ParamsArr;
	FIntPoint Size;
	UTextureRenderTarget2DArray* BakeBuffer;
	int32 MIDNum;
	int TargetID;
	TArray<TArray<FCurveKey>> CurveKeys;
};


// This is a public interface that we define so outside code can invoke our compute shader.
class FDSHADERS_API FFDAutoCalCSInterface {
public:

	// Dispatches this shader. Can be called from any thread
	template<typename T>
	static void Dispatch(
		TArray<FUVVertex>& Vertices,
		TArray<FTriangle>& Triangles,
		T ExtraParams,
		TFunction<void(T ExtraParams)> CallBack
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	template<typename T>
	static void Dispatch_GameThread(
		TArray<FUVVertex>& Vertices,
		TArray<FTriangle>& Triangles,
		T ExtraParams,
		TFunction<void(T ExtraParams)> CallBack
	);


	// Executes this shader on the render thread
	static void Dispatch_RenderThread(
		FRHICommandListImmediate& RHICmdList,
		TArray<FUVVertex>& Vertices,
		TArray<FTriangle>& Triangles,
		FExtraParams ExtraParams,
		TFunction<void(FExtraParams ExtraParams)> CallBack,
		std::atomic<bool>& bDidGPUFinish
	);

	// Executes this shader on the render thread
	static void Dispatch_RenderThread(
		FRHICommandListImmediate& RHICmdList,
		TArray<FUVVertex>& Vertices,
		TArray<FTriangle>& Triangles,
		FMultiExtraParams ExtraParams,
		TFunction<void(FMultiExtraParams ExtraParams)> CallBack,
		std::atomic<bool>& bDidGPUFinish
	);

};