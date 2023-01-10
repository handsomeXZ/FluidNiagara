#include "RHI.h"
#include "GlobalShader.h"
#include "ShaderParameters.h"
#include "ShaderCompilerCore.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"

#include "DynamicMesh/DynamicMesh3.h"

using namespace UE::Geometry;
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
	float MaxCurveOffset;
	int MaterialID;
};
struct FCurveKey
{
	float Time;
	float Value;
	float ArriveTangent;
	float LeaveTangent;
};
struct FExtraParams
{
	FParams Params;
	TArray<FCurveKey> CurveKeys;
	FIntPoint Size;
	TRefCountPtr<IPooledRenderTarget> ExtractedTexture;
	UTexture2D* OutputTexture;
};

BEGIN_SHADER_PARAMETER_STRUCT(FCopyTextureParameters, )
// 声明CopySrc访问FRDGTexture*
RDG_TEXTURE_ACCESS(CopySrc, ERHIAccess::CopySrc)
// 直接取 UTexture2D的RHI
//RDG_TEXTURE_ACCESS(Output, ERHIAccess::CopyDest)
END_SHADER_PARAMETER_STRUCT()

BEGIN_SHADER_PARAMETER_STRUCT(FFDAutoCalParameters, )
SHADER_PARAMETER_RDG_BUFFER(StructuredBuffer<FAppliedVertex>, VerticesBuffer)
SHADER_PARAMETER_RDG_BUFFER(StructuredBuffer<FTriangle>, TrianglesBuffer)
SHADER_PARAMETER_RDG_BUFFER(StructuredBuffer<FCurveKey>, KeyBuffer)
SHADER_PARAMETER_RDG_BUFFER(StructuredBuffer<FParams>, ParamsBuffer)
SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, RDGRWTexture)
END_SHADER_PARAMETER_STRUCT()

class FFDAutoCalCS : public FGlobalShader
{
public:

	DECLARE_GLOBAL_SHADER(FFDAutoCalCS);
	SHADER_USE_PARAMETER_STRUCT(FFDAutoCalCS, FGlobalShader);

	using FParameters = FFDAutoCalParameters;

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment) {
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		/**
		 * Typed UAV loads are disallowed by default, as Windows 7 D3D 11.0 does not support them; this flag allows a shader to use them.
		 * so, you should add 'CFLAG_AllowTypedUAVLoads' into your shader
		 */
		OutEnvironment.CompilerFlags.Add(CFLAG_AllowTypedUAVLoads);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), FComputeShaderUtils::kGolden2DGroupSize);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), FComputeShaderUtils::kGolden2DGroupSize);
	}


private:

};
IMPLEMENT_GLOBAL_SHADER(FFDAutoCalCS, "/Plugin/FDShaders/Private/FDAutoCal.usf", "CS", SF_Compute);

