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
#include "Engine\TextureRenderTarget2DArray.h"
#include "TextureRenderTarget2DArrayResource.h"
//#include "HAL\ThreadSafeBool.h"		DEPRECATED. use `std::atomic<bool>`

#include "DynamicMesh/DynamicMeshAttributeSet.h" //FDynamicMeshUVOverlay
#include "DynamicMesh/DynamicMesh3.h"

#define LOCTEXT_NAMESPACE "FluidDynamicShader"

using namespace UE::Geometry;

BEGIN_SHADER_PARAMETER_STRUCT(FCopyTextureParameters, )
// 声明CopySrc访问FRDGTexture*
RDG_TEXTURE_ACCESS(CopySrc, ERHIAccess::CopySrc)
// 直接取 UTexture2D的RHI
RDG_TEXTURE_ACCESS(CopyDest, ERHIAccess::CopyDest)
END_SHADER_PARAMETER_STRUCT()

BEGIN_SHADER_PARAMETER_STRUCT(FFDAutoCalParameters, )
SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FAppliedVertex>, VerticesBuffer)
SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FTriangle>, TrianglesBuffer)
SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FCurveKey>, KeyBuffer)
SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FParams>, ParamsBuffer)
SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2DArray<float4>, RDGRWTexture)
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
	FExtraParams ExtraParams,
	TFunction<void(FExtraParams ExtraParams)> CallBack
)
{
	check(IsInGameThread());

	std::atomic<bool> bDidGPUFinish(false);

	ENQUEUE_RENDER_COMMAND(FDAutoCalCommand)(
		[Vertices, Triangles, ExtraParams, CallBack, &bDidGPUFinish](FRHICommandListImmediate& RHICmdList)
		{
			Dispatch_RenderThread(RHICmdList, Vertices, Triangles, ExtraParams, CallBack, bDidGPUFinish);

		});
	// Block thread until GPU has finished
	//std::atomic<bool> bDidGPUFinish(false);
	//ENQUEUE_RENDER_COMMAND(ForwardGPU_FDAutoCalCommand)(
	//	[&bDidGPUFinish](FRHICommandListImmediate& RHICmdList)
	//	{
	//		bDidGPUFinish = true;
	//	}
	//);
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 可以借助 FDelegateGraphTask、FGraphEventRef 或者 ENQUEUE_RENDER_COMMAND 来实现渲染回调，但它们似乎不能保证在游戏线程执行
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	while (!bDidGPUFinish)
	{
		FPlatformProcess::Sleep(0.1e-3);
	}
	CallBack(ExtraParams);
}


void FFDAutoCalCSInterface::Dispatch(
	TArray<FAppliedVertex> Vertices,
	TArray<FTriangle> Triangles,
	FExtraParams ExtraParams,
	TFunction<void(FExtraParams ExtraParams)> CallBack
)
{

	Dispatch_GameThread(Vertices, Triangles, ExtraParams, CallBack);
	
}



void FFDAutoCalCSInterface::Dispatch_RenderThread(FRHICommandListImmediate& RHICmdList, TArray<FAppliedVertex> Vertices, TArray<FTriangle> Triangles, FExtraParams ExtraParams, TFunction<void(FExtraParams ExtraParams)> CallBack, std::atomic<bool>& bDidGPUFinish)
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

	FRDGTextureDesc Desc = FRDGTextureDesc::Create2DArray(ExtraParams.Size, EPixelFormat::PF_FloatRGBA, FClearValueBinding::Black, 
		TexCreate_ShaderResource | TexCreate_RenderTargetable | TexCreate_UAV, ExtraParams.MIDNum);
	FRDGTextureRef RDGRWTexture = GraphBuilder.CreateTexture(Desc, TEXT("FluidDynamicOverlayOutputPooledTexture"));

	//UTextureRenderTarget2D* RenderTarget;
	//FRenderTarget* RenderTargetResource = RenderTarget->GetRenderTargetResource();
	//FTexture2DRHIRef RenderTargetRHI = RenderTargetResource->GetRenderTargetTexture();
	//FSceneRenderTargetItem RenderTargetItem;
	//RenderTargetItem.TargetableTexture = RenderTargetRHI;
	//RenderTargetItem.ShaderResourceTexture = RenderTargetRHI;
	//FPooledRenderTargetDesc RenderTargetDesc = FPooledRenderTargetDesc::Create2DDesc(FIntPoint(InputTexture->GetSizeX(), InputTexture->GetSizeY()), InputTexture->GetPixelFormat(), FClearValueBinding::None, TexCreate_None, TexCreate_ShaderResource | TexCreate_UAV, false);
	//TRefCountPtr<IPooledRenderTarget> PooledRenderTarget;
	//GRenderTargetPool.CreateUntrackedElement(RenderTargetDesc, PooledRenderTarget, RenderTargetItem);

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
			ERDGPassFlags::AsyncCompute,
			ComputeShader,
			PassParameters,
			GroupCount
		);
	}
	// Select if the generated texture should be copy back to a CPU texture for being saved, or directly used
	// GroomTextureBuilder.cpp - Row 215
	// RenderGraphUtils.cpp    - Row 842

	{
	
		AddReadbackTexturePass(
			GraphBuilder,
			RDG_EVENT_NAME("CopyRDGToTexture2D"),
			RDGRWTexture,
			[RDGRWTexture, ExtraParams, &bDidGPUFinish](FRHICommandListImmediate& RHICmdList)
			{
				if (RDGRWTexture->GetRHI()/* && ExtraParams.OutputTexture->Source.IsValid() && ExtraParams.OutputTexture && ExtraParams.OutputTexture->GetResource() && ExtraParams.OutputTexture->GetResource()->GetTexture2DRHI()*/)
				{
					FTextureRenderTarget2DArrayResource* TextureResource = (FTextureRenderTarget2DArrayResource*)ExtraParams.BakeBuffer->GetRenderTargetResource();

					FRHICopyTextureInfo CopyInfo;
					CopyInfo.NumMips = 1;
					CopyInfo.NumSlices = ExtraParams.MIDNum;
					RHICmdList.CopyTexture(
						RDGRWTexture->GetRHI(),
						/*ExtraParams.OutputTexture->GetResource()->GetTexture2DRHI(),*/
						TextureResource->GetRenderTargetTexture(),
						CopyInfo);
	
					//TArray<FFloat16Color> ColorData;
					//FReadSurfaceDataFlags flag;
					//RHICmdList.ReadSurfaceFloatData(RDGRWTexture->GetRHI(), FIntRect(0, 0, ExtraParams.Size.X, ExtraParams.Size.Y), ColorData, flag);
					//ExtraParams.OutputTexture->Source.Init(ExtraParams.Size.X, ExtraParams.Size.Y, 1, 1, ETextureSourceFormat::TSF_RGBA16F, (uint8*)ColorData.GetData());
					//ExtraParams.OutputTexture->DeferCompression = true;
					bDidGPUFinish.store(true);
				}
			});
	}
	GraphBuilder.Execute();

	/*AsyncTask(ENamedThreads::ActualRenderingThread, []() {
		});*/

}




#undef LOCTEXT_NAMESPACE