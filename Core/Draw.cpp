#include "./Draw.h"
#include "./d3dUtil.h"
#include "../Include/FrameResourceData.h"
#include "../Include/RenderItem.h"
#include "../Include/Types.h"
#include "./FrameResources.h"
#include "./Directx3D.h"
#include "./RootSignature.h"
#include "./CoreDefine.h"
#include "./headerUtility.h"
#include "./ShadowMap.h"
#include "./PipelineStateObjects.h"
#include "./SsaoMap.h"
#include "./DescriptorHeap.h"

CDraw::~CDraw() = default;
CDraw::CDraw(CDirectx3D* directx3D)
	: m_directx3D{ directx3D }
	, m_device{ nullptr }
	, m_cmdList{ nullptr }
	, m_descHeap{ nullptr }
	, m_pso{ nullptr }
	, m_shadowMap{ nullptr }
	, m_screenViewport{}
	, m_scissorRect{}
{}

bool CDraw::Initialize(CDescriptorHeap* descHeap, CPipelineStateObjects* pso)
{
	m_descHeap = descHeap;
	m_device = m_directx3D->GetDevice();
	m_cmdList = m_directx3D->GetCommandList();
	m_shadowMap = std::make_unique<CShadowMap>(descHeap);
	m_pso = pso;

	ReturnIfFalse(m_shadowMap->Initialize(m_directx3D));

	return true;
}

void CDraw::OnResize(int width, int height)
{
	m_screenViewport.TopLeftX = 0;
	m_screenViewport.TopLeftY = 0;
	m_screenViewport.Width = static_cast<float>(width);
	m_screenViewport.Height = static_cast<float>(height);
	m_screenViewport.MinDepth = 0.0f;
	m_screenViewport.MaxDepth = 1.0f;

	m_scissorRect = { 0, 0, width, height };
}

D3D12_GPU_VIRTUAL_ADDRESS GetFrameResourceAddress(CFrameResources* frameRes, eBufferType bufType)
{
	return frameRes->GetResource(bufType)->GetGPUVirtualAddress();
}

bool CDraw::Excute(CRootSignature* rootSignature, CFrameResources* frameRes, CSsaoMap* ssaoMap, AllRenderItems& renderItem)
{
	auto cmdListAlloc = frameRes->GetCurrCmdListAlloc();
	ReturnIfFailed(cmdListAlloc->Reset());
	ReturnIfFailed(m_cmdList->Reset(cmdListAlloc, nullptr));

	m_cmdList->SetGraphicsRootSignature(rootSignature->Get(RootSignature::Common));

	m_descHeap->SetSrvDescriptorHeaps(m_cmdList);

	m_cmdList->SetGraphicsRootShaderResourceView(EtoV(MainRegisterType::Material), GetFrameResourceAddress(frameRes, eBufferType::Material));
	m_cmdList->SetGraphicsRootDescriptorTable(EtoV(MainRegisterType::Diffuse), m_descHeap->GetGpuSrvHandle(SrvOffset::Texture2D));

	DrawSceneToShadowMap(frameRes, renderItem);
	DrawNormalsAndDepth(frameRes, ssaoMap, renderItem);

	m_cmdList->SetGraphicsRootSignature(rootSignature->Get(RootSignature::Ssao));
	ssaoMap->ComputeSsao(m_cmdList, frameRes, 3);

	m_cmdList->SetGraphicsRootSignature(rootSignature->Get(RootSignature::Common));

	m_cmdList->RSSetViewports(1, &m_screenViewport);
	m_cmdList->RSSetScissorRects(1, &m_scissorRect);

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_directx3D->CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)));

	m_cmdList->ClearRenderTargetView(m_descHeap->CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
	m_cmdList->OMSetRenderTargets(1, &RvToLv(m_descHeap->CurrentBackBufferView()), true, &RvToLv(m_descHeap->GetCpuDsvHandle(DsvOffset::Common)));

	m_cmdList->SetGraphicsRootConstantBufferView(EtoV(MainRegisterType::Pass), GetFrameResourceAddress(frameRes, eBufferType::PassCB));
	m_cmdList->SetGraphicsRootShaderResourceView(EtoV(MainRegisterType::Material), GetFrameResourceAddress(frameRes, eBufferType::Material));
	m_cmdList->SetGraphicsRootDescriptorTable(EtoV(MainRegisterType::Shadow), m_descHeap->GetGpuSrvHandle(SrvOffset::ShadowMap));
	m_cmdList->SetGraphicsRootDescriptorTable(EtoV(MainRegisterType::Ssao), m_descHeap->GetGpuSrvHandle(SrvOffset::SsaoAmbientMap0));
	m_cmdList->SetGraphicsRootDescriptorTable(EtoV(MainRegisterType::Cube), m_descHeap->GetGpuSrvHandle(SrvOffset::TextureCube));
	m_cmdList->SetGraphicsRootDescriptorTable(EtoV(MainRegisterType::Diffuse), m_descHeap->GetGpuSrvHandle(SrvOffset::Texture2D));

	std::ranges::for_each(renderItem, [this, frameRes, &renderItem](auto& curRenderItem) {
		auto pso = curRenderItem.first;
		m_cmdList->SetPipelineState(m_pso->GetPso(pso));
		DrawRenderItems(frameRes, pso, renderItem[pso].get()); });

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_directx3D->CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));

	ReturnIfFalse(m_directx3D->ExcuteCommandLists());

	UINT64 curFenceIdx{ 0 };
	ReturnIfFalse(m_directx3D->ExcuteSwapChain(&curFenceIdx));
	frameRes->SetFence(curFenceIdx);

	return true;
}