// This is a public interface that we define so outside code can invoke our compute shader.
class FFDAutoCalCSInterface {
public:
	// Executes this shader on the render thread
	static void Dispatch_RenderThread(
		FRHICommandListImmediate& RHICmdList,
		TArray<FAppliedVertex>& Vertices,
		TArray<FTriangle>& Triangles,
		FExtraParams& ExtraParams,
		TFunction<void(UTexture2D* OutputTexture)> CallBack
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void Dispatch_GameThread(
		TArray<FAppliedVertex>& Vertices,
		TArray<FTriangle>& Triangles,
		FExtraParams& ExtraParams,
		TFunction<void(UTexture2D* OutputTexture)> CallBack
	)
	{
		check(IsInGameThread());
		ENQUEUE_RENDER_COMMAND(FDAutoCalCommand)(
			[&](FRHICommandListImmediate& RHICmdList)
			{
				Dispatch_RenderThread(RHICmdList, Vertices, Triangles, ExtraParams, CallBack);

			});
		// Block thread until GPU has finished
		std::atomic<bool> bDidGPUFinish(false);
		ENQUEUE_RENDER_COMMAND(ForwardGPU_FDAutoCalCommand)(
			[&bDidGPUFinish](FRHICommandListImmediate& RHICmdList)
			{
				bDidGPUFinish = true;
			}
		);
		while (!bDidGPUFinish)
		{
			FPlatformProcess::Sleep(0.1e-3);
		}
		CallBack(ExtraParams.OutputTexture);
	}



	// Dispatches this shader. Can be called from any thread
	static void Dispatch(
		const TSharedPtr<FDynamicMesh3> AppliedCanonical,
		FExtraParams& ExtraParams,
		TFunction<void(UTexture2D* OutputTexture)> CallBack
	)
	{
		int VerticesNum = AppliedCanonical->VertexCount();
		int TrianglesNum = AppliedCanonical->TriangleCount();

		TArray<FAppliedVertex> AppliedVertices;
		AppliedVertices.Reset(VerticesNum);
		for (int i = 0; i < VerticesNum; i++)
		{
			FAppliedVertex AppliedVertex(AppliedCanonical->GetVertex(i), AppliedCanonical->GetVertexNormal(i), AppliedCanonical->GetVertexUV(i));
			AppliedVertices.Emplace(AppliedVertex);
		}
		TArray<FTriangle> Triangles;
		Triangles.Reset(TrianglesNum);
		for (int i = 0; i < TrianglesNum; i++)
		{
			FIndex3i trangle = AppliedCanonical->GetTriangle(i);
			FTriangle Triangle(trangle.A, trangle.B, trangle.C);
			Triangles.Emplace(Triangle);
		}

		
		if (IsInRenderingThread()) {
			Dispatch_RenderThread(GetImmediateCommandList_ForRenderCommand(), AppliedVertices, Triangles, ExtraParams, CallBack);
		}
		else {
			Dispatch_GameThread(AppliedVertices, Triangles, ExtraParams, CallBack);
		}
	}
};

void FFDAutoCalCSInterface::Dispatch_RenderThread(FRHICommandListImmediate& RHICmdList, TArray<FAppliedVertex>& Vertices, TArray<FTriangle>& Triangles, FExtraParams& ExtraParams, TFunction<void(UTexture2D* OutputTexture)> CallBack)
{
	check(IsInRenderingThread());
	FRDGBuilder GraphBuilder(RHICmdList);

	FRDGBufferRef VerticesBuffer = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(sizeof(FAppliedVertex), Vertices.Num()), TEXT("FluidDynamicOverlay.VerticesBuffer"));
	FRDGBufferRef TrianglesBuffer = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(sizeof(FTriangle), Triangles.Num()), TEXT("FluidDynamicOverlay.TrianglesBuffer"));
	FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(ExtraParams.Size, EPixelFormat::PF_FloatRGBA, FClearValueBinding::Black, TexCreate_UAV);
	FRDGTextureRef RDGRWTexture = GraphBuilder.CreateTexture(Desc, TEXT("FluidDynamicOverlayOutputPooledTexture"));
	{
		DECLARE_GPU_STAT(FDAutoCalCS)
		RDG_GPU_STAT_SCOPE(GraphBuilder, FDAutoCalCS);
		RDG_EVENT_SCOPE(GraphBuilder, "FluidDynamicOverlayComputeShader");
		TShaderMapRef<FFDAutoCalCS>ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

		FFDAutoCalCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FFDAutoCalCS::FParameters>();
		PassParameters->VerticesBuffer = VerticesBuffer;
		PassParameters->TrianglesBuffer = TrianglesBuffer;
		PassParameters->RDGRWTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(RDGRWTexture));

		FIntVector GroupCount = FComputeShaderUtils::GetGroupCount(ExtraParams.Size, FComputeShaderUtils::kGolden2DGroupSize);

		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("FluidDynamicOverlayComputeShader"),
			ERDGPassFlags::Compute | ERDGPassFlags::NeverCull,
			ComputeShader,
			PassParameters,
			GroupCount
		);
	}
	{
		DECLARE_GPU_STAT(FDAutoCalCopy)
		RDG_GPU_STAT_SCOPE(GraphBuilder, FDAutoCalCopy);
		RDG_EVENT_SCOPE(GraphBuilder, "FluidDynamicOverlayCopyRDGToTexture2D");
		auto* PassParameters = GraphBuilder.AllocParameters<FCopyTextureParameters>();
		PassParameters->CopySrc = RDGRWTexture;
		// Select if the generated texture should be copy back to a CPU texture for being saved, or directly used
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("CopyRDGToTexture2D"),
			PassParameters,
			ERDGPassFlags::Readback,
			[RDGRWTexture, ExtraParams](FRHICommandList& RHICmdList)
			{
				if (ExtraParams.OutputTexture && ExtraParams.OutputTexture->GetResource() && ExtraParams.OutputTexture->GetResource()->GetTexture2DRHI())
				{
					FRHICopyTextureInfo CopyInfo;
					RHICmdList.CopyTexture(
						RDGRWTexture->GetRHI(),
						ExtraParams.OutputTexture->GetResource()->GetTexture2DRHI(),
						CopyInfo);
				}
			});
	}

	GraphBuilder.Execute();

	/*AsyncTask(ENamedThreads::ActualRenderingThread, []() {
		});*/

}

