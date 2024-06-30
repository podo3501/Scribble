#include "Renderer.h"
#include "./RendererDefine.h"
#include <DirectXColors.h>
#include "../Core/Directx3D.h"
#include "../Core/d3dx12.h"
#include "../Core/d3dUtil.h"
#include "../Core/Utility.h"
#include "./GameTimer.h"
#include "./UploadBuffer.h"
#include "./FrameResource.h"
#include "./FrameResourceData.h"
#include "./RenderItem.h"
#include "./Shader.h"

using Microsoft::WRL::ComPtr;

std::unique_ptr<IRenderer> CreateRenderer(std::wstring resPath, CWindow* window)
{
	std::unique_ptr<CRenderer> renderer = std::make_unique<CRenderer>(resPath);
	bool bResult = renderer->Initialize(window);
	if (bResult != true)
		return nullptr;

	return std::move(renderer);
}

CRenderer::CRenderer(std::wstring resPath)
	: m_resPath(std::move(resPath))
{}
CRenderer::~CRenderer() = default;

bool CRenderer::Initialize(CWindow* window)
{
	m_shader = std::make_unique<CShader>(m_resPath);
	m_directx3D = std::make_unique<CDirectx3D>(window);
	ReturnIfFalse(m_directx3D->Initialize());

	m_device = m_directx3D->GetDevice();
	m_cmdList = m_directx3D->GetCommandList();

	m_psoList.resize(EtoV(GraphicsPSO::Count));
	ReturnIfFalse(BuildRootSignature());
	ReturnIfFalse(BuildDescriptorHeaps());
	ReturnIfFalse(BuildPSOs());

	return true;
}

bool CRenderer::LoadData(std::function<bool(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)> loadGraphicMemory)
{
	ReturnIfFalse(m_directx3D->ResetCommandLists());

	ReturnIfFalse(loadGraphicMemory(m_device, m_cmdList));

	ReturnIfFalse(m_directx3D->ExcuteCommandLists());
	ReturnIfFalse(m_directx3D->FlushCommandQueue());

	return true;
}

bool CRenderer::OnResize(int wndWidth, int wndHeight)
{
	ReturnIfFalse(m_directx3D->OnResize());
	// Update the viewport transform to cover the client area.
	m_screenViewport.TopLeftX = 0;
	m_screenViewport.TopLeftY = 0;
	m_screenViewport.Width = static_cast<float>(wndWidth);
	m_screenViewport.Height = static_cast<float>(wndHeight);
	m_screenViewport.MinDepth = 0.0f;
	m_screenViewport.MaxDepth = 1.0f;

	m_scissorRect = { 0, 0, wndWidth, wndHeight };

	return true;
}

bool CRenderer::WaitUntilGpuFinished(UINT64 fenceCount)
{
	//fence를 renderer로 올린다.
	return m_directx3D->WaitUntilGpuFinished(fenceCount);
}


enum class ParamType : int
{
	Pass = 0,
	Material,
	Instance,
	Cube,
	Diffuse,
	Count,
};

//셰이더의 내용물은 늘 자료가 있다고 가정한다.
//남아서 넘치는 건 상관없지만 셰이더 데이터에 빈공간이 있으면 안된다.
constexpr UINT CubeCount{ 1u };
constexpr UINT TextureCount{ 7u };
constexpr UINT TotalHeapCount = CubeCount + TextureCount;
bool CRenderer::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE cubeTexTable{}, texTable{};
	
	cubeTexTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, CubeCount, 0, 0);	//t0
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, TextureCount, 1, 0);	//t1...t10(세번째인자)

	std::array<CD3DX12_ROOT_PARAMETER, EtoV(ParamType::Count)> slotRootParameter;
	auto GetRootParameter = [&slotRootParameter](ParamType type) {
		return &slotRootParameter[EtoV(type)];	};

	GetRootParameter(ParamType::Pass)->InitAsConstantBufferView(0);
	GetRootParameter(ParamType::Material)->InitAsShaderResourceView(0, 1);
	GetRootParameter(ParamType::Instance)->InitAsShaderResourceView(1, 1);
	GetRootParameter(ParamType::Cube)->InitAsDescriptorTable(1, &cubeTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
	GetRootParameter(ParamType::Diffuse)->InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = CoreUtil::GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
		static_cast<UINT>(slotRootParameter.size()), slotRootParameter.data(),
		static_cast<UINT>(staticSamplers.size()), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serialized = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serialized.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ReturnIfFailed(hr);

	ReturnIfFailed(m_device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)));

	return true;
}

bool CRenderer::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = TotalHeapCount;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ReturnIfFailed(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_srvDescHeap)));

	return true;
}

bool CRenderer::BuildPSOs()
{
	return std::ranges::all_of(GraphicsPSO_ALL, [this](auto pso) { return MakePSOPipelineState(pso); });
}