void CDraw::DrawSceneToShadowMap(CFrameResources* frameRes, AllRenderItems& renderItem)
{
	m_cmdList->RSSetViewports(1, &RvToLv(m_shadowMap->Viewport()));
	m_cmdList->RSSetScissorRects(1, &RvToLv(m_shadowMap->ScissorRect()));

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_shadowMap->Resource(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE)));
	D3D12_CPU_DESCRIPTOR_HANDLE dsvShadowMap = m_descHeap->GetCpuDsvHandle(DsvOffset::ShadowMap);
	m_cmdList->ClearDepthStencilView(dsvShadowMap, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_cmdList->OMSetRenderTargets(0, nullptr, false, &dsvShadowMap);

	UINT passCBByteSize = frameRes->GetBufferSize(eBufferType::PassCB);
	//2개의 cb가 들어가 있는데 2번째를 가져올 함수가 아직 없다. -> + 1 * passCBByteSize;
	D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = GetFrameResourceAddress(frameRes, eBufferType::PassCB) + 1 * passCBByteSize;
	m_cmdList->SetGraphicsRootConstantBufferView(EtoV(MainRegisterType::Pass), passCBAddress);

	m_cmdList->SetPipelineState(m_pso->GetPso(GraphicsPSO::ShadowMap));
	DrawRenderItems(frameRes, GraphicsPSO::NormalOpaque, renderItem[GraphicsPSO::NormalOpaque].get());

	m_cmdList->SetPipelineState(m_pso->GetPso(GraphicsPSO::SkinnedShadowOpaque));
	DrawRenderItems(frameRes, GraphicsPSO::SkinnedOpaque, renderItem[GraphicsPSO::SkinnedOpaque].get());

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_shadowMap->Resource(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ)));
}

void CDraw::DrawNormalsAndDepth(CFrameResources* frameRes, CSsaoMap* ssaoMap, AllRenderItems& renderItem)
{
	m_cmdList->RSSetViewports(1, &m_screenViewport);
	m_cmdList->RSSetScissorRects(1, &m_scissorRect);

	auto normalMap = ssaoMap->NormalMap();
	auto normalMapRtv = m_descHeap->GetCpuRtvHandle(RtvOffset::NormalMap);

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(normalMap,
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET)));

	float clearValue[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	m_cmdList->ClearRenderTargetView(normalMapRtv, clearValue, 0, nullptr);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCommon = m_descHeap->GetCpuDsvHandle(DsvOffset::Common);
	m_cmdList->ClearDepthStencilView(dsvCommon, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_cmdList->OMSetRenderTargets(1, &normalMapRtv, true, &dsvCommon);
	m_cmdList->SetGraphicsRootConstantBufferView(EtoV(MainRegisterType::Pass), GetFrameResourceAddress(frameRes, eBufferType::PassCB));

	m_cmdList->SetPipelineState(m_pso->GetPso(GraphicsPSO::SsaoDrawNormals));
	DrawRenderItems(frameRes, GraphicsPSO::NormalOpaque, renderItem[GraphicsPSO::NormalOpaque].get());

	m_cmdList->SetPipelineState(m_pso->GetPso(GraphicsPSO::SkinnedDrawNormals));
	DrawRenderItems(frameRes, GraphicsPSO::SkinnedOpaque, renderItem[GraphicsPSO::SkinnedOpaque].get());

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(normalMap,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ)));
}

void CDraw::DrawRenderItems(CFrameResources* frameRes, GraphicsPSO pso, RenderItem* renderItem)
{
	ID3D12Resource* instanceRes = frameRes->GetResource(eBufferType::Instance);

	m_cmdList->IASetVertexBuffers(0, 1, &renderItem->vertexBufferView);
	m_cmdList->IASetIndexBuffer(&renderItem->indexBufferView);
	m_cmdList->IASetPrimitiveTopology(renderItem->primitiveType);

	auto skinnedCB = frameRes->GetResource(eBufferType::SkinnedCB);
	UINT skinnedCBByteSize = frameRes->GetBufferSize(eBufferType::SkinnedCB);

	for (auto& ri : renderItem->subRenderItems)
	{
		auto& subRenderItem = ri.second;
		auto& subItem = subRenderItem.subItem;

		m_cmdList->SetGraphicsRootShaderResourceView(EtoV(MainRegisterType::Instance),
			instanceRes->GetGPUVirtualAddress() +
			(renderItem->startIndexInstance + subRenderItem.startSubIndexInstance) * sizeof(InstanceBuffer));

		if (pso == GraphicsPSO::SkinnedOpaque)
		{
			D3D12_GPU_VIRTUAL_ADDRESS skinnedCBAddress = skinnedCB->GetGPUVirtualAddress();// +1 * skinnedCBByteSize;
			m_cmdList->SetGraphicsRootConstantBufferView(EtoV(MainRegisterType::Bone), skinnedCBAddress);
		}
		else
			m_cmdList->SetGraphicsRootConstantBufferView(EtoV(MainRegisterType::Bone), 0);

		m_cmdList->DrawIndexedInstanced(subItem.indexCount, subRenderItem.instanceCount,
			subItem.startIndexLocation, subItem.baseVertexLocation, 0);
	}
}