#include "./Renderer.h"
#include <DirectXColors.h>
#include "./Directx3D.h"
#include "./d3dx12.h"
#include "./d3dUtil.h"
#include "../Include/RendererDefine.h"
#include "../Include/FrameResourceData.h"
#include "../Include/RenderItem.h"
#include "../Include/Types.h"
#include "./CoreDefine.h"
#include "./Shader.h"
#include "./Texture.h"
#include "./ShadowMap.h"
#include "./FrameResources.h"
#include "./SsaoMap.h"

using Microsoft::WRL::ComPtr;

std::unique_ptr<IRenderer> CreateRenderer(std::wstring& resPath, HWND hwnd, int width, int height, const ShaderFileList& shaderFileList)
{
	std::unique_ptr<CRenderer> renderer = std::make_unique<CRenderer>();
	bool bResult = renderer->Initialize(resPath, hwnd, width, height, shaderFileList);
	if (bResult != true)
		return nullptr;

	return std::move(renderer);
}

CRenderer::CRenderer()
	: m_directx3D{ nullptr }
	, m_shader{ nullptr }
	, m_texture{ nullptr }
	, m_shadowMap{ nullptr }
	, m_ssaoMap{ nullptr }
	, m_rootSignature{ nullptr }
	, m_ssaoRootSignature{ nullptr }
	, m_srvDescHeap{ nullptr }
	, m_frameResources{ nullptr }
	, m_psoList{}
{}

CRenderer::~CRenderer() = default;

bool CRenderer::Initialize(const std::wstring& resPath, HWND hwnd, int width, int height, const ShaderFileList& shaderFileList)
{
	m_directx3D = std::make_unique<CDirectx3D>();
	ReturnIfFalse(m_directx3D->Initialize(hwnd, width, height));
	m_device = m_directx3D->GetDevice();
	m_cmdList = m_directx3D->GetCommandList();

	m_shader = std::make_unique<CShader>(resPath, shaderFileList);
	m_texture = std::make_unique<CTexture>(resPath);
	m_shadowMap = std::make_unique<CShadowMap>(this);
	m_ssaoMap = std::make_unique<CSsaoMap>(this);

	ReturnIfFalse(BuildRootSignature());
	ReturnIfFalse(BuildDescriptorHeaps());
	ReturnIfFalse(BuildPSOs());
	ReturnIfFalse(MakeFrameResource());
	ReturnIfFalse(m_shadowMap->Initialize());
	ReturnIfFalse(m_ssaoMap->Initialize(m_directx3D->GetDepthStencilBufferResource(), width, height));

	m_ssaoMap->SetPSOs(m_psoList[GraphicsPSO::SsaoMap].Get(), m_psoList[GraphicsPSO::SsaoBlur].Get());

	m_isInitialize = true;

	return true;
}

bool CRenderer::MakeFrameResource()
{
	m_frameResources = std::make_unique<CFrameResources>();
	ReturnIfFalse(m_frameResources->BuildFrameResources(
		m_device, gPassCBCount, gInstanceBufferCount, gMaterialBufferCount));

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

bool CRenderer::LoadMesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
	Vertices& totalVertices, Indices& totalIndices, RenderItem* renderItem)
{
	ReturnIfFalse(CoreUtil::CreateDefaultBuffer(
		device, cmdList,
		totalVertices.data(),
		renderItem->vertexBufferView.SizeInBytes,
		renderItem->vertexBufferUploader,
		&renderItem->vertexBufferGPU));
	renderItem->vertexBufferView.BufferLocation = renderItem->vertexBufferGPU->GetGPUVirtualAddress();

	ReturnIfFalse(CoreUtil::CreateDefaultBuffer(
		device, cmdList,
		totalIndices.data(),
		renderItem->indexBufferView.SizeInBytes,
		renderItem->indexBufferUploader,
		&renderItem->indexBufferGPU));
	renderItem->indexBufferView.BufferLocation = renderItem->indexBufferGPU->GetGPUVirtualAddress();

	return true;
}

