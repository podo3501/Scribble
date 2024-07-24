#pragma once

#include <wrl.h>

struct ID3D12Device;
struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct D3D12_SHADER_RESOURCE_VIEW_DESC;
struct D3D12_GPU_DESCRIPTOR_HANDLE;
enum class eTextureType : int;

class CDescriptorHeap
{
public:
	CDescriptorHeap(ID3D12Device* device);
	~CDescriptorHeap();

	CDescriptorHeap() = delete;
	CDescriptorHeap(const CDescriptorHeap&) = delete;
	CDescriptorHeap& operator=(const CDescriptorHeap&) = delete;

	bool Build();
	void CreateShaderResourceView(eTextureType type, UINT index,
		const D3D12_SHADER_RESOURCE_VIEW_DESC& pDesc, ID3D12Resource* pRes);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuSrvHandle(eTextureType type);
	inline ID3D12DescriptorHeap* GetSrvDescriptorHeap();

private:
	ID3D12Device* m_device;
	UINT m_cbvSrvUavDescSize{ 0u };

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvDescHeap;
};

inline ID3D12DescriptorHeap* CDescriptorHeap::GetSrvDescriptorHeap() { return m_srvDescHeap.Get(); };