bool CRenderer::MakePSOPipelineState(GraphicsPSO psoType)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	ReturnIfFalse(m_shader->SetPipelineStateDesc(psoType, &psoDesc));
	MakeBasicDesc(&psoDesc);

	switch (psoType)
	{
	case GraphicsPSO::Sky:				MakeSkyDesc(&psoDesc);				break;
	case GraphicsPSO::Opaque:		MakeOpaqueDesc(&psoDesc);		break;
	default: assert(!"wrong type");
	}
	
	ReturnIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, 
		IID_PPV_ARGS(&m_psoList[EtoV(psoType)])));

	return true;
}

void CRenderer::MakeBasicDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{
	inoutDesc->NodeMask = 0;
	inoutDesc->SampleMask = UINT_MAX;
	inoutDesc->NumRenderTargets = 1;
	inoutDesc->pRootSignature = m_rootSignature.Get();
	inoutDesc->BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	inoutDesc->DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	inoutDesc->RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	inoutDesc->PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	m_directx3D->SetPipelineStateDesc(inoutDesc);
}

void CRenderer::MakeSkyDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{
	inoutDesc->RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	inoutDesc->DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void CRenderer::MakeOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{}

bool CRenderer::Draw(CGameTimer* gt, CFrameResources* frameResources,
	std::unordered_map<std::string, std::unique_ptr<RenderItem>>& renderItem)
{
	auto cmdListAlloc = frameResources->GetCurrCmdListAlloc();
	ReturnIfFailed(cmdListAlloc->Reset());
	ReturnIfFailed(m_cmdList->Reset(cmdListAlloc, m_psoList[EtoV(GraphicsPSO::Sky)].Get()));

	m_cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_cmdList->RSSetViewports(1, &m_screenViewport);
	m_cmdList->RSSetScissorRects(1, &m_scissorRect);

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_directx3D->CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)));

	m_cmdList->ClearRenderTargetView(m_directx3D->CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
	m_cmdList->ClearDepthStencilView(m_directx3D->DepthStencilView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_cmdList->OMSetRenderTargets(1, &RvToLv(m_directx3D->CurrentBackBufferView()), true, &RvToLv(m_directx3D->DepthStencilView()));

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvDescHeap.Get() };
	m_cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	auto matBuf = frameResources->GetUploadBuffer(eBufferType::Material);
	m_cmdList->SetGraphicsRootShaderResourceView(EtoV(ParamType::Material), matBuf->Resource()->GetGPUVirtualAddress());

	auto passCB = frameResources->GetUploadBuffer(eBufferType::PassCB);
	m_cmdList->SetGraphicsRootConstantBufferView(EtoV(ParamType::Pass), passCB->Resource()->GetGPUVirtualAddress());

	m_cmdList->SetGraphicsRootDescriptorTable(EtoV(ParamType::Cube), m_srvDescHeap->GetGPUDescriptorHandleForHeapStart());

	UINT cbvSrvUavDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_GPU_DESCRIPTOR_HANDLE texDescriptor(m_srvDescHeap->GetGPUDescriptorHandleForHeapStart());
	texDescriptor.Offset(CubeCount, cbvSrvUavDescSize);
	m_cmdList->SetGraphicsRootDescriptorTable(EtoV(ParamType::Diffuse), texDescriptor);

	DrawRenderItems(frameResources->GetUploadBuffer(eBufferType::Instance), renderItem["nature"].get());

	m_cmdList->SetPipelineState(m_psoList[EtoV(GraphicsPSO::Opaque)].Get());
	DrawRenderItems(frameResources->GetUploadBuffer(eBufferType::Instance), renderItem["things"].get());

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_directx3D->CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));

	ReturnIfFalse(m_directx3D->ExcuteCommandLists());

	UINT64 curFenceIdx{ 0 };
	ReturnIfFalse(m_directx3D->ExcuteSwapChain(&curFenceIdx));
	frameResources->SetFence(curFenceIdx);

	return true;
}

void CRenderer::DrawRenderItems(CUploadBuffer* instanceBuffer, RenderItem* renderItem)
{
	m_cmdList->IASetVertexBuffers(0, 1, &renderItem->vertexBufferView);
	m_cmdList->IASetIndexBuffer(&renderItem->indexBufferView);
	m_cmdList->IASetPrimitiveTopology(renderItem->primitiveType);

	for (auto& ri : renderItem->subRenderItems)
	{
		auto& subRenderItem = ri.second;
		auto& subItem = subRenderItem.subItem;
		m_cmdList->SetGraphicsRootShaderResourceView(EtoV(ParamType::Instance),
			instanceBuffer->Resource()->GetGPUVirtualAddress() + subRenderItem.startIndexInstance * sizeof(InstanceBuffer));

		m_cmdList->DrawIndexedInstanced(subItem.indexCount, subRenderItem.instanceCount,
			subItem.startIndexLocation, subItem.baseVertexLocation, 0);
	}
}