bool CRenderer::LoadMesh(GraphicsPSO pso, Vertices& totalVertices, Indices& totalIndices, RenderItem* renderItem)
{
	if (m_psoList.find(pso) == m_psoList.end())
		return false;	//renderer에서 PSO가 준비되지 않았다.
	
	return LoadData([&, this](ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)->bool {
		return LoadMesh(device, cmdList, totalVertices, totalIndices, renderItem); });
}

bool CRenderer::LoadSkinnedMesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
	const SkinnedVertices& totalVertices, const Indices& totalIndices, RenderItem* renderItem)
{
	ReturnIfFalse(CoreUtil::CreateDefaultBuffer(
		device, cmdList,
		totalVertices.data(),
		renderItem->vertexBufferView.SizeInBytes,
		renderItem->vertexBufferUploader,
		&renderItem->vertexBufferGPU));
	renderItem->vertexBufferView.BufferLocation = renderItem->vertexBufferGPU->GetGPUVirtualAddress();

	ReturnIfFalse(CoreUtil::CreateDefaultBuffer(
		device, cmdList,
		totalIndices.data(),
		renderItem->indexBufferView.SizeInBytes,
		renderItem->indexBufferUploader,
		&renderItem->indexBufferGPU));
	renderItem->indexBufferView.BufferLocation = renderItem->indexBufferGPU->GetGPUVirtualAddress();

	return true;
}

bool CRenderer::LoadSkinnedMesh(const SkinnedVertices& totalVertices, const Indices& totalIndices, RenderItem* renderItem)
{
	if (m_psoList.find(GraphicsPSO::SkinnedOpaque) == m_psoList.end())
		return false;

	return LoadData([&, this](ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)->bool {
		return LoadSkinnedMesh(device, cmdList, totalVertices, totalIndices, renderItem); });
}

bool CRenderer::LoadTexture(const TextureList& textureList)
{
	ReturnIfFalse(LoadData(
		[this, &textureList](ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)->bool {
			return (m_texture->Upload(device, cmdList, textureList)); }));

	m_texture->CreateShaderResourceView(this);

	return true;
}

bool CRenderer::LoadTexture(const TextureList& textureList, std::vector<std::wstring>* srvFilename)
{
	ReturnIfFalse(LoadData(
		[this, &textureList](ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)->bool {
			return (m_texture->Upload(device, cmdList, textureList)); }));

	m_texture->CreateShaderResourceView(this);

	(*srvFilename) = m_srvTexture2DFilename;

	return true;
}

bool CRenderer::SetUploadBuffer(eBufferType bufferType, const void* bufferData, size_t dataSize)
{
	return m_frameResources->SetUploadBuffer(bufferType, bufferData, dataSize);
}

bool CRenderer::PrepareFrame()
{
	return m_frameResources->PrepareFrame(this);
}

bool CRenderer::OnResize(int width, int height)
{
	ReturnIfFalse(m_directx3D->OnResize(width, height));
	// Update the viewport transform to cover the client area.
	m_screenViewport.TopLeftX = 0;
	m_screenViewport.TopLeftY = 0;
	m_screenViewport.Width = static_cast<float>(width);
	m_screenViewport.Height = static_cast<float>(height);
	m_screenViewport.MinDepth = 0.0f;
	m_screenViewport.MaxDepth = 1.0f;

	m_scissorRect = { 0, 0, width, height };

	if (m_ssaoMap == nullptr)
		return true;

	ReturnIfFalse(m_ssaoMap->OnResize(width, height));
	m_ssaoMap->RebuildDescriptors(m_directx3D->GetDepthStencilBufferResource());
	
	return true;
}

bool CRenderer::WaitUntilGpuFinished(UINT64 fenceCount)
{
	return m_directx3D->WaitUntilGpuFinished(fenceCount);
}


