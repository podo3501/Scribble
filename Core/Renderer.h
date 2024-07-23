#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <functional>
#include <memory>
#include <array>
#include <set>
#include <vector>
#include <string>
#include "../Include/Interface.h"

class CDirectx3D;
class CShader;
class CTexture;
class CSsaoMap;
class CDraw;
class CPipelineStateObjects;
struct CD3DX12_ROOT_PARAMETER;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;

enum class RootSignature : int
{
	Common = 0,
	Ssao,
};

class CRenderer : public IRenderer
{
public:
	CRenderer();
	~CRenderer();

	CRenderer(const CRenderer&) = delete;
	CRenderer& operator=(const CRenderer&) = delete;

	virtual bool IsInitialize() override { return m_isInitialize; };
	virtual bool OnResize(int width, int height) override;
	virtual bool LoadMesh(GraphicsPSO pso, Vertices& totalVertices, Indices& totalIndices, RenderItem* renderItem) override;
	virtual bool LoadSkinnedMesh(const SkinnedVertices& totalVertices, const Indices& totalIndices, RenderItem* renderItem) override;
	virtual bool LoadTexture(const TextureList& textureList, std::vector<std::wstring>* srvFilename) override;
	virtual bool SetUploadBuffer(eBufferType bufferType, const void* bufferData, size_t dataSize) override;
	virtual bool PrepareFrame() override;
	virtual bool Draw(AllRenderItems& renderItem) override;
	virtual void Set4xMsaaState(HWND hwnd, int widht, int height, bool value) override;

	bool Initialize(const std::wstring& resPath, HWND hwnd, int width, int height, const ShaderFileList& shaderFileList);
	bool WaitUntilGpuFinished(UINT64 fenceCount);
	bool LoadData(std::function<bool(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)> loadGraphicMemory);

	ID3D12RootSignature* GetRootSignature(RootSignature sigType);
	inline ID3D12Device* GetDevice() const;
	inline ID3D12DescriptorHeap* GetSrvDescriptorHeap() const;
	inline CDirectx3D* GetDirectx3D() const;

	void CreateShaderResourceView(eTextureType type, const std::wstring& filename,
		ID3D12Resource* pRes, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc);

private:
	bool CreateRootSignature(
		RootSignature type,
		const std::vector<CD3DX12_ROOT_PARAMETER>& rootParamList,
		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers);
	bool BuildRootSignature();
	bool BuildMainRootSignature();
	bool BuildSsaoRootSignature();
	bool BuildDescriptorHeaps();

	UINT GetSrvIndex(eTextureType type);

	bool LoadMesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
		Vertices& totalVertices, Indices& totalIndices, RenderItem* renderItem);
	bool LoadSkinnedMesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
		const SkinnedVertices& totalVertices, const Indices& totalIndices, RenderItem* renderItem);

	bool MakeFrameResource();

private:
	std::unique_ptr<CDirectx3D> m_directx3D;
	std::unique_ptr<CShader> m_shader;
	std::unique_ptr<CTexture> m_texture;
	std::unique_ptr<CDraw> m_draw;
	std::unique_ptr<CSsaoMap> m_ssaoMap;

	bool m_isInitialize{ false };
	ID3D12Device* m_device{ nullptr };
	ID3D12GraphicsCommandList* m_cmdList{ nullptr };

	std::array< Microsoft::WRL::ComPtr<ID3D12RootSignature>, 2> m_rootSignatures;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvDescHeap;
	UINT m_srvOffsetTexture2D{ 0u };
	std::vector<std::wstring> m_srvTexture2DFilename{};

	std::unique_ptr<CFrameResources> m_frameResources;
	std::unique_ptr<CPipelineStateObjects> m_pso;
};

inline ID3D12Device* CRenderer::GetDevice() const												{ return m_device; }
inline ID3D12DescriptorHeap* CRenderer::GetSrvDescriptorHeap() const			{ return m_srvDescHeap.Get(); }
inline CDirectx3D* CRenderer::GetDirectx3D() const												{ return m_directx3D.get(); }

