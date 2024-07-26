#include "pch.h"
#include "./Renderer.h"
#include "./Directx3D.h"
#include "./d3dUtil.h"
#include "../Include/RenderItem.h"
#include "../Include/Types.h"
#include "./CoreDefine.h"
#include "./RootSignature.h"
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
	, m_rootSignature{ nullptr }
	, m_shader{ nullptr }
	, m_texture{ nullptr }
	, m_draw{ nullptr }
	, m_ssaoMap{ nullptr }
	, m_frameResources{ nullptr }
	, m_pso{ nullptr }
{}

bool CRenderer::Initialize(const std::wstring& resPath, HWND hwnd, int width, int height, const ShaderFileList& shaderFileList)
{
	m_directx3D = std::make_unique<CDirectx3D>();
	m_rootSignature = std::make_unique<CRootSignature>();
	m_descHeap = std::make_unique<CDescriptorHeap>();
	ReturnIfFalse(m_directx3D->Initialize(hwnd, width, height, m_descHeap.get()));

	m_shader = std::make_unique<CShader>(resPath, shaderFileList);
	m_texture = std::make_unique<CTexture>(resPath);
	m_draw = std::make_unique<CDraw>(m_directx3D.get());
	m_ssaoMap = std::make_unique<CSsaoMap>(m_descHeap.get());
	m_pso = std::make_unique<CPipelineStateObjects>(m_directx3D.get());
	m_frameResources = std::make_unique<CFrameResources>();
	
	ID3D12Device* device = m_directx3D->GetDevice();
	ReturnIfFalse(m_rootSignature->Build(device));
	ReturnIfFalse(m_pso->Build(m_rootSignature.get(), m_shader.get()));
	ReturnIfFalse(m_frameResources->Build(device, gPassCBCount, gInstanceBufferCount, gMaterialBufferCount));
	ReturnIfFalse(m_draw->Initialize(m_descHeap.get(), m_pso.get()));
	ReturnIfFalse(m_ssaoMap->Initialize(m_directx3D.get(), width, height));

	m_ssaoMap->SetPSOs(m_pso->GetPso(GraphicsPSO::SsaoMap), m_pso->GetPso(GraphicsPSO::SsaoBlur));

	m_isInitialize = true;

	return true;
}

bool CRenderer::OnResize(int width, int height)
{
	ReturnIfFalse(m_directx3D->OnResize(width, height));
	m_draw->OnResize(width, height);

	if (m_ssaoMap == nullptr)
		return true;

	ReturnIfFalse(m_ssaoMap->OnResize(m_directx3D.get(), width, height));
	m_ssaoMap->RebuildDescriptors(m_directx3D->GetDepthStencilBufferResource());
	
	return true;
}

bool CRenderer::WaitUntilGpuFinished(UINT64 fenceCount)
{
	return m_directx3D->WaitUntilGpuFinished(fenceCount);
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

	return m_directx3D->LoadData([&, this](ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)->bool {
		return LoadMesh(device, cmdList, verticesData, indicesData, renderItem); });
}

bool CRenderer::LoadTexture(const TextureList& textureList, std::vector<std::wstring>* srvFilename)
{
	ReturnIfFalse(m_directx3D->LoadData(
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
	return m_draw->Excute(
		m_rootSignature.get(),
		m_frameResources.get(), 
		m_ssaoMap.get(), 
		renderItem);
}

void CRenderer::Set4xMsaaState(HWND hwnd, int width, int height, bool value)
{
	m_directx3D->Set4xMsaaState(hwnd, width, height, value);
}