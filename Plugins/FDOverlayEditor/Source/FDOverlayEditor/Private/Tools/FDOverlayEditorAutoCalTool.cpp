#include "Tools/FDOverlayEditorAutoCalTool.h"

#include "RHI.h"
#include "GlobalShader.h"
#include "ShaderParameters.h"
#include "ShaderCompilerCore.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "RenderGraphUtils.h"
#include "DynamicMesh/DynamicMesh3.h"

#include "FDOverlayMeshInput.h"

using namespace UE::Geometry;

///** The filter vertex declaration resource type. */
//class FDistortionVertexDeclaration : public FRenderResource
//{
//public:
//	FVertexDeclarationRHIRef VertexDeclarationRHI;
//
//	/** Destructor. */
//	virtual ~FDistortionVertexDeclaration() {}
//
//	virtual void InitRHI() override
//	{
//		uint16 Stride = sizeof(FDistortionVertex);
//		FVertexDeclarationElementList Elements;
//		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FDistortionVertex, Position), VET_Float2, 0, Stride));
//		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FDistortionVertex, TexR), VET_Float2, 1, Stride));
//		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FDistortionVertex, TexG), VET_Float2, 2, Stride));
//		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FDistortionVertex, TexB), VET_Float2, 3, Stride));
//		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FDistortionVertex, VignetteFactor), VET_Float1, 4, Stride));
//		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FDistortionVertex, TimewarpFactor), VET_Float1, 5, Stride));
//		VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
//	}
//
//	virtual void ReleaseRHI() override
//	{
//		VertexDeclarationRHI.SafeRelease();
//	}
//};
///** The Distortion vertex declaration. */
//TGlobalResource<FDistortionVertexDeclaration> GDistortionVertexDeclaration;


BEGIN_SHADER_PARAMETER_STRUCT(FFDOverlayParameters, )
SHADER_PARAMETER_RDG_BUFFER(StructuredBuffer<FAppliedVertex>, VerticesBuffer)
SHADER_PARAMETER_RDG_BUFFER(StructuredBuffer<FTriangle>, TrianglesBuffer)
SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputTex)
END_SHADER_PARAMETER_STRUCT()

class FFDOverlayCS : public FGlobalShader
{
public:

	DECLARE_GLOBAL_SHADER(FFDOverlayCS);
	SHADER_USE_PARAMETER_STRUCT(FFDOverlayCS, FGlobalShader);

	using FParameters = FFDOverlayParameters;

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment) {
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

	}


private:

};
IMPLEMENT_GLOBAL_SHADER(FFDOverlayCS, "/Plugin/FDOverlayEditor/Private/FDOverlay.usf", "CS", SF_Compute);


