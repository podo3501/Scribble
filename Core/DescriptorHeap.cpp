#include "./DescriptorHeap.h"
#include <d3d12.h>
#include "../Include/types.h"
#include "./CoreDefine.h"
#include "./d3dUtil.h"
#include "./headerUtility.h"

CDescriptorHeap::~CDescriptorHeap() = default;
CDescriptorHeap::CDescriptorHeap(ID3D12Device* device)
	: m_device{ device }
	, m_srvDescHeap{ nullptr }
{
	m_cbvSrvUavDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

bool CDescriptorHeap::Build()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = TotalShaderResourceViewHeap;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ReturnIfFailed(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_srvDescHeap)));

	return true;
}

inline UINT StartOffset(eTextureType type)	{	return static_cast<UINT>(EtoV(type));		}

void CDescriptorHeap::CreateShaderResourceView(eTextureType type, UINT index,
	const D3D12_SHADER_RESOURCE_VIEW_DESC& pDesc, ID3D12Resource* pRes)
{
	UINT offset = StartOffset(type); 
	offset += index;

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDesc{ m_srvDescHeap->GetCPUDescriptorHandleForHeapStart() };
	hCpuDesc.Offset(offset, m_cbvSrvUavDescSize);
	m_device->CreateShaderResourceView(pRes, &pDesc, hCpuDesc);
}

D3D12_GPU_DESCRIPTOR_HANDLE CDescriptorHeap::GetGpuSrvHandle(eTextureType type)
{
	UINT cbvSrvUavDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	UINT offset = StartOffset(type);
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandle{ m_srvDescHeap->GetGPUDescriptorHandleForHeapStart() };
	gpuDescHandle.Offset(offset, cbvSrvUavDescSize);

	return gpuDescHandle;
}