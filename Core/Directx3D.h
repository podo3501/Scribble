#pragma once

#include<wtypes.h>
#include"CoreInterface.h"
#include<memory>
#include <wrl.h>
#include <dxgiformat.h>
#include <d3d12.h>

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

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

class CWindow;

class CDirectx3D
{
public:
	CDirectx3D(HINSTANCE hInstance);
	bool Initialize(WNDPROC wndProc);

	void ResetCommandLists();
	void FlushCommandQueue();
	void OnResize();

public:
	CDirectx3D(const CDirectx3D&) = delete;
	CDirectx3D& operator=(const CDirectx3D&) = delete;

private:
	bool InitDirect3D();

	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();

	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

private:
	std::unique_ptr<CWindow> m_window{ nullptr };

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

	UINT m_rtvDescriptorSize{ 0 };
	UINT m_dsvDescriptorSize{ 0 };
	UINT m_cbvSrvUavDescriptorSize{ 0 };

	DXGI_FORMAT m_backBufferFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };
	DXGI_FORMAT m_depthStencilFormat{ DXGI_FORMAT_D24_UNORM_S8_UINT };

	bool m_4xMsaaState{ false };
	UINT m_4xMsaaQuality{ 0 };

	D3D12_VIEWPORT m_screenViewport{};
	D3D12_RECT m_scissorRect{};
};