// This is a public interface that we define so outside code can invoke our compute shader.
class FDOVERLAYEDITOR_API FFDOverlayCSInterface {
public:
	// Executes this shader on the render thread
	static void Dispatch_RenderThread(
		FRHICommandListImmediate& RHICmdList,
		TArray<FAppliedVertex>& Vertices,
		TArray<FTriangle>& Triangles,
		ExtraParams& Params,
		TFunction<void(int OutputVal)> AsyncCallback
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void Dispatch_GameThread(
		TArray<FAppliedVertex>& Vertices,
		TArray<FTriangle>& Triangles,
		ExtraParams& Params,
		TFunction<void(int OutputVal)> AsyncCallback
	)
	{
		check(IsInGameThread());
		ENQUEUE_RENDER_COMMAND(CalOverlayCommand)(
			[&](FRHICommandListImmediate& RHICmdList)
			{
				Dispatch_RenderThread(RHICmdList, Vertices, Triangles, Params, AsyncCallback);
			});
	}

	// Dispatches this shader. Can be called from any thread
	static void Dispatch(
		const TObjectPtr<UFDOverlayMeshInput>& InTarget,
		ExtraParams& Params,
		TFunction<void(int OutputVal)> AsyncCallback
	)
	{
		int VerticesNum = InTarget->AppliedCanonical->VertexCount();
		int TrianglesNum = InTarget->AppliedCanonical->TriangleCount();

		TArray<FAppliedVertex> AppliedVertices;
		AppliedVertices.Reset(VerticesNum);
		for (int i = 0; i < VerticesNum; i++)
		{
			AppliedVertices.Emplace(FAppliedVertex(InTarget->AppliedCanonical->GetVertex(i), InTarget->AppliedCanonical->GetVertexNormal(i), InTarget->AppliedCanonical->GetVertexUV(i)));
		}

		TArray<FTriangle> Triangles;
		Triangles.Reset(TrianglesNum);
		for (int i = 0; i < TrianglesNum; i++)
		{
			FIndex3i trangle = InTarget->AppliedCanonical->GetTriangle(i)
			Triangles.Emplace(FTriangle(trangle.A, trangle.B, trangle.C));
		}

		
		if (IsInRenderingThread()) {
			Dispatch_RenderThread(GetImmediateCommandList_ForRenderCommand(), Vertices, Triangles, Params, AsyncCallback);
		}
		else {
			Dispatch_GameThread(Vertices, Triangles, Params, AsyncCallback);
		}
	}
};

void FFDOverlayCSInterface::Dispatch_RenderThread(FRHICommandListImmediate& RHICmdList, TArray<FAppliedVertex>& Vertices, TArray<FTriangle>& Triangles, ExtraParams& Params, TFunction<void(int OutputVal)> AsyncCallback)
{
	check(IsInRenderingThread());
	FRDGBuilder GraphBuilder(RHICmdList);

	FRDGBufferRef VerticesBuffer = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(sizeof(FAppliedVertex), Vertices.Num()), TEXT("FluidDynamicOverlay.VerticesBuffer"));
	FRDGBufferRef TrianglesBuffer = GraphBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(sizeof(FTriangle), Triangles.Num()), TEXT("FluidDynamicOverlay.TrianglesBuffer"));
	FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(Params.Size, EPixelFormat::PF_FloatRGBA, FClearValueBinding::Black, TexCreate_UAV);
	FRDGTextureRef OutputTex = GraphBuilder.CreateTexture(Desc, TEXT("FluidDynamicOverlayOutputPooledTexture"));
	{
		DECLARE_GPU_STAT(FDOverlayCS)
		RDG_GPU_STAT_SCOPE(GraphBuilder, FDOverlayCS);
		RDG_EVENT_SCOPE(GraphBuilder, "FluidDynamicOverlayComputeShader");
		TShaderMapRef<FFDOverlayCS>ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

		FFDOverlayCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FFDOverlayCS::FParameters>();
		PassParameters->VerticesBuffer = VerticesBuffer;
		PassParameters->TrianglesBuffer = TrianglesBuffer;
		PassParameters->OutputTex = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutputTex));

		FIntVector GroupCount = FComputeShaderUtils::GetGroupCount(Params.Size, FComputeShaderUtils::kGolden2DGroupSize);

		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("FluidDynamicOverlayComputeShader"),
			ERDGPassFlags::Compute | ERDGPassFlags::NeverCull,
			ComputeShader,
			PassParameters,
			GroupCount
		);
	}

	GraphBuilder.QueueTextureExtraction(OutputTex, &Params.ExtractedTexture);
	GraphBuilder.Execute();
}

bool UFDOverlayEditorAutoCalToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	return true;
}

UInteractiveTool* UFDOverlayEditorAutoCalToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	UFDOverlayEditorAutoCalTool* NewTool = NewObject<UFDOverlayEditorAutoCalTool>(SceneState.ToolManager);
	NewTool->SetTarget(*Target);

	return NewTool;
}

void UFDOverlayEditorAutoCalTool::SetTarget(const TObjectPtr<UFDOverlayMeshInput>& InTarget)
{
	Target = InTarget;

}


void UFDOverlayEditorAutoCalTool::Setup()
{
	UInteractiveTool::Setup();

}

void UFDOverlayEditorAutoCalTool::Shutdown(EToolShutdownType ShutdownType)
{
	
}

