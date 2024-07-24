#pragma once

#include <d3d12.h>
#include <wrl.h>

struct ID3D12Device;
struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct ID3D12GraphicsCommandList;
enum class SrvOffset : int;
enum class DsvOffset : int;
enum class RtvOffset : int;

class CDescriptorHeap
{
public:
	CDescriptorHeap();
	~CDescriptorHeap();

	CDescriptorHeap(const CDescriptorHeap&) = delete;
	CDescriptorHeap& operator=(const CDescriptorHeap&) = delete;

	bool Build(ID3D12Device* device);
	void CreateShaderResourceView(SrvOffset offset, UINT index,
		const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D12Resource* pRes);
	void CreateDepthStencilView(DsvOffset offset, 
		const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D12Resource* pRes);
	void CreateRenderTargetView(RtvOffset offset,
		const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, ID3D12Resource* pRes);

	void SetSrvDescriptorHeaps(ID3D12GraphicsCommandList* cmdList);

	void SwapBackBuffer();
	inline void SetupFirstBackBuffer();
	inline int GetCurrBackBuffer() const;

	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuSrvHandle(SrvOffset offset) const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDsvHandle(DsvOffset offset) const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuRtvHandle(RtvOffset offset) const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;

private:
	bool CreateDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE heapType,
		UINT numDescriptor,
		D3D12_DESCRIPTOR_HEAP_FLAGS flags,
		ID3D12DescriptorHeap** descriptorHeap);

private:
	ID3D12Device* m_device;
	UINT m_cbvSrvUavDescSize{ 0u };
	UINT m_dsvDescriptorSize{ 0u };
	UINT m_rtvDescriptorSize{ 0u };

	int m_currBackBuffer{ 0 };

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvDescHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvDescHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescHeap;
};

inline void CDescriptorHeap::SetupFirstBackBuffer() { m_currBackBuffer = 0; }
inline int CDescriptorHeap::GetCurrBackBuffer() const { return m_currBackBuffer; }