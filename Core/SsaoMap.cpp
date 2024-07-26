#include "pch.h"
#include "./SsaoMap.h"
#include "../Include/FrameResourceData.h"
#include "../Include/types.h"
#include "./Directx3D.h"
#include "./d3dUtil.h"
#include "./FrameResources.h"
#include "./DescriptorHeap.h"
#include "./CoreDefine.h"

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

CSsaoMap::~CSsaoMap() = default;
CSsaoMap::CSsaoMap(CDescriptorHeap* descHeap)
	: m_descHeap{ descHeap }
	, m_ssaoRootSig{ nullptr }
	, m_ssaoPso{ nullptr }
	, m_blurPso{ nullptr }
	, m_randomVectorMap{ nullptr }
	, m_randomVectorMapUploadBuffer{ nullptr }
	, m_normalMap{ nullptr }
	, m_ambientMap0{ nullptr }
	, m_ambientMap1{ nullptr }
{}

bool CSsaoMap::Initialize(CDirectx3D* directx3D, UINT width, UINT height)
{
	ReturnIfFalse(OnResize(directx3D, width, height));
	ReturnIfFalse(BuildRandomVectorTexture(directx3D));
	RebuildDescriptors(directx3D->GetDepthStencilBufferResource());

	return true;
}

ID3D12Resource* CSsaoMap::NormalMap()
{
	return m_normalMap.Get();
}

ID3D12Resource* CSsaoMap::AmbientMap()
{
	return m_ambientMap0.Get();
}

void CSsaoMap::RebuildDescriptors(ID3D12Resource* depthStencilBuffer)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = NormalMapFormat;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	m_descHeap->CreateShaderResourceView(SrvOffset::SsaoNormalMap, 0, &srvDesc, m_normalMap.Get());

	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	m_descHeap->CreateShaderResourceView(SrvOffset::SsaoDepthMap, 0, &srvDesc, depthStencilBuffer);

	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_descHeap->CreateShaderResourceView(SrvOffset::SsaoRandomVectorMap, 0, &srvDesc, m_randomVectorMap.Get());

	srvDesc.Format = AmbientMapFormat;
	m_descHeap->CreateShaderResourceView(SrvOffset::SsaoAmbientMap0, 0, &srvDesc, m_ambientMap0.Get());
	m_descHeap->CreateShaderResourceView(SrvOffset::SsaoAmbientMap1, 0, &srvDesc, m_ambientMap1.Get());

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = NormalMapFormat;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	m_descHeap->CreateRenderTargetView(RtvOffset::NormalMap, &rtvDesc, m_normalMap.Get());

	rtvDesc.Format = AmbientMapFormat;
	m_descHeap->CreateRenderTargetView(RtvOffset::AmbientMap0, &rtvDesc, m_ambientMap0.Get());
	m_descHeap->CreateRenderTargetView(RtvOffset::AmbientMap1, &rtvDesc, m_ambientMap1.Get());
}

void CSsaoMap::SetPSOs(ID3D12PipelineState* ssaoPso, ID3D12PipelineState* ssaoBlurPso)
{
	m_ssaoPso = ssaoPso;
	m_blurPso = ssaoBlurPso;
}

bool CSsaoMap::OnResize(CDirectx3D* directx3D, UINT newWidth, UINT newHeight)
{
	if (m_renderTargetWidth == newWidth && m_renderTargetHeight == newHeight)
		return true;
	
	m_renderTargetWidth = newWidth;
	m_renderTargetHeight = newHeight;

	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_viewport.Width = static_cast<float>(m_renderTargetWidth) / 2.0f;
	m_viewport.Height = static_cast<float>(m_renderTargetHeight) / 2.0f;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_scissorRect = { 0, 0, static_cast<int>(m_renderTargetWidth / 2), static_cast<int>(m_renderTargetHeight / 2) };

	return BuildResources(directx3D);
}

void CSsaoMap::ComputeSsao(
	ID3D12GraphicsCommandList* cmdList,
	CFrameResources* currFrame,
	int blurCount)
{
	cmdList->RSSetViewports(1, &m_viewport);
	cmdList->RSSetScissorRects(1, &m_scissorRect);

	cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_ambientMap0.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET)));

	D3D12_CPU_DESCRIPTOR_HANDLE ambientMap0Rtv = m_descHeap->GetCpuRtvHandle(RtvOffset::AmbientMap0);
	float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	cmdList->ClearRenderTargetView(ambientMap0Rtv, clearValue, 0, nullptr);
	cmdList->OMSetRenderTargets(1, &ambientMap0Rtv, true, nullptr);

	auto ssaoCBAddress = currFrame->GetResource(eBufferType::SsaoCB)->GetGPUVirtualAddress();
	cmdList->SetGraphicsRootConstantBufferView(EtoV(SsaoRegisterType::Pass), ssaoCBAddress);
	cmdList->SetGraphicsRoot32BitConstant(EtoV(SsaoRegisterType::Constants), 0, 0);

	cmdList->SetGraphicsRootDescriptorTable(EtoV(SsaoRegisterType::Normal), m_descHeap->GetGpuSrvHandle(SrvOffset::SsaoNormalMap));
	cmdList->SetGraphicsRootDescriptorTable(EtoV(SsaoRegisterType::Depth), m_descHeap->GetGpuSrvHandle(SrvOffset::SsaoDepthMap));
	cmdList->SetGraphicsRootDescriptorTable(EtoV(SsaoRegisterType::RandomVec), m_descHeap->GetGpuSrvHandle(SrvOffset::SsaoRandomVectorMap));

	cmdList->SetPipelineState(m_ssaoPso);

	cmdList->IASetVertexBuffers(0, 0, nullptr);
	cmdList->IASetIndexBuffer(nullptr);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->DrawInstanced(6, 1, 0, 0);

	cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_ambientMap0.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ)));

	BlurAmbientMap(cmdList, currFrame, blurCount);
}

