#include "FDAutoCalCS.h"

#include "RHI.h"
#include "GlobalShader.h"
#include "ShaderParameters.h"
#include "ShaderCompilerCore.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "Kismet\KismetMathLibrary.h"

#include "DynamicMesh/DynamicMeshAttributeSet.h" //FDynamicMeshUVOverlay
#include "DynamicMesh/DynamicMesh3.h"

#define LOCTEXT_NAMESPACE "FluidDynamicShader"

using namespace UE::Geometry;

BEGIN_SHADER_PARAMETER_STRUCT(FCopyTextureParameters, )
// 声明CopySrc访问FRDGTexture*
RDG_TEXTURE_ACCESS(CopySrc, ERHIAccess::CopySrc)
// 直接取 UTexture2D的RHI
//RDG_TEXTURE_ACCESS(Output, ERHIAccess::CopyDest)
END_SHADER_PARAMETER_STRUCT()

BEGIN_SHADER_PARAMETER_STRUCT(FFDAutoCalParameters, )
SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FAppliedVertex>, VerticesBuffer)
SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FTriangle>, TrianglesBuffer)
SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FCurveKey>, KeyBuffer)
SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FParams>, ParamsBuffer)
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


void FFDAutoCalCSInterface::Dispatch_GameThread(
	TArray<FAppliedVertex>& Vertices,
	TArray<FTriangle>& Triangles,
	FExtraParams& ExtraParams,
	TFunction<void(UTexture2D* OutputTexture)> CallBack
)
{
	check(IsInGameThread());
	ENQUEUE_RENDER_COMMAND(FDAutoCalCommand)(
		[Vertices, Triangles, ExtraParams, CallBack](FRHICommandListImmediate& RHICmdList)
		{
			Dispatch_RenderThread(RHICmdList, Vertices, Triangles, ExtraParams, CallBack);

		});
	//// Block thread until GPU has finished
	//std::atomic<bool> bDidGPUFinish(false);
	//ENQUEUE_RENDER_COMMAND(ForwardGPU_FDAutoCalCommand)(
	//	[&bDidGPUFinish](FRHICommandListImmediate& RHICmdList)
	//	{
	//		bDidGPUFinish = true;
	//	}
	//);
	//while (!bDidGPUFinish)
	//{
	//	FPlatformProcess::Sleep(0.1e-3);
	//}
	//CallBack(ExtraParams.OutputTexture);
}


void FFDAutoCalCSInterface::Dispatch(
	const TSharedPtr<FDynamicMesh3> AppliedCanonical,
	FExtraParams& ExtraParams,
	TFunction<void(UTexture2D* OutputTexture)> CallBack
)
{
	int VerticesNum = AppliedCanonical->VertexCount();
	int TrianglesNum = AppliedCanonical->TriangleCount();

	TArray<FAppliedVertex> AppliedVertices;
	AppliedVertices.Reset(VerticesNum);
	double GradientMax = 0;
	for (int i = 0; i < VerticesNum; i++)
	{
		FAppliedVertex AppliedVertex(AppliedCanonical->GetVertex(i), AppliedCanonical->GetVertexNormal(i), AppliedCanonical->GetVertexUV(i));
		AppliedVertices.Emplace(AppliedVertex);

		// 计算 GradientMax
		float d = UKismetMathLibrary::GetPointDistanceToLine(AppliedCanonical->GetVertex(i), FVector(ExtraParams.Params.GradientOrigin), FVector(ExtraParams.Params.GradientDir));
		if (GradientMax < d)
		{
			GradientMax = d;
		}
	}
	ExtraParams.Params.GradientMax = GradientMax;
	TArray<FTriangle> Triangles;
	Triangles.Reset(TrianglesNum);
	for (int i = 0; i < TrianglesNum; i++)
	{
		if (AppliedCanonical->Attributes()->GetMaterialID()->GetValue(i) == ExtraParams.MaterialID)
		{
			FIndex3i trangle = AppliedCanonical->GetTriangle(i);
			FTriangle Triangle(trangle.A, trangle.B, trangle.C);
			Triangles.Emplace(Triangle);
		}
	}


	if (IsInRenderingThread()) {
		Dispatch_RenderThread(GetImmediateCommandList_ForRenderCommand(), AppliedVertices, Triangles, ExtraParams, CallBack);
	}
	else {
		Dispatch_GameThread(AppliedVertices, Triangles, ExtraParams, CallBack);
	}
}



