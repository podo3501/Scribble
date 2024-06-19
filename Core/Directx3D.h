#pragma once

#include<wtypes.h>
#include"CoreInterface.h"
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

class CDirectx3D
{
public:
	CDirectx3D(CWindow* pWindow);

	CDirectx3D() = delete;
	CDirectx3D(const CDirectx3D&) = delete;
	CDirectx3D& operator=(const CDirectx3D&) = delete;

	bool Initialize();
	bool ResetCommandLists();
	bool ExcuteCommandLists();
	bool ExcuteSwapChain(UINT64* outFenceIdx);
	bool FlushCommandQueue();
	bool OnResize();

	void SetPipelineStateDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);

	inline ID3D12Device* GetDevice() const;
	inline ID3D12GraphicsCommandList* GetCommandList() const;
	inline ID3D12Fence* GetFence() const;

	inline ID3D12Resource* CurrentBackBuffer() const;
	inline D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	inline D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

private:
	bool InitDirect3D();

	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	bool CreateCommandObjects();
	bool CreateSwapChain();
	bool CreateRtvAndDsvDescriptorHeaps();

	bool CreateDescriptorHeap(
		UINT numDescriptor, 
		D3D12_DESCRIPTOR_HEAP_TYPE heapType, 
		ID3D12DescriptorHeap** descriptorHeap);

	bool MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& lr);
	bool Set4xMsaaState(bool value);

private:
	CWindow* m_window{ nullptr };
	Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgiFactory{ nullptr };
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Device> m_device{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence{ nullptr };
	UINT64 m_currentFence{ 0 };

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_cmdListAlloc{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap{ nullptr };

	static const int SwapChainBufferCount = 2;
	int m_currBackBuffer{ 0 };
	Microsoft::WRL::ComPtr<ID3D12Resource> m_swapChainBuffer[SwapChainBufferCount]{ nullptr, };
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer{ nullptr };

	DXGI_FORMAT m_backBufferFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };
	DXGI_FORMAT m_depthStencilFormat{ DXGI_FORMAT_D24_UNORM_S8_UINT };

	bool m_4xMsaaState{ false };
	UINT m_4xMsaaQuality{ 0 };
};

inline ID3D12Device* CDirectx3D::GetDevice() const { return m_device.Get(); }
inline ID3D12GraphicsCommandList* CDirectx3D::GetCommandList() const {	return m_commandList.Get(); }
inline ID3D12Fence* CDirectx3D::GetFence() const { return m_fence.Get(); }
inline ID3D12Resource* CDirectx3D::CurrentBackBuffer() const { return m_swapChainBuffer[m_currBackBuffer].Get(); }
inline D3D12_CPU_DESCRIPTOR_HANDLE CDirectx3D::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_currBackBuffer,
		m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
}