void CSsaoMap::BlurAmbientMap(ID3D12GraphicsCommandList* cmdList, CFrameResources* currFrame, int blurCount)
{
	cmdList->SetPipelineState(m_blurPso);

	auto ssaoCBAddress = currFrame->GetResource(eBufferType::SsaoCB)->GetGPUVirtualAddress();
	cmdList->SetGraphicsRootConstantBufferView(0, ssaoCBAddress);

	for (auto i : std::views::iota(0, blurCount))
	{
		BlurAmbientMap(cmdList, true);
		BlurAmbientMap(cmdList, false);
	}
}

void CSsaoMap::BlurAmbientMap(ID3D12GraphicsCommandList* cmdList, bool horzBlur)
{
	ID3D12Resource* output = nullptr;
	CD3DX12_GPU_DESCRIPTOR_HANDLE inputSrv{};
	CD3DX12_CPU_DESCRIPTOR_HANDLE outputRtv{};

	if (horzBlur == true)
	{
		output = m_ambientMap1.Get();
		inputSrv = m_descHeap->GetGpuSrvHandle(SrvOffset::SsaoAmbientMap0);
		outputRtv = m_descHeap->GetCpuRtvHandle(RtvOffset::AmbientMap1);
		cmdList->SetGraphicsRoot32BitConstant(EtoV(SsaoRegisterType::Constants), 1, 0);
	}
	else
	{
		output = m_ambientMap0.Get();
		inputSrv = m_descHeap->GetGpuSrvHandle(SrvOffset::SsaoAmbientMap1);
		outputRtv = m_descHeap->GetCpuRtvHandle(RtvOffset::AmbientMap0);
		cmdList->SetGraphicsRoot32BitConstant(EtoV(SsaoRegisterType::Constants), 0, 0);
	}

	cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(output,
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET)));

	float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	cmdList->ClearRenderTargetView(outputRtv, clearValue, 0, nullptr);
	cmdList->OMSetRenderTargets(1, &outputRtv, true, nullptr);

	cmdList->SetGraphicsRootDescriptorTable(EtoV(SsaoRegisterType::Normal), m_descHeap->GetGpuSrvHandle(SrvOffset::SsaoNormalMap));
	cmdList->SetGraphicsRootDescriptorTable(EtoV(SsaoRegisterType::SsaoAmbientMap0), inputSrv);

	cmdList->IASetVertexBuffers(0, 0, nullptr);
	cmdList->IASetIndexBuffer(nullptr);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->DrawInstanced(6, 1, 0, 0);

	cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(output,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ)));
}

bool CSsaoMap::BuildResources(CDirectx3D* directx3D)
{
	return (directx3D->LoadData([this](ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)->bool {
		return CreateResources(device); }));
}

bool CSsaoMap::CreateResources(ID3D12Device* device)
{
	m_normalMap = nullptr;
	m_ambientMap0 = nullptr;
	m_ambientMap1 = nullptr;

	D3D12_RESOURCE_DESC texDesc{};
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = m_renderTargetWidth;
	texDesc.Height = m_renderTargetHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = CSsaoMap::NormalMapFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	float normalClearColor[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	CD3DX12_CLEAR_VALUE optClear(NormalMapFormat, normalClearColor);
	ReturnIfFailed(device->CreateCommittedResource(
		&RvToLv(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&m_normalMap)));

	texDesc.Width = m_renderTargetWidth / 2;
	texDesc.Height = m_renderTargetHeight / 2;
	texDesc.Format = CSsaoMap::AmbientMapFormat;

	float ambientClearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	optClear = CD3DX12_CLEAR_VALUE(AmbientMapFormat, ambientClearColor);

	ReturnIfFailed(device->CreateCommittedResource(
		&RvToLv(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&m_ambientMap0)));

	ReturnIfFailed(device->CreateCommittedResource(
		&RvToLv(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&m_ambientMap1)));

	return true;
}

bool CSsaoMap::BuildRandomVectorTexture(CDirectx3D* directx3D)
{
	return (directx3D->LoadData([this](ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)->bool {
		return CreateRandomVectorTexture(device, cmdList); }));
}

bool CSsaoMap::CreateRandomVectorTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	D3D12_RESOURCE_DESC texDesc{};
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ReturnIfFailed(device->CreateCommittedResource(
		&RvToLv(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_randomVectorMap)));

	const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_randomVectorMap.Get(), 0, num2DSubresources);

	ReturnIfFailed(device->CreateCommittedResource(
		&RvToLv(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
		D3D12_HEAP_FLAG_NONE,
		&RvToLv(CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_randomVectorMapUploadBuffer.GetAddressOf())));

	auto RandFloat = []()->float { return (float)(rand()) / (float)RAND_MAX; };

	std::vector<XMCOLOR> initData{};
	for (auto i : std::views::iota(0, 256))
		for (auto j : std::views::iota(0, 256))
			initData.emplace_back(RandFloat(), RandFloat(), RandFloat(), 0.0f);

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData.data();
	subResourceData.RowPitch = 256 * sizeof(XMCOLOR);
	subResourceData.SlicePitch = subResourceData.RowPitch * 256;

	cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_randomVectorMap.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST)));
	UpdateSubresources(cmdList, m_randomVectorMap.Get(), m_randomVectorMapUploadBuffer.Get(),
		0, 0, num2DSubresources, &subResourceData);
	cmdList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(m_randomVectorMap.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ)));

	return true;
}
