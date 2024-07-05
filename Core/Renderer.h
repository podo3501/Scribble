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
class CCamera;
class CUploadBuffer;
class CTexture;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;
struct D3D12_GRAPHICS_PIPELINE_STATE_DESC;
struct ID3D12PipelineState;
struct ID3D12Resource;
enum class GraphicsPSO;
enum class eBufferType;

class CRenderer : public IRenderer
{
public:
	CRenderer();
	~CRenderer();

	CRenderer(const CRenderer&) = delete;
	CRenderer& operator=(const CRenderer&) = delete;

	virtual bool IsInitialize() override { return m_isInitialize; };
	virtual bool OnResize(int width, int height) override;
	virtual bool LoadModel(Vertices& totalVertices, Indices& totalIndices, RenderItem* renderItem) override;
	virtual bool LoadTexture(eTextureType type, std::vector<std::wstring>& filenames) override;
	virtual bool LoadTexture(eTextureType type, std::set<std::wstring>& filenames) override;
	virtual bool SetUploadBuffer(eBufferType bufferType, const void* bufferData, size_t dataSize) override;
	virtual bool PrepareFrame() override;
	virtual bool Draw(AllRenderItems& renderItem) override;
	virtual void Set4xMsaaState(HWND hwnd, int widht, int height, bool value) override;

	bool Initialize(const std::wstring& resPath, HWND hwnd, int width, int height);
	bool WaitUntilGpuFinished(UINT64 fenceCount);

	inline ID3D12Device* GetDevice() const;
	inline ID3D12DescriptorHeap* GetSrvDescriptorHeap() const;

private:
	bool BuildRootSignature();
	bool BuildDescriptorHeaps();
	bool BuildPSOs();

	bool LoadData(std::function<bool(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)> loadGraphicMemory);
	bool LoadModel(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
		Vertices& totalVertices, Indices& totalIndices, RenderItem* renderItem);

	bool MakeFrameResource();
	bool MakePSOPipelineState(GraphicsPSO psoType);
	void MakeBasicDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);
	void MakeSkyDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);
	void MakeOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);

	void DrawRenderItems(ID3D12Resource* instanceRes, RenderItem* renderItem);

private:
	std::unique_ptr<CDirectx3D> m_directx3D{ nullptr };
	std::unique_ptr<CShader> m_shader{ nullptr };

	bool m_isInitialize{ false };
	ID3D12Device* m_device{ nullptr };
	ID3D12GraphicsCommandList* m_cmdList{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvDescHeap{ nullptr };

	std::unique_ptr<CFrameResources> m_frameResources{ nullptr };
	std::vector<Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_psoList{};

	std::unique_ptr<CTexture> m_texture{ nullptr };

	D3D12_VIEWPORT m_screenViewport{};
	D3D12_RECT m_scissorRect{};
};

inline ID3D12Device* CRenderer::GetDevice() const											{	return m_device;		}
inline ID3D12DescriptorHeap* CRenderer::GetSrvDescriptorHeap() const		{	return m_srvDescHeap.Get();		}