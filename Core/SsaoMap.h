#pragma once

#include <wrl.h>
#include <vector>
#include <DirectXMath.h>
#include "d3dx12.h"

class CRenderer;
class CFrameResources;
struct ID3D12RootSignature;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;
struct ID3D12PipelineState;

enum class SsaoRegisterType : int
{
	Pass = 0,
	Constants,
	Normal,
	Depth,
	RandomVec,
	SsaoAmbientMap0 = RandomVec,
};

class CSsaoMap
{
public:
	CSsaoMap(CRenderer* renderer);
	~CSsaoMap();

	CSsaoMap(const CSsaoMap& rhs) = delete;
	CSsaoMap& operator=(const CSsaoMap& rhs) = delete;

	static const DXGI_FORMAT AmbientMapFormat = DXGI_FORMAT_R16_UNORM;
	static const DXGI_FORMAT NormalMapFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

	bool Initialize(ID3D12Resource* depthStencilBuffer, UINT width, UINT height);
	UINT SsaoMapWidth() const;
	UINT SsaoMapHeight() const;

	ID3D12Resource* NormalMap();
	ID3D12Resource* AmbientMap();

	void RebuildDescriptors(ID3D12Resource* depthStencilBuffer);
	void SetPSOs(ID3D12PipelineState* ssaoPso, ID3D12PipelineState* ssaoBlurPso);
	bool OnResize(UINT newWidth, UINT newHeight);
	void ComputeSsao(
		ID3D12GraphicsCommandList* cmdList,
		CFrameResources* currFrame,
		int blurCount);

private:
	void BlurAmbientMap(ID3D12GraphicsCommandList* cmdList, CFrameResources* currFrame, int blurCount);
	void BlurAmbientMap(ID3D12GraphicsCommandList* cmdList, bool horzBlur);

	bool BuildResources();
	bool BuildRandomVectorTexture();
	bool CreateResources(ID3D12Device* device);
	bool CreateRandomVectorTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);

private:
	CRenderer* m_renderer{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_ssaoRootSig;

	ID3D12PipelineState* m_ssaoPso{ nullptr };
	ID3D12PipelineState* m_blurPso{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12Resource> m_randomVectorMap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_randomVectorMapUploadBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_normalMap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_ambientMap0;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_ambientMap1;

	UINT m_renderTargetWidth{ 0 };
	UINT m_renderTargetHeight{ 0 };

	D3D12_VIEWPORT m_viewport{};
	D3D12_RECT m_scissorRect{};
};