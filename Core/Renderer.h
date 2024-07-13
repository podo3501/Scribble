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
class CShadowMap;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;
struct D3D12_GRAPHICS_PIPELINE_STATE_DESC;
struct ID3D12PipelineState;

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
	virtual bool LoadTexture(const TextureList& textureList) override;
	virtual bool LoadTexture(const TextureList& textureList, std::vector<std::wstring>* srvFilename) override;
	virtual bool SetUploadBuffer(eBufferType bufferType, const void* bufferData, size_t dataSize) override;
	virtual bool PrepareFrame() override;
	virtual bool Draw(AllRenderItems& renderItem) override;
	virtual void Set4xMsaaState(HWND hwnd, int widht, int height, bool value) override;

	bool Initialize(const std::wstring& resPath, HWND hwnd, int width, int height, const ShaderFileList& shaderFileList);
	bool WaitUntilGpuFinished(UINT64 fenceCount);
	bool LoadData(std::function<bool(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)> loadGraphicMemory);

	inline ID3D12Device* GetDevice() const;
	inline ID3D12DescriptorHeap* GetSrvDescriptorHeap() const;
	inline CDirectx3D* GetDirectx3D() const;

	void CreateShaderResourceView(eTextureType type, const std::wstring& filename,
		ID3D12Resource* pRes, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc);

private:
	bool BuildRootSignature();
	bool BuildDescriptorHeaps();
	bool BuildPSOs();

	UINT GetSrvIndex(eTextureType type);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuDescripterHandle(eTextureType type);
	D3D12_GPU_VIRTUAL_ADDRESS GetFrameResourceAddress(eBufferType bufType);

	bool LoadMesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
		Vertices& totalVertices, Indices& totalIndices, RenderItem* renderItem);

	bool MakeFrameResource();
	bool MakePSOPipelineState(GraphicsPSO psoType);
	void MakeBasicDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);
	void MakeSkyDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);
	void MakeOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);
	void MakeNormalOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);
	void MakeShadowDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);

	void DrawSceneToShadowMap(AllRenderItems& renderItem);
	void DrawRenderItems(ID3D12Resource* instanceRes, RenderItem* renderItem);

private:
	std::unique_ptr<CDirectx3D> m_directx3D;
	std::unique_ptr<CShader> m_shader;
	std::unique_ptr<CTexture> m_texture;
	std::unique_ptr<CShadowMap> m_shadowMap;

	bool m_isInitialize{ false };
	ID3D12Device* m_device{ nullptr };
	ID3D12GraphicsCommandList* m_cmdList{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvDescHeap;
	UINT m_srvOffsetTexture2D{ 0u };
	std::vector<std::wstring> m_srvTexture2DFilename{};

	std::unique_ptr<CFrameResources> m_frameResources;
	std::map<GraphicsPSO, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_psoList;

	D3D12_VIEWPORT m_screenViewport{};
	D3D12_RECT m_scissorRect{};
};

inline ID3D12Device* CRenderer::GetDevice() const											{	return m_device;		}
inline ID3D12DescriptorHeap* CRenderer::GetSrvDescriptorHeap() const		{	return m_srvDescHeap.Get();		}
inline CDirectx3D* CRenderer::GetDirectx3D() const											{ return m_directx3D.get(); }