void FFDAutoCalCSInterface::Dispatch_RenderThread(FRHICommandListImmediate& RHICmdList, TArray<FAppliedVertex> Vertices, TArray<FTriangle> Triangles, FExtraParams ExtraParams, TFunction<void(UTexture2D* OutputTexture)> CallBack)
{
	check(IsInRenderingThread());
	FRDGBuilder GraphBuilder(RHICmdList);

	FRDGBufferRef VerticesBuffer = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(sizeof(FAppliedVertex), Vertices.Num()), TEXT("FluidDynamicOverlay.VerticesBuffer"));
	FRDGBufferRef TrianglesBuffer = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(sizeof(FTriangle), Triangles.Num()), TEXT("FluidDynamicOverlay.TrianglesBuffer"));
	FRDGBufferRef KeyBuffer = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(sizeof(FCurveKey), ExtraParams.CurveKeys.Num()), TEXT("FluidDynamicOverlay.VerticesBuffer"));
	FRDGBufferRef ParamsBuffer = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(sizeof(FParams), 1), TEXT("FluidDynamicOverlay.TrianglesBuffer"));

	GraphBuilder.QueueBufferUpload(VerticesBuffer, Vertices.GetData(), sizeof(FAppliedVertex) * Vertices.Num(), ERDGInitialDataFlags::None);
	GraphBuilder.QueueBufferUpload(TrianglesBuffer, Triangles.GetData(), sizeof(FTriangle) * Triangles.Num(), ERDGInitialDataFlags::None);
	GraphBuilder.QueueBufferUpload(KeyBuffer, ExtraParams.CurveKeys.GetData(), sizeof(FCurveKey) * ExtraParams.CurveKeys.Num(), ERDGInitialDataFlags::None);
	GraphBuilder.QueueBufferUpload(ParamsBuffer, &(ExtraParams.Params), sizeof(FParams), ERDGInitialDataFlags::None);

	FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(ExtraParams.Size, EPixelFormat::PF_FloatRGBA, FClearValueBinding::Black, TexCreate_UAV);
	FRDGTextureRef RDGRWTexture = GraphBuilder.CreateTexture(Desc, TEXT("FluidDynamicOverlayOutputPooledTexture"));
	{
		DECLARE_GPU_STAT(FDAutoCalCS)
		RDG_GPU_STAT_SCOPE(GraphBuilder, FDAutoCalCS);
		RDG_EVENT_SCOPE(GraphBuilder, "FluidDynamicOverlayComputeShader");
		TShaderMapRef<FFDAutoCalCS>ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

		FFDAutoCalCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FFDAutoCalCS::FParameters>();
		PassParameters->VerticesBuffer = GraphBuilder.CreateSRV(VerticesBuffer);
		PassParameters->TrianglesBuffer = GraphBuilder.CreateSRV(TrianglesBuffer);
		PassParameters->KeyBuffer = GraphBuilder.CreateSRV(KeyBuffer);
		PassParameters->ParamsBuffer = GraphBuilder.CreateSRV(ParamsBuffer);
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
	// Select if the generated texture should be copy back to a CPU texture for being saved, or directly used
	// GroomTextureBuilder.cpp - Row 215
	// RenderGraphUtils.cpp    - Row 842
	{
		DECLARE_GPU_STAT(FDAutoCalCopy)
		RDG_GPU_STAT_SCOPE(GraphBuilder, FDAutoCalCopy);
		RDG_EVENT_SCOPE(GraphBuilder, "FluidDynamicOverlayCopyRDGToTexture2D");
		//auto PassParameters = GraphBuilder.AllocParameters<FCopyTextureParameters>();
		//PassParameters->CopySrc = RDGRWTexture;
		//
		//GraphBuilder.AddPass(
		//	RDG_EVENT_NAME("CopyRDGToTexture2D"),
		//	PassParameters,
		//	ERDGPassFlags::Readback,
		//	[RDGRWTexture, ExtraParams](FRHICommandList& RHICmdList)
		//	{
		//		if (ExtraParams.OutputTexture && ExtraParams.OutputTexture->GetResource() && ExtraParams.OutputTexture->GetResource()->GetTexture2DRHI())
		//		{
		//			FRHICopyTextureInfo CopyInfo;
		//			RHICmdList.CopyTexture(
		//				RDGRWTexture->GetRHI(),
		//				ExtraParams.OutputTexture->GetResource()->GetTexture2DRHI(),
		//				CopyInfo);
		//		}
		//	});
		AddReadbackTexturePass(
			GraphBuilder,
			RDG_EVENT_NAME("CopyRDGToTexture2D"),
			RDGRWTexture,
			[RDGRWTexture, ExtraParams](FRHICommandListImmediate& RHICmdList)
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




#undef LOCTEXT_NAMESPACE