#pragma once

#include <wrl.h>
#include "d3dx12.h"

class CRenderer;
class CDescriptorHeap;
struct ID3D12Device;
struct ID3D12Resource;

class CShadowMap
{
public:
	CShadowMap(CRenderer* renderer, CDescriptorHeap* descHeap);
	~CShadowMap();

	CShadowMap() = delete;
	CShadowMap(const CShadowMap& rhs) = delete;
	CShadowMap& operator=(const CShadowMap& rhs) = delete;

	UINT Width() const;
	UINT Height() const;
	ID3D12Resource* Resource();

	D3D12_VIEWPORT Viewport() const;
	D3D12_RECT ScissorRect() const;

	bool Initialize();
	bool OnResize(UINT newWidth, UINT newHeight);

private:
	void BuildDescriptors();
	bool BuildResource();
	bool CreateResource();

private:
	CRenderer* m_renderer;
	CDescriptorHeap* m_descHeap;

	D3D12_VIEWPORT m_viewport{};
	D3D12_RECT m_scissorRect{};

	UINT m_mapWidth{ 0 };
	UINT m_mapHeight{ 0 };
	DXGI_FORMAT mFormat = DXGI_FORMAT_R24G8_TYPELESS;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_shadowMap;
};