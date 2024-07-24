#include "./Renderer.h"
#include <ranges>
#include "./Directx3D.h"
#include "./d3dUtil.h"
#include "./headerUtility.h"
#include "../Include/RenderItem.h"
#include "../Include/Types.h"
#include "./CoreDefine.h"
#include "./Shader.h"
#include "./Texture.h"
#include "./FrameResources.h"
#include "./SsaoMap.h"
#include "./PipelineStateObjects.h"
#include "./DescriptorHeap.h"
#include "./Draw.h"

using Microsoft::WRL::ComPtr;

std::unique_ptr<IRenderer> CreateRenderer(const std::wstring& resPath, HWND hwnd, int width, int height, const ShaderFileList& shaderFileList)
{
	std::unique_ptr<CRenderer> renderer = std::make_unique<CRenderer>();
	bool bResult = renderer->Initialize(resPath, hwnd, width, height, shaderFileList);
	if (bResult != true)
		return nullptr;

	return std::move(renderer);
}

CRenderer::~CRenderer() = default;
CRenderer::CRenderer()
	: m_directx3D{ nullptr }
	, m_shader{ nullptr }
	, m_texture{ nullptr }
	, m_draw{ nullptr }
	, m_ssaoMap{ nullptr }
	, m_rootSignatures{}
	, m_frameResources{ nullptr }
	, m_pso{ nullptr }
{}

bool CRenderer::Initialize(const std::wstring& resPath, HWND hwnd, int width, int height, const ShaderFileList& shaderFileList)
{
	m_directx3D = std::make_unique<CDirectx3D>();
	m_descHeap = std::make_unique<CDescriptorHeap>();
	ReturnIfFalse(m_directx3D->Initialize(hwnd, width, height, m_descHeap.get()));
	m_device = m_directx3D->GetDevice();
	m_cmdList = m_directx3D->GetCommandList();

	m_shader = std::make_unique<CShader>(resPath, shaderFileList);
	m_texture = std::make_unique<CTexture>(resPath);
	m_draw = std::make_unique<CDraw>();
	m_ssaoMap = std::make_unique<CSsaoMap>(this, m_descHeap.get());
	m_pso = std::make_unique<CPipelineStateObjects>(this);
	
	ReturnIfFalse(BuildRootSignature());
	ReturnIfFalse(m_pso->Build(m_shader.get()));
	ReturnIfFalse(MakeFrameResource());
	ReturnIfFalse(m_draw->Initialize(this, m_descHeap.get(), m_pso.get()));
	ReturnIfFalse(m_ssaoMap->Initialize(m_directx3D->GetDepthStencilBufferResource(), width, height));

	m_ssaoMap->SetPSOs(m_pso->GetPso(GraphicsPSO::SsaoMap), m_pso->GetPso(GraphicsPSO::SsaoBlur));

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

bool CRenderer::OnResize(int width, int height)
{
	ReturnIfFalse(m_directx3D->OnResize(width, height));
	m_draw->OnResize(width, height);

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

	return CreateRootSignature(RootSignature::Common, rp, CoreUtil::GetStaticSamplers());
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

	return CreateRootSignature(RootSignature::Ssao, rp, CoreUtil::GetSsaoSamplers());
}

bool CRenderer::CreateRootSignature(RootSignature type,
	const std::vector<CD3DX12_ROOT_PARAMETER>& rootParamList,
	std::vector<D3D12_STATIC_SAMPLER_DESC> samplers)
{
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
		static_cast<UINT>(rootParamList.size()), rootParamList.data(),
		static_cast<UINT>(samplers.size()), samplers.data(),
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
		IID_PPV_ARGS(&m_rootSignatures[EtoV(type)])));

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
	const void* verticesData, const void* indicesData, RenderItem* renderItem)
{
	auto CreateDefaultBuffer = [device, cmdList](auto data, auto& bufferView, auto& uploader, auto& bufferGpu)->bool {
		ReturnIfFalse(CoreUtil::CreateDefaultBuffer( device, cmdList, data, bufferView.SizeInBytes, uploader, &bufferGpu));
		bufferView.BufferLocation = bufferGpu->GetGPUVirtualAddress();
		return true;
		};

	ReturnIfFalse(CreateDefaultBuffer(verticesData, renderItem->vertexBufferView, renderItem->vertexBufferUploader, renderItem->vertexBufferGPU));
	ReturnIfFalse(CreateDefaultBuffer(indicesData, renderItem->indexBufferView, renderItem->indexBufferUploader, renderItem->indexBufferGPU));

	return true;
}

bool CRenderer::LoadMesh(GraphicsPSO pso, const void* verticesData, const void* indicesData, RenderItem* renderItem)
{
	if (m_pso->GetPso(pso) == nullptr)
		return false;	//renderer에서 PSO가 준비되지 않았다.

	return LoadData([&, this](ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)->bool {
		return LoadMesh(device, cmdList, verticesData, indicesData, renderItem); });
}

bool CRenderer::LoadTexture(const TextureList& textureList, std::vector<std::wstring>* srvFilename)
{
	ReturnIfFalse(LoadData(
		[this, &textureList](ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)->bool {
			return (m_texture->Upload(device, cmdList, textureList)); }));

	m_texture->CreateShaderResourceView(m_descHeap.get());
	(*srvFilename) = m_texture->GetListSrvTexture2D();

	return true;
}

bool CRenderer::SetUploadBuffer(eBufferType bufferType, const void* bufferData, size_t dataSize)
{
	return m_frameResources->SetUploadBuffer(bufferType, bufferData, dataSize);
}

bool CRenderer::PrepareFrame()
{
	UINT64 fenceCount = m_frameResources->ForwardFrame();
	if (fenceCount == 0) return true;

	ReturnIfFalse(WaitUntilGpuFinished(fenceCount));

	return true;
}

bool CRenderer::Draw(AllRenderItems& renderItem)
{
	return m_draw->Excute(m_frameResources.get(), m_ssaoMap.get(), renderItem);
}

void CRenderer::Set4xMsaaState(HWND hwnd, int width, int height, bool value)
{
	m_directx3D->Set4xMsaaState(hwnd, width, height, value);
}

ID3D12RootSignature* CRenderer::GetRootSignature(RootSignature sigType) 
{ 
	return m_rootSignatures[EtoV(sigType)].Get(); 
}




