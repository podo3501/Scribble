#include "Helper.h"
#include "./Renderer.h"
#include "./Directx3D.h"
#include "./headerUtility.h"

D3D12_GPU_DESCRIPTOR_HANDLE GetGpuSrvHandle(CRenderer* renderer, eTextureType type)
{
	ID3D12Device* device = renderer->GetDevice();
	UINT cbvSrvUavDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	UINT srvStartIndex = static_cast<UINT>(EtoV(type));
	ID3D12DescriptorHeap* srvDescHeap = renderer->GetSrvDescriptorHeap();
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandle{ srvDescHeap->GetGPUDescriptorHandleForHeapStart() };
	gpuDescHandle.Offset(srvStartIndex, cbvSrvUavDescSize);
	return gpuDescHandle;
}