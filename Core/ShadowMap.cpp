#include "./ShadowMap.h"
#include "../Include/types.h"
#include "../Include/RendererDefine.h"
#include "./d3dUtil.h"
#include "./Renderer.h"
#include "./DescriptorHeap.h"
#include "./Directx3D.h"
#include "./CoreDefine.h"

CShadowMap::~CShadowMap() = default;
CShadowMap::CShadowMap(CRenderer* renderer, CDescriptorHeap* descHeap)
	: m_renderer{ renderer }
	, m_descHeap{ descHeap }
	, m_shadowMap{ nullptr }
{
	m_mapWidth = gShadowMapWidth;
	m_mapHeight = gShadowMapHeight;

	m_viewport = { 0.0f, 0.0f, (float)m_mapWidth, (float)m_mapHeight, 0.0f, 1.0f };
	m_scissorRect = { 0, 0, (int)m_mapWidth, (int)m_mapHeight };
}

UINT CShadowMap::Width() const		{	return m_mapWidth;	}
UINT CShadowMap::Height() const		{	return m_mapHeight;		}
ID3D12Resource* CShadowMap::Resource()		{	return m_shadowMap.Get();	}

D3D12_VIEWPORT CShadowMap::Viewport() const	{	return m_viewport;	}
D3D12_RECT CShadowMap::ScissorRect() const		{	return m_scissorRect;	}

bool CShadowMap::BuildResource()
{
	return (m_renderer->LoadData([this](ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)->bool {
		return CreateResource(); }));
}

bool CShadowMap::Initialize()
{
	ReturnIfFalse(BuildResource());
	BuildDescriptors();

	return true;
}

bool CShadowMap::OnResize(UINT newWidth, UINT newHeight)
{
	if ((m_mapWidth == newWidth) && (m_mapHeight == newHeight))
		return true;
	
	m_mapWidth = newWidth;
	m_mapHeight = newHeight;

	ReturnIfFalse(BuildResource());
	BuildDescriptors();

	return true;
}

void CShadowMap::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	m_descHeap->CreateShaderResourceView(eTextureType::ShadowMap, 0, srvDesc, m_shadowMap.Get());

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	auto directx3D = m_renderer->GetDirectx3D();
	directx3D->CreateDepthStencilView(DsvOffset::ShadowMap, m_shadowMap.Get(), &dsvDesc);
}

bool CShadowMap::CreateResource()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = m_mapWidth;
	texDesc.Height = m_mapHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = mFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	ID3D12Device* device = m_renderer->GetDevice();
	ReturnIfFailed(device->CreateCommittedResource(
		&RvToLv(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&m_shadowMap)));

	return true;
}