enum class MainRegisterType : int
{
	Pass = 0,
	Bone,
	Material,
	Instance,
	Shadow,
	Ssao,
	Cube,
	Diffuse,
};

bool CRenderer::BuildRootSignature()
{
	ReturnIfFalse(BuildMainRootSignature());
	ReturnIfFalse(BuildSsaoRootSignature());

	return true;
}

template<typename T>
CD3DX12_ROOT_PARAMETER* GetRootParameter(
	std::vector<CD3DX12_ROOT_PARAMETER>& rootParameter, T type)
{
	rootParameter.resize(rootParameter.size() + 1);
	return &rootParameter[EtoV(type)];
};

bool CRenderer::BuildMainRootSignature()
{
	using enum MainRegisterType;

	CD3DX12_DESCRIPTOR_RANGE shadowTexTable{}, ssaoTexTable{}, cubeTexTable{}, texTable{};

	shadowTexTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, ShadowCount, 0, 0); //t0
	ssaoTexTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SsaoAmbientMap0Count, 1, 0); //t1
	cubeTexTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, CubeCount, 2, 0);	//t2
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, TextureCount, 3, 0);	//t3...t10(세번째인자)

	std::vector<CD3DX12_ROOT_PARAMETER> rp{};
	GetRootParameter(rp, Pass)->InitAsConstantBufferView(0);
	GetRootParameter(rp, Bone)->InitAsConstantBufferView(1);
	GetRootParameter(rp, Material)->InitAsShaderResourceView(0, 1);
	GetRootParameter(rp, Instance)->InitAsShaderResourceView(1, 1);
	GetRootParameter(rp, Shadow)->InitAsDescriptorTable(1, &shadowTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
	GetRootParameter(rp, Ssao)->InitAsDescriptorTable(1, &ssaoTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
	GetRootParameter(rp, Cube)->InitAsDescriptorTable(1, &cubeTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
	GetRootParameter(rp, Diffuse)->InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = CoreUtil::GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
		static_cast<UINT>(rp.size()), rp.data(), static_cast<UINT>(staticSamplers.size()), staticSamplers.data(),
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

bool CRenderer::BuildSsaoRootSignature()
{
	using enum SsaoRegisterType;

	CD3DX12_DESCRIPTOR_RANGE normalTexTable{}, depthTexTable{}, randomVecTable{};

	normalTexTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SsaoNormalMapCount, 0, 0); //t0
	depthTexTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SsaoDepthMapCount, 1, 0); //t1
	randomVecTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SsaoRandomVectorCount, 2, 0);	//t2

	std::vector<CD3DX12_ROOT_PARAMETER> rp{};
	GetRootParameter(rp, Pass)->InitAsConstantBufferView(0);
	GetRootParameter(rp, Constants)->InitAsConstants(1, 1);
	GetRootParameter(rp, Normal)->InitAsDescriptorTable(1, &normalTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
	GetRootParameter(rp, Depth)->InitAsDescriptorTable(1, &depthTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
	GetRootParameter(rp, RandomVec)->InitAsDescriptorTable(1, &randomVecTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto ssaoSamplers = CoreUtil::GetSsaoSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
		static_cast<UINT>(rp.size()), rp.data(), static_cast<UINT>(ssaoSamplers.size()), ssaoSamplers.data(),
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
		IID_PPV_ARGS(&m_ssaoRootSignature)));

	return true;
}

bool CRenderer::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = TotalShaderResourceViewHeap;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ReturnIfFailed(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_srvDescHeap)));

	return true;
}

bool CRenderer::BuildPSOs()
{
	return std::ranges::all_of(m_shader->GetPSOList(), [this](auto pso) { return MakePSOPipelineState(pso); });
}

bool CRenderer::MakePSOPipelineState(GraphicsPSO psoType)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	ReturnIfFalse(m_shader->SetPipelineStateDesc(psoType, &psoDesc));

	MakeBasicDesc(&psoDesc);

	switch (psoType)
	{
	case GraphicsPSO::Sky:								MakeSkyDesc(&psoDesc);							break;
	case GraphicsPSO::Opaque:						MakeOpaqueDesc(&psoDesc);					break;
	case GraphicsPSO::NormalOpaque:			MakeNormalOpaqueDesc(&psoDesc);		break;
	case GraphicsPSO::ShadowMap:				MakeShadowDesc(&psoDesc);					break;
	case GraphicsPSO::SsaoDrawNormals:		MakeDrawNormals(&psoDesc);				break;
	case GraphicsPSO::SsaoMap:					MakeSsaoDesc(&psoDesc);						break;
	case GraphicsPSO::SsaoBlur:						MakeSsaoBlurDesc(&psoDesc);				break;
	case GraphicsPSO::Debug:							MakeDebugDesc(&psoDesc);					break;
	default: assert(!"wrong type");
	}
	
	ReturnIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_psoList[psoType])));

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

void CRenderer::MakeNormalOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{
	inoutDesc->DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;
	inoutDesc->DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
}

void CRenderer::MakeShadowDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{
	inoutDesc->RasterizerState.DepthBias = 10000;
	inoutDesc->RasterizerState.DepthBiasClamp = 0.0f;
	inoutDesc->RasterizerState.SlopeScaledDepthBias = 1.0f;
	inoutDesc->RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	inoutDesc->NumRenderTargets = 0;
}

void CRenderer::MakeDrawNormals(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{
	inoutDesc->RTVFormats[0] = CSsaoMap::NormalMapFormat;
	inoutDesc->SampleDesc.Count = 1;
	inoutDesc->SampleDesc.Quality = 0;
	inoutDesc->DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
}

void CRenderer::MakeSsaoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{
	inoutDesc->InputLayout = { nullptr, 0 };
	inoutDesc->pRootSignature = m_ssaoRootSignature.Get();
	inoutDesc->DepthStencilState.DepthEnable = false;
	inoutDesc->DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	inoutDesc->RTVFormats[0] = CSsaoMap::AmbientMapFormat;
	inoutDesc->SampleDesc.Count = 1;
	inoutDesc->SampleDesc.Quality = 0;
	inoutDesc->DSVFormat = DXGI_FORMAT_UNKNOWN;
}

void CRenderer::MakeSsaoBlurDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{
	MakeSsaoDesc(inoutDesc);
}

void CRenderer::MakeDebugDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{}

D3D12_GPU_DESCRIPTOR_HANDLE CRenderer::GetGpuSrvHandle(eTextureType type)
{
	static UINT cbvSrvUavDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	UINT srvStartIndex = static_cast<UINT>(EtoV(type));
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandle{ m_srvDescHeap->GetGPUDescriptorHandleForHeapStart() };
	gpuDescHandle.Offset(srvStartIndex, cbvSrvUavDescSize);
	return gpuDescHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE CRenderer::GetCpuSrvHandle(eTextureType type)
{
	static UINT cbvSrvUavDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	UINT srvStartIndex = static_cast<UINT>(EtoV(type));
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandle{ m_srvDescHeap->GetCPUDescriptorHandleForHeapStart() };
	cpuDescHandle.Offset(srvStartIndex, cbvSrvUavDescSize);
	return cpuDescHandle;
}

D3D12_GPU_VIRTUAL_ADDRESS CRenderer::GetFrameResourceAddress(eBufferType bufType)
{
	return m_frameResources->GetResource(bufType)->GetGPUVirtualAddress();
}

bool CRenderer::Draw(AllRenderItems& renderItem)
{
	auto cmdListAlloc = m_frameResources->GetCurrCmdListAlloc();
	ReturnIfFailed(cmdListAlloc->Reset());
	ReturnIfFailed(m_cmdList->Reset(cmdListAlloc, nullptr));

	m_cmdList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvDescHeap.Get() };
	m_cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_cmdList->SetGraphicsRootShaderResourceView(EtoV(MainRegisterType::Material), GetFrameResourceAddress(eBufferType::Material));
	m_cmdList->SetGraphicsRootDescriptorTable(EtoV(MainRegisterType::Diffuse), GetGpuSrvHandle(eTextureType::Texture2D));

	DrawSceneToShadowMap(renderItem);
	DrawNormalsAndDepth(renderItem);

	m_cmdList->SetGraphicsRootSignature(m_ssaoRootSignature.Get());
	m_ssaoMap->ComputeSsao(m_cmdList, m_frameResources.get(), 3);

	m_cmdList->SetGraphicsRootSignature(m_rootSignature.Get());

	m_cmdList->RSSetViewports(1, &m_screenViewport);
	m_cmdList->RSSetScissorRects(1, &m_scissorRect);

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_directx3D->CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)));

	m_cmdList->ClearRenderTargetView(m_directx3D->CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);

	m_cmdList->OMSetRenderTargets(1, &RvToLv(m_directx3D->CurrentBackBufferView()), true, &RvToLv(m_directx3D->GetCpuDsvHandle(DsvOffset::Common)));

	m_cmdList->SetGraphicsRootConstantBufferView(EtoV(MainRegisterType::Pass), GetFrameResourceAddress(eBufferType::PassCB));
	m_cmdList->SetGraphicsRootShaderResourceView(EtoV(MainRegisterType::Material), GetFrameResourceAddress(eBufferType::Material));
	m_cmdList->SetGraphicsRootDescriptorTable(EtoV(MainRegisterType::Shadow), GetGpuSrvHandle(eTextureType::ShadowMap));
	m_cmdList->SetGraphicsRootDescriptorTable(EtoV(MainRegisterType::Ssao), GetGpuSrvHandle(eTextureType::SsaoAmbientMap0));
	m_cmdList->SetGraphicsRootDescriptorTable(EtoV(MainRegisterType::Cube), GetGpuSrvHandle(eTextureType::TextureCube));
	m_cmdList->SetGraphicsRootDescriptorTable(EtoV(MainRegisterType::Diffuse), GetGpuSrvHandle(eTextureType::Texture2D));

	std::ranges::for_each(renderItem, [this, &renderItem](auto& curRenderItem) {
		auto pso = curRenderItem.first;
		m_cmdList->SetPipelineState(m_psoList[pso].Get());
		DrawRenderItems(m_frameResources->GetResource(eBufferType::Instance), renderItem[pso].get()); });

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_directx3D->CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));

	ReturnIfFalse(m_directx3D->ExcuteCommandLists());

	UINT64 curFenceIdx{ 0 };
	ReturnIfFalse(m_directx3D->ExcuteSwapChain(&curFenceIdx));
	m_frameResources->SetFence(curFenceIdx);

	return true;
}

void CRenderer::DrawSceneToShadowMap(AllRenderItems& renderItem)
{
	m_cmdList->RSSetViewports(1, &RvToLv(m_shadowMap->Viewport()));
	m_cmdList->RSSetScissorRects(1, &RvToLv(m_shadowMap->ScissorRect()));

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_shadowMap->Resource(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE)));
	D3D12_CPU_DESCRIPTOR_HANDLE dsvShadowMap = m_directx3D->GetCpuDsvHandle(DsvOffset::ShadowMap);
	m_cmdList->ClearDepthStencilView(dsvShadowMap, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_cmdList->OMSetRenderTargets(0, nullptr, false, &dsvShadowMap);

	UINT passCBByteSize = CoreUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
	//2개의 cb가 들어가 있는데 2번째를 가져올 함수가 아직 없다. -> + 1 * passCBByteSize;
	D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = GetFrameResourceAddress(eBufferType::PassCB) + 1 * passCBByteSize;
	m_cmdList->SetGraphicsRootConstantBufferView(EtoV(MainRegisterType::Pass), passCBAddress);

	m_cmdList->SetPipelineState(m_psoList[GraphicsPSO::ShadowMap].Get());

	DrawRenderItems(m_frameResources->GetResource(eBufferType::Instance), renderItem[GraphicsPSO::NormalOpaque].get());

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_shadowMap->Resource(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ)));
}

