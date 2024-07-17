#pragma once

#include<wtypes.h>
#include<vector>
#include<memory>
#include <wrl.h>
#include <dxgiformat.h>
#include <d3d12.h>
#include <functional>
#include "../Core/d3dx12.h"

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "../Lib/DirectXTK12_d.lib")
#else
#pragma comment(lib, "../Lib/DirectXTK12.lib")
#endif

class CWindow;
struct IDXGIFactory4;
struct ID3D12Device;
struct ID3D12Fence;
struct IDXGIAdapter;
struct IDXGIOutput;
struct ID3D12CommandQueue;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct IDXGISwapChain;
struct ID3D12DescriptorHeap;
struct ID3D12Resource;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct D3D12_GRAPHICS_PIPELINE_STATE_DESC;
enum class RtvOffset : int;
enum class DsvOffset : int;

class CDirectx3D
{
public:
	CDirectx3D();
	~CDirectx3D();

	CDirectx3D(const CDirectx3D&) = delete;
	CDirectx3D& operator=(const CDirectx3D&) = delete;

	bool Initialize(HWND hwnd, int width, int height);
	bool ResetCommandLists();
	bool ExcuteCommandLists();
	bool ExcuteSwapChain(UINT64* outFenceIdx);
	bool FlushCommandQueue();
	bool WaitUntilGpuFinished(UINT64 fenceCount);
	bool OnResize(int width, int height);
	bool Set4xMsaaState(HWND hwnd, int width, int height, bool value);

	void SetPipelineStateDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc) noexcept;
	void CreateDepthStencilView(DsvOffset offset, ID3D12Resource* pRes,
		const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc);
	void CreateRenderTargetView(RtvOffset offsetType, ID3D12Resource* pRes,
		const D3D12_RENDER_TARGET_VIEW_DESC* pDesc);

	inline ID3D12Device* GetDevice() const;
	inline ID3D12GraphicsCommandList* GetCommandList() const;
	inline ID3D12Fence* GetFence() const;

	inline ID3D12Resource* GetDepthStencilBufferResource() const;
	inline ID3D12Resource* CurrentBackBuffer() const;
	inline D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDsvHandle(DsvOffset offset);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuRtvHandle(RtvOffset rtvOffset);

private:
	bool InitDirect3D(HWND hwnd, int width, int height);

	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	bool CreateCommandObjects();
	bool CreateSwapChain(HWND hwnd, int width, int height);
	bool CreateRtvAndDsvDescriptorHeaps();

	bool CreateDescriptorHeap(
		UINT numDescriptor, 
		D3D12_DESCRIPTOR_HEAP_TYPE heapType, 
		ID3D12DescriptorHeap** descriptorHeap);

private:
	Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> m_device;

	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	UINT64 m_currentFence{ 0 };

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_cmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

	int m_currBackBuffer{ 0 };
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_swapChainBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;

	DXGI_FORMAT m_backBufferFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };
	DXGI_FORMAT m_depthStencilFormat{ DXGI_FORMAT_D24_UNORM_S8_UINT };

	bool m_4xMsaaState{ false };
	UINT m_4xMsaaQuality{ 0 };
};

inline ID3D12Device* CDirectx3D::GetDevice() const { return m_device.Get(); }
inline ID3D12GraphicsCommandList* CDirectx3D::GetCommandList() const {	return m_commandList.Get(); }
inline ID3D12Fence* CDirectx3D::GetFence() const { return m_fence.Get(); }
inline ID3D12Resource* CDirectx3D::GetDepthStencilBufferResource() const { return m_depthStencilBuffer.Get(); };
inline ID3D12Resource* CDirectx3D::CurrentBackBuffer() const { return m_swapChainBuffer[m_currBackBuffer].Get(); }
inline D3D12_CPU_DESCRIPTOR_HANDLE CDirectx3D::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_currBackBuffer,
		m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
}