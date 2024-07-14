#pragma once

#include <wrl.h>
#include <vector>
#include <DirectXMath.h>
#include "d3dx12.h"

class CFrameResources;
struct ID3D12RootSignature;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;
struct ID3D12PipelineState;

class Ssao
{
public:
	Ssao(ID3D12Device* device);
	~Ssao();

	Ssao(const Ssao& rhs) = delete;
	Ssao& operator=(const Ssao& rhs) = delete;

	static const DXGI_FORMAT AmbientMapFormat = DXGI_FORMAT_R16_UNORM;
	static const DXGI_FORMAT NormalMapFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

	static const int MaxBlurRadius = 5;

	bool Initialize(ID3D12GraphicsCommandList* cmdList, UINT width, UINT height);
	UINT SsaoMapWidth() const;
	UINT SsaoMapHeight() const;

	void GetOffsetVectors(DirectX::XMFLOAT4 offsets[14]);
	std::vector<float> CalcGaussWeights(float sigma);

	ID3D12Resource* NormalMap();
	ID3D12Resource* AmbientMap();

	CD3DX12_CPU_DESCRIPTOR_HANDLE NormalMapRtv() const;
	CD3DX12_GPU_DESCRIPTOR_HANDLE NormalMapSrv() const;
	CD3DX12_GPU_DESCRIPTOR_HANDLE AmbientMapSrv() const;

	void BuildDescriptors(
		ID3D12Resource* depthStencilBuffer,
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuRtv,
		UINT cbvSrvUavDescriptorSize,
		UINT RtvDescriptorSize);

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
	void BuildOffsetVectors();
	bool BuildRandomVectorTexture(ID3D12GraphicsCommandList* cmdList);

private:
	ID3D12Device* m_device{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_ssaoRootSig;

	ID3D12PipelineState* m_ssaoPso{ nullptr };
	ID3D12PipelineState* m_blurPso{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12Resource> m_randomVectorMap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_randomVectorMapUploadBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_normalMap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_ambientMap0;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_ambientMap1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mhNormalMapCpuSrv{};
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhNormalMapGpuSrv{};
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhNormalMapCpuRtv{};

	CD3DX12_CPU_DESCRIPTOR_HANDLE mhDepthMapCpuSrv{};
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhDepthMapGpuSrv{};

	CD3DX12_CPU_DESCRIPTOR_HANDLE mhRandomVectorMapCpuSrv{};
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhRandomVectorMapGpuSrv{};

	CD3DX12_CPU_DESCRIPTOR_HANDLE mhAmbientMap0CpuSrv{};
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhAmbientMap0GpuSrv{};
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhAmbientMap0CpuRtv{};

	CD3DX12_CPU_DESCRIPTOR_HANDLE mhAmbientMap1CpuSrv{};
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhAmbientMap1GpuSrv{};
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhAmbientMap1CpuRtv{};

	UINT m_renderTargetWidth{ 0 };
	UINT m_renderTargetHeight{ 0 };

	DirectX::XMFLOAT4 m_offsets[14]{};

	D3D12_VIEWPORT m_viewport{};
	D3D12_RECT m_scissorRect{};
};