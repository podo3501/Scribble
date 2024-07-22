#include "PipelineStateObjects.h"
#include <ranges>
#include <algorithm>
#include "../Include/Types.h"
#include "./Shader.h"
#include "./d3dUtil.h"
#include "./Directx3D.h"
#include "./Renderer.h"
#include "./SsaoMap.h"

CPipelineStateObjects::~CPipelineStateObjects() = default;
CPipelineStateObjects::CPipelineStateObjects(CRenderer* renderer)
	: m_renderer{ renderer }
	, m_psoList{}
{}

ID3D12PipelineState* CPipelineStateObjects::GetPso(GraphicsPSO type) noexcept
{
	auto find = m_psoList.find(type);
	if (find == m_psoList.end())
		return nullptr;

	return find->second.Get();
}

bool CPipelineStateObjects::Build(CShader* shader)
{
	return (std::ranges::all_of(shader->GetPSOList(), [this, shader](auto pso) {
		return CreatePipelineState(shader, pso);
		}));
}

bool CPipelineStateObjects::CreatePipelineState(CShader* shader, GraphicsPSO psoType)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	ReturnIfFalse(shader->SetPipelineStateDesc(psoType, &psoDesc));

	MakePSOPipelineState(psoType, &psoDesc);

	ID3D12Device* device = m_renderer->GetDevice();
	ReturnIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_psoList[psoType])));

	return true;
}

void CPipelineStateObjects::MakePSOPipelineState(GraphicsPSO psoType, D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept
{
	MakeBasicDesc(psoDesc);

	switch (psoType)
	{
	case GraphicsPSO::Sky:										MakeSkyDesc(psoDesc);										break;
	case GraphicsPSO::Opaque:								MakeOpaqueDesc(psoDesc);									break;
	case GraphicsPSO::NormalOpaque:					MakeNormalOpaqueDesc(psoDesc);					break;
	case GraphicsPSO::SkinnedOpaque:					MakeSkinnedOpaqueDesc(psoDesc);					break;
	case GraphicsPSO::SkinnedShadowOpaque:	MakeSkinnedShadowOpaqueDesc(psoDesc);		break;
	case GraphicsPSO::SkinnedDrawNormals:		MakeSkinnedDrawNormals(psoDesc);					break;
	case GraphicsPSO::ShadowMap:						MakeShadowDesc(psoDesc);									break;
	case GraphicsPSO::SsaoDrawNormals:				MakeDrawNormals(psoDesc);								break;
	case GraphicsPSO::SsaoMap:							MakeSsaoDesc(psoDesc);										break;
	case GraphicsPSO::SsaoBlur:								MakeSsaoBlurDesc(psoDesc);								break;
	case GraphicsPSO::Debug:									MakeDebugDesc(psoDesc);									break;
	default: assert(!"wrong type");
	}
}

void CPipelineStateObjects::MakeBasicDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept
{
	psoDesc->NodeMask = 0;
	psoDesc->SampleMask = UINT_MAX;
	psoDesc->NumRenderTargets = 1;
	psoDesc->pRootSignature = m_renderer->GetRootSignature(RootSignature::Common);
	psoDesc->BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc->DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc->RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc->PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	m_renderer->GetDirectx3D()->SetPipelineStateDesc(psoDesc);
}

void CPipelineStateObjects::MakeSkyDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept
{
	psoDesc->RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc->DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void CPipelineStateObjects::MakeOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept
{}

void CPipelineStateObjects::MakeNormalOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept
{
	psoDesc->DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;
	psoDesc->DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
}

void CPipelineStateObjects::MakeSkinnedOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept
{
	MakeNormalOpaqueDesc(psoDesc);
}

void CPipelineStateObjects::MakeSkinnedShadowOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept
{
	MakeShadowDesc(psoDesc);
}

void CPipelineStateObjects::MakeSkinnedDrawNormals(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept
{
	MakeDrawNormals(psoDesc);
}

void CPipelineStateObjects::MakeShadowDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept
{
	psoDesc->RasterizerState.DepthBias = 10000;
	psoDesc->RasterizerState.DepthBiasClamp = 0.0f;
	psoDesc->RasterizerState.SlopeScaledDepthBias = 1.0f;
	psoDesc->RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	psoDesc->NumRenderTargets = 0;
}

void CPipelineStateObjects::MakeDrawNormals(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept
{
	psoDesc->RTVFormats[0] = CSsaoMap::NormalMapFormat;
	psoDesc->SampleDesc.Count = 1;
	psoDesc->SampleDesc.Quality = 0;
	psoDesc->DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
}

void CPipelineStateObjects::MakeSsaoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept
{
	psoDesc->InputLayout = { nullptr, 0 };
	psoDesc->pRootSignature = m_renderer->GetRootSignature(RootSignature::Ssao);
	psoDesc->DepthStencilState.DepthEnable = false;
	psoDesc->DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc->RTVFormats[0] = CSsaoMap::AmbientMapFormat;
	psoDesc->SampleDesc.Count = 1;
	psoDesc->SampleDesc.Quality = 0;
	psoDesc->DSVFormat = DXGI_FORMAT_UNKNOWN;
}

void CPipelineStateObjects::MakeSsaoBlurDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept
{
	MakeSsaoDesc(psoDesc);
}

void CPipelineStateObjects::MakeDebugDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept
{}