void CRenderer::DrawNormalsAndDepth(AllRenderItems& renderItem)
{
	m_cmdList->RSSetViewports(1, &m_screenViewport);
	m_cmdList->RSSetScissorRects(1, &m_scissorRect);

	auto normalMap = m_ssaoMap->NormalMap();
	auto normalMapRtv = m_directx3D->GetCpuRtvHandle(RtvOffset::NormalMap);

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(normalMap,
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET)));

	float clearValue[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	m_cmdList->ClearRenderTargetView(normalMapRtv, clearValue, 0, nullptr);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCommon = m_directx3D->GetCpuDsvHandle(DsvOffset::Common);
	m_cmdList->ClearDepthStencilView(dsvCommon, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_cmdList->OMSetRenderTargets(1, &normalMapRtv, true, &dsvCommon);
	m_cmdList->SetGraphicsRootConstantBufferView(EtoV(MainRegisterType::Pass), GetFrameResourceAddress(eBufferType::PassCB));
	m_cmdList->SetPipelineState(m_psoList[GraphicsPSO::SsaoDrawNormals].Get());

	DrawRenderItems(m_frameResources->GetResource(eBufferType::Instance), renderItem[GraphicsPSO::NormalOpaque].get());

	m_cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(normalMap,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ)));
}


void CRenderer::DrawRenderItems(ID3D12Resource* instanceRes, RenderItem* renderItem)
{
	m_cmdList->IASetVertexBuffers(0, 1, &renderItem->vertexBufferView);
	m_cmdList->IASetIndexBuffer(&renderItem->indexBufferView);
	m_cmdList->IASetPrimitiveTopology(renderItem->primitiveType);

	for (auto& ri : renderItem->subRenderItems)
	{
		auto& subRenderItem = ri.second;
		auto& subItem = subRenderItem.subItem;

		m_cmdList->SetGraphicsRootShaderResourceView(EtoV(MainRegisterType::Instance),
			instanceRes->GetGPUVirtualAddress() + 
			(renderItem->startIndexInstance + subRenderItem.startSubIndexInstance) * sizeof(InstanceBuffer));

		m_cmdList->DrawIndexedInstanced(subItem.indexCount, subRenderItem.instanceCount,
			subItem.startIndexLocation, subItem.baseVertexLocation, 0);
	}
}

void CRenderer::Set4xMsaaState(HWND hwnd, int width, int height, bool value)
{
	m_directx3D->Set4xMsaaState(hwnd, width, height, value);
}

UINT CRenderer::GetSrvIndex(eTextureType type)
{
	UINT srvStartIndex = static_cast<UINT>(EtoV(type));
	UINT srvOffset{ 0 };
	if (type == eTextureType::Texture2D)
		srvOffset = m_srvOffsetTexture2D++;

	return srvStartIndex + srvOffset;
}

void CRenderer::CreateShaderResourceView(eTextureType type, const std::wstring& filename, 
	ID3D12Resource* pRes, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc)
{
	static UINT cbvSrvUavDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	UINT index = GetSrvIndex(type);

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDesc{ m_srvDescHeap->GetCPUDescriptorHandleForHeapStart() };
	hCpuDesc.Offset(index, cbvSrvUavDescSize);
	m_device->CreateShaderResourceView(pRes, pDesc, hCpuDesc);
	
	if(type == eTextureType::Texture2D)
		m_srvTexture2DFilename.emplace_back(filename);
}




