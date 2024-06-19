#pragma once

#include <memory>
#include <wrl.h>
#include <array>
#include <vector>
#include <d3d12.h>

class CDirectx3D;
class CShader;
class CCamera;
class CGameTimer;
class UploadBuffer;
class CFrameResources;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;
struct D3D12_GRAPHICS_PIPELINE_STATE_DESC;
struct ID3D12PipelineState;
struct RenderItem;

class CRenderer
{
	enum class GraphicsPSO : int
	{
		Opaque,
		Count
	};

	static constexpr std::array<GraphicsPSO, static_cast<size_t>(GraphicsPSO::Count)> GraphicsPSO_ALL
	{
		GraphicsPSO::Opaque,
	};

public:
	CRenderer(CDirectx3D* directx3D);

	CRenderer() = delete;
	CRenderer(const CRenderer&) = delete;
	CRenderer& operator=(const CRenderer&) = delete;

	bool Initialize();
	void OnResize(int wndWidth, int wndHeight);
	void Draw(CGameTimer* gt, CFrameResources* pCurrFrameRes,
		const std::vector<std::unique_ptr<RenderItem>>& renderItem);

	inline ID3D12Device* GetDevice() const;
	inline ID3D12GraphicsCommandList* GetCommandList() const;
	inline ID3D12DescriptorHeap* GetSrvDescriptorHeap() const;

private:
	bool BuildRootSignature();
	bool BuildDescriptorHeaps();
	bool BuildPSOs();
	bool MakePSOPipelineState(GraphicsPSO psoType);
	bool MakeOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);
	void DrawRenderItems(UploadBuffer* instanceBuffer, 
		const std::vector<std::unique_ptr<RenderItem>>& ritems);

private:
	CDirectx3D* m_directx3D{ nullptr };
	std::unique_ptr<CShader> m_shader{ nullptr };

	ID3D12Device* m_device{ nullptr };
	ID3D12GraphicsCommandList* m_cmdList{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvDescHeap{ nullptr };

	std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<size_t>(GraphicsPSO::Count)> m_psoList{};

	D3D12_VIEWPORT m_screenViewport{};
	D3D12_RECT m_scissorRect{};
};

inline ID3D12Device* CRenderer::GetDevice() const											{	return m_device;		}
inline ID3D12GraphicsCommandList* CRenderer::GetCommandList() const		{	return m_cmdList;	}
inline ID3D12DescriptorHeap* CRenderer::GetSrvDescriptorHeap() const		{	return m_srvDescHeap.Get();		}