#include "Renderer.h"
#include <DirectXColors.h>
#include "../Core/Directx3D.h"
#include "../Core/d3dx12.h"
#include "../Core/d3dUtil.h"
#include "../Core/Utility.h"
#include "../Core/GameTimer.h"
#include "../Core/UploadBuffer.h"
#include "./FrameResource.h"
#include "./RendererData.h"
#include "./Shader.h"

using Microsoft::WRL::ComPtr;

//7인 이유는 텍스춰를 7장을 다 올린다음 동적으로 선택하기 위함이다.  Texture2D gDiffuseMap[7] : register(t0)
constexpr UINT DescriptorHeapSize{ 7 };

CRenderer::CRenderer(CDirectx3D* directx3D)
	: m_directx3D(directx3D)
	, m_shader(std::make_unique<CShader>())
	, m_device(directx3D->GetDevice())
	, m_cmdList(directx3D->GetCommandList())
{}

bool CRenderer::Initialize()
{
	ReturnIfFalse(BuildRootSignature());
	ReturnIfFalse(BuildDescriptorHeaps());
	ReturnIfFalse(BuildPSOs());

	return true;
}

bool CRenderer::OnResize(int wndWidth, int wndHeight)
{
	ReturnIfFalse(m_directx3D->OnResize());
	// Update the viewport transform to cover the client area.
	m_screenViewport.TopLeftX = 0;
	m_screenViewport.TopLeftY = 0;
	m_screenViewport.Width = static_cast<float>(wndWidth);
	m_screenViewport.Height = static_cast<float>(wndHeight);
	m_screenViewport.MinDepth = 0.0f;
	m_screenViewport.MaxDepth = 1.0f;

	m_scissorRect = { 0, 0, wndWidth, wndHeight };

	return true;
}

bool CRenderer::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, DescriptorHeapSize, 0, 0);

	CD3DX12_ROOT_PARAMETER slotRootParameter[4];
	slotRootParameter[0].InitAsShaderResourceView(0, 1);
	slotRootParameter[1].InitAsShaderResourceView(1, 1);
	slotRootParameter[2].InitAsConstantBufferView(0);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = CoreUtil::GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(slotRootParameter), slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serialized = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serialized.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ReturnIfFailed(hr);

	ReturnIfFailed(m_device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)));

	return true;
}

bool CRenderer::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = DescriptorHeapSize;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ReturnIfFailed(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_srvDescHeap)));

	return true;
}

bool CRenderer::BuildPSOs()
{
	for (auto gPso : GraphicsPSO_ALL)
		ReturnIfFalse(MakePSOPipelineState(gPso));

	return true;
}

bool CRenderer::MakePSOPipelineState(GraphicsPSO psoType)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	ReturnIfFalse(MakeOpaqueDesc(&psoDesc));

	switch (psoType)
	{
	case GraphicsPSO::Opaque:						break;
	default: assert(!"wrong type");
	}
	
	ReturnIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, 
		IID_PPV_ARGS(&m_psoList[toUType(psoType)])));

	return true;
}

bool CRenderer::MakeOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{
	inoutDesc->NodeMask = 0;
	inoutDesc->SampleMask = UINT_MAX;
	inoutDesc->NumRenderTargets = 1;
	inoutDesc->pRootSignature = m_rootSignature.Get();
	inoutDesc->BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	inoutDesc->DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	inoutDesc->RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	inoutDesc->PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	ReturnIfFalse(m_shader->SetPipelineStateDesc(inoutDesc));
	m_directx3D->SetPipelineStateDesc(inoutDesc);

	return true;
}

bool CRenderer::Draw( CGameTimer* gt, CFrameResources* frameResources, const std::vector<std::unique_ptr<RenderItem>>& renderItem)
{
	auto cmdListAlloc = frameResources->GetCurrCmdListAlloc();
	ReturnIfFailed(cmdListAlloc->Reset());
	ReturnIfFailed(m_cmdList->Reset(cmdListAlloc, m_psoList[toUType(GraphicsPSO::Opaque)].Get()));

	m_cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_cmdList->RSSetViewports(1, &m_screenViewport);
	m_cmdList->RSSetScissorRects(1, &m_scissorRect);

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_directx3D->CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)));

	m_cmdList->ClearRenderTargetView(m_directx3D->CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
	m_cmdList->ClearDepthStencilView(m_directx3D->DepthStencilView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_cmdList->OMSetRenderTargets(1, &RvToLv(m_directx3D->CurrentBackBufferView()), true, &RvToLv(m_directx3D->DepthStencilView()));

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvDescHeap.Get() };
	m_cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	auto matBuf = frameResources->GetUploadBuffer(eBufferType::Material);
	m_cmdList->SetGraphicsRootShaderResourceView(1, matBuf->Resource()->GetGPUVirtualAddress());

	auto passCB = frameResources->GetUploadBuffer(eBufferType::PassCB);
	m_cmdList->SetGraphicsRootConstantBufferView(2, passCB->Resource()->GetGPUVirtualAddress());

	m_cmdList->SetGraphicsRootDescriptorTable(3, m_srvDescHeap->GetGPUDescriptorHandleForHeapStart());

	DrawRenderItems(frameResources->GetUploadBuffer(eBufferType::Instance), renderItem);

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_directx3D->CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));
	
	ReturnIfFalse(m_directx3D->ExcuteCommandLists());

	UINT64 curFenceIdx{ 0 };
	ReturnIfFalse(m_directx3D->ExcuteSwapChain(&curFenceIdx));
	frameResources->SetFence(curFenceIdx);

	return true;
}

void CRenderer::DrawRenderItems(UploadBuffer* instanceBuffer, const std::vector<std::unique_ptr<RenderItem>>& ritems)
{
	for (auto& ri : ritems)
	{
		m_cmdList->IASetVertexBuffers(0, 1, &RvToLv(ri->Geo->VertexBufferView()));
		m_cmdList->IASetIndexBuffer(&RvToLv(ri->Geo->IndexBufferView()));
		m_cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		m_cmdList->SetGraphicsRootShaderResourceView(0, instanceBuffer->Resource()->GetGPUVirtualAddress());

		m_cmdList->DrawIndexedInstanced(ri->IndexCount, ri->InstanceCount,
			ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}


