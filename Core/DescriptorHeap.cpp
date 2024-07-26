#include "pch.h"
#include "./DescriptorHeap.h"
#include "../Include/types.h"
#include "./CoreDefine.h"
#include "./d3dUtil.h"

CDescriptorHeap::~CDescriptorHeap() = default;
CDescriptorHeap::CDescriptorHeap()
	: m_device{ nullptr }
	, m_srvDescHeap{ nullptr }
	, m_dsvDescHeap{ nullptr }
	, m_rtvDescHeap{ nullptr }
{}

bool CDescriptorHeap::CreateDescriptorHeap(
	D3D12_DESCRIPTOR_HEAP_TYPE heapType,
	UINT numDescriptor, 
	D3D12_DESCRIPTOR_HEAP_FLAGS flags,
	ID3D12DescriptorHeap** descriptorHeap)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.Type = heapType;
	heapDesc.NumDescriptors = numDescriptor;
	heapDesc.Flags = flags;
	heapDesc.NodeMask = 0;
	ReturnIfFailed(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(descriptorHeap)));

	return true;
}

bool CDescriptorHeap::Build(ID3D12Device* device)
{
	m_device = device;
	m_cbvSrvUavDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	ReturnIfFalse(CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, TotalShaderResourceViewHeap,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, m_srvDescHeap.GetAddressOf()));
	ReturnIfFalse(CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, TotalDepthStencilView,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_dsvDescHeap.GetAddressOf()));
	ReturnIfFalse(CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, TotalRenderTargetViewHeap,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_rtvDescHeap.GetAddressOf()));

	return true;
}

void CDescriptorHeap::CreateShaderResourceView(SrvOffset offset, UINT index,
	const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D12Resource* pRes)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDesc{ m_srvDescHeap->GetCPUDescriptorHandleForHeapStart() };
	cpuDesc.Offset(EtoV(offset) + index, m_cbvSrvUavDescSize);

	m_device->CreateShaderResourceView(pRes, pDesc, cpuDesc);
}

void CDescriptorHeap::CreateDepthStencilView(DsvOffset offset,
	const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D12Resource* pRes)
{
	m_device->CreateDepthStencilView(pRes, pDesc, GetCpuDsvHandle(offset));
}

void CDescriptorHeap::CreateRenderTargetView(RtvOffset offset, 
	const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, ID3D12Resource* pRes)
{
	m_device->CreateRenderTargetView(pRes, pDesc, GetCpuRtvHandle(offset));
}

void CDescriptorHeap::SetSrvDescriptorHeaps(ID3D12GraphicsCommandList* cmdList)
{
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvDescHeap.Get()};
	cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
}

void CDescriptorHeap::SwapBackBuffer()
{
	m_currBackBuffer = (m_currBackBuffer + 1) % SwapChainBufferCount;
}

D3D12_GPU_DESCRIPTOR_HANDLE CDescriptorHeap::GetGpuSrvHandle(SrvOffset offset) const
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDesc{ m_srvDescHeap->GetGPUDescriptorHandleForHeapStart() };
	gpuDesc.Offset(EtoV(offset), m_cbvSrvUavDescSize);

	return gpuDesc;
}

D3D12_CPU_DESCRIPTOR_HANDLE CDescriptorHeap::GetCpuDsvHandle(DsvOffset offset) const
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDesc{ m_dsvDescHeap->GetCPUDescriptorHandleForHeapStart() };
	cpuDesc.Offset(EtoV(offset), m_dsvDescriptorSize);

	return cpuDesc;
}

D3D12_CPU_DESCRIPTOR_HANDLE CDescriptorHeap::GetCpuRtvHandle(RtvOffset offset) const
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDesc{ m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart() };
	cpuDesc.Offset(EtoV(offset), m_rtvDescriptorSize);

	return cpuDesc;
}

D3D12_CPU_DESCRIPTOR_HANDLE CDescriptorHeap::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart(), m_currBackBuffer, m_rtvDescriptorSize);
}