#include "pch.h"
#include "Directx3D.h"
#include "d3dUtil.h"
#include "./CoreDefine.h"
#include "./DescriptorHeap.h"

using Microsoft::WRL::ComPtr;

CDirectx3D::CDirectx3D()
	: m_dxgiFactory{ nullptr }
	, m_swapChain{ nullptr }
	, m_device{ nullptr }
	, m_fence{ nullptr }
	, m_commandQueue{ nullptr }
	, m_cmdListAlloc{ nullptr }
	, m_commandList{ nullptr }
	, m_descHeap{ nullptr }
	, m_depthStencilBuffer{ nullptr }
{
	m_swapChainBuffer.resize(SwapChainBufferCount);
	for (auto i : std::views::iota(0, static_cast<int>(SwapChainBufferCount)))
		m_swapChainBuffer[i] = nullptr;
}

CDirectx3D::~CDirectx3D()
{
	if (m_device != nullptr)
		FlushCommandQueue();
}

bool CDirectx3D::Initialize(HWND hwnd, int width, int height, CDescriptorHeap* descHeap)
{
	m_descHeap = descHeap;

	ReturnIfFalse(InitDirect3D(hwnd, width, height));
	ReturnIfFalse(m_descHeap->Build(m_device.Get()));
	ReturnIfFalse(OnResize(width, height));

	return true;
}

bool CDirectx3D::InitDirect3D(HWND hwnd, int width, int height)
{
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		ReturnIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ReturnIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

	// Try to create hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_device));

	// Fallback to WARP device.
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ReturnIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ReturnIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)));
	}

	ReturnIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&m_fence)));

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = m_backBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ReturnIfFailed(m_device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

	#ifdef _DEBUG
	CoreUtil::LogAdapters(m_dxgiFactory.Get(), m_backBufferFormat);
	#endif

	ReturnIfFalse(CreateCommandObjects());
	ReturnIfFalse(CreateSwapChain(hwnd, width, height));

	return true;
}

bool CDirectx3D::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ReturnIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	ReturnIfFailed(m_device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_cmdListAlloc.GetAddressOf())));

	ReturnIfFailed(m_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_cmdListAlloc.Get(), // Associated command allocator
		nullptr,                   // Initial PipelineStateObject
		IID_PPV_ARGS(m_commandList.GetAddressOf())));

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	ReturnIfFailed(m_commandList->Close());

	return true;
}

bool CDirectx3D::CreateSwapChain(HWND hwnd, int width, int height)
{
	// Release the previous swapchain we will be recreating.
	m_swapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = m_backBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = hwnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	ReturnIfFailed(m_dxgiFactory->CreateSwapChain(
		m_commandQueue.Get(),
		&sd,
		m_swapChain.GetAddressOf()));

	return true;
}

bool CDirectx3D::OnResize(int width, int height)
{
	assert(m_device);
	assert(m_swapChain);
	assert(m_cmdListAlloc);

	// Flush before changing any resources.
	ReturnIfFalse(FlushCommandQueue());
	ReturnIfFalse(ResetCommandLists());

	// Release the previous resources we will be recreating.
	for (auto i : std::views::iota(0u, SwapChainBufferCount))
		m_swapChainBuffer[i].Reset();
	m_depthStencilBuffer.Reset();

	// Resize the swap chain.
	ReturnIfFailed(m_swapChain->ResizeBuffers(
		SwapChainBufferCount,
		width, height,
		m_backBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	m_descHeap->SetupFirstBackBuffer();

	for(auto i : std::views::iota(0u, SwapChainBufferCount))
	{
		ReturnIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_swapChainBuffer[i])));
		m_descHeap->CreateRenderTargetView(i == 0 ? RtvOffset::SwapChain0 : RtvOffset::SwapChain1,
			nullptr, m_swapChainBuffer[i].Get());
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = m_depthStencilFormat;
	depthStencilDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = m_depthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	CD3DX12_HEAP_PROPERTIES heap_properties(D3D12_HEAP_TYPE_DEFAULT);
	ReturnIfFailed(m_device->CreateCommittedResource(
		&heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(m_depthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	m_descHeap->CreateDepthStencilView(DsvOffset::Common, nullptr, m_depthStencilBuffer.Get());

	// Transition the resource from its initial state to be used as a depth buffer.
	CD3DX12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	m_commandList->ResourceBarrier(1, &barrier);

	ReturnIfFalse(ExcuteCommandLists());
	ReturnIfFalse(FlushCommandQueue());

	return true;
}

bool CDirectx3D::LoadData(std::function<bool(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)> loadGraphicMemory)
{
	ReturnIfFalse(ResetCommandLists());

	ReturnIfFalse(loadGraphicMemory(m_device.Get(), m_commandList.Get()));

	ReturnIfFalse(ExcuteCommandLists());
	ReturnIfFalse(FlushCommandQueue());

	return true;
}

bool CDirectx3D::ResetCommandLists()
{
	ReturnIfFailed(m_commandList->Reset(m_cmdListAlloc.Get(), nullptr));

	return true;
}

bool CDirectx3D::ExcuteCommandLists()
{
	ReturnIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	return true;
}

bool CDirectx3D::ExcuteSwapChain(UINT64* outFenceIdx)
{
	ReturnIfFailed(m_swapChain->Present(0, 0));
	m_descHeap->SwapBackBuffer();

	UINT64 curFenceIdx = ++m_currentFence;
	m_commandQueue->Signal(m_fence.Get(), curFenceIdx);

	(*outFenceIdx) = curFenceIdx;
	return true;
}

bool CDirectx3D::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	m_currentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ReturnIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));
	ReturnIfFalse(WaitUntilGpuFinished(m_currentFence));

	return true;
}

bool CDirectx3D::WaitUntilGpuFinished(UINT64 fenceCount)
{
	if (m_fence->GetCompletedValue() >= fenceCount)
		return true;
	
	HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
	if (eventHandle == nullptr) 
		return false;

	ReturnIfFailed(m_fence->SetEventOnCompletion(m_currentFence, eventHandle));
	WaitForSingleObject(eventHandle, INFINITE);
	CloseHandle(eventHandle);

	return true;
}

void CDirectx3D::SetPipelineStateDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc) noexcept
{
	inoutDesc->RTVFormats[0] = m_backBufferFormat;
	inoutDesc->DSVFormat = m_depthStencilFormat;
	inoutDesc->SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	inoutDesc->SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
}

bool CDirectx3D::Set4xMsaaState(HWND hwnd, int width, int height, bool value)
{
	if (m_4xMsaaState == value)
		return true;
	
	m_4xMsaaState = value;
	ReturnIfFalse(CreateSwapChain(hwnd, width, height));
	ReturnIfFalse(OnResize(width, height));

	return true;
}

ID3D12Resource* CDirectx3D::CurrentBackBuffer() const 
{ 
	return m_swapChainBuffer[m_descHeap->GetCurrBackBuffer()].Get(); 
}