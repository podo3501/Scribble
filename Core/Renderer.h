#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <functional>
#include <memory>
#include <array>
#include "../Include/Interface.h"

class CDirectx3D;
class CRootSignature;
class CShader;
class CTexture;
class CSsaoMap;
class CDraw;
class CPipelineStateObjects;
class CDescriptorHeap;

class CRenderer : public IRenderer
{
public:
	CRenderer();
	~CRenderer();

	CRenderer(const CRenderer&) = delete;
	CRenderer& operator=(const CRenderer&) = delete;

	virtual bool IsInitialize() override { return m_isInitialize; };
	virtual bool OnResize(int width, int height) override;
	virtual bool LoadMesh(GraphicsPSO pso, const void* verticesData, const void* indicesData, RenderItem* renderItem) override;
	virtual bool LoadTexture(const TextureList& textureList, std::vector<std::wstring>* srvFilename) override;
	virtual bool SetUploadBuffer(eBufferType bufferType, const void* bufferData, size_t dataSize) override;
	virtual bool PrepareFrame() override;
	virtual bool Draw(AllRenderItems& renderItem) override;
	virtual void Set4xMsaaState(HWND hwnd, int widht, int height, bool value) override;

	bool Initialize(const std::wstring& resPath, HWND hwnd, int width, int height, const ShaderFileList& shaderFileList);
	bool WaitUntilGpuFinished(UINT64 fenceCount);

private:
	bool LoadMesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
		const void* verticesData, const void* indicesData, RenderItem* renderItem);

private:
	std::unique_ptr<CDirectx3D> m_directx3D;
	std::unique_ptr<CRootSignature> m_rootSignature;
	std::unique_ptr<CShader> m_shader;
	std::unique_ptr<CTexture> m_texture;
	std::unique_ptr<CDraw> m_draw;
	std::unique_ptr<CSsaoMap> m_ssaoMap;
	std::unique_ptr<CDescriptorHeap> m_descHeap;
	std::unique_ptr<CFrameResources> m_frameResources;
	std::unique_ptr<CPipelineStateObjects> m_pso;

	bool m_isInitialize{ false };
};

