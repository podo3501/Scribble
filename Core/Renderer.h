#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <functional>
#include <memory>
#include <array>
#include <vector>
#include <string>
#include "../Include/Interface.h"

class CDirectx3D;
class CShader;
class CCamera;
class CUploadBuffer;
class CFrameResources;
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
	CRenderer(std::wstring resPath);
	~CRenderer();

	CRenderer() = delete;
	CRenderer(const CRenderer&) = delete;
	CRenderer& operator=(const CRenderer&) = delete;

	bool Initialize(CWindow* window);
	virtual bool OnResize(int wndWidth, int wndHeight) override;
	virtual bool LoadData(std::function<bool(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)> loadGraphicMemory) override;
	virtual bool SetUploadBuffer(eBufferType bufferType, const void* bufferData, size_t dataSize) override;
	virtual bool PrepareFrame() override;
	virtual bool Draw(AllRenderItems& renderItem) override;
	virtual bool WaitUntilGpuFinished(UINT64 fenceCount) override;

	virtual inline ID3D12Device* GetDevice() const override;
	virtual inline ID3D12DescriptorHeap* GetSrvDescriptorHeap() const override;

private:
	bool BuildRootSignature();
	bool BuildDescriptorHeaps();
	bool BuildPSOs();
	bool MakeFrameResource();
	bool MakePSOPipelineState(GraphicsPSO psoType);
	void MakeBasicDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);
	void MakeSkyDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);
	void MakeOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);
	void DrawRenderItems(ID3D12Resource* instanceRes, RenderItem* renderItem);

private:
	std::wstring m_resPath{};

	std::unique_ptr<CDirectx3D> m_directx3D{ nullptr };
	std::unique_ptr<CShader> m_shader{ nullptr };

	ID3D12Device* m_device{ nullptr };
	ID3D12GraphicsCommandList* m_cmdList{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvDescHeap{ nullptr };

	std::unique_ptr<CFrameResources> m_frameResources{ nullptr };
	std::vector<Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_psoList{};

	D3D12_VIEWPORT m_screenViewport{};
	D3D12_RECT m_scissorRect{};
};

inline ID3D12Device* CRenderer::GetDevice() const											{	return m_device;		}
inline ID3D12DescriptorHeap* CRenderer::GetSrvDescriptorHeap() const		{	return m_srvDescHeap.Get();		}