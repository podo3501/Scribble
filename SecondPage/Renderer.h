#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <functional>
#include <memory>
#include <array>
#include <vector>
#include <string>

class CDirectx3D;
class CShader;
class CCamera;
class CGameTimer;
class CUploadBuffer;
class CFrameResources;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;
struct D3D12_GRAPHICS_PIPELINE_STATE_DESC;
struct ID3D12PipelineState;
struct RenderItem;
enum class GraphicsPSO;

class CRenderer
{
public:
	template<typename T>
	CRenderer(T&& resPath)
		: m_resPath(std::forward<T>(resPath))
	{}

	CRenderer() = delete;
	CRenderer(const CRenderer&) = delete;
	CRenderer& operator=(const CRenderer&) = delete;

	bool Initialize(CDirectx3D* directx3D);
	bool OnResize(int wndWidth, int wndHeight);
	bool Draw(CGameTimer* gt, CFrameResources* pCurrFrameRes,
		const std::vector<std::unique_ptr<RenderItem>>& renderItem);

	inline CDirectx3D* GetDirectx3D() const;
	inline ID3D12Device* GetDevice() const;
	inline ID3D12GraphicsCommandList* GetCommandList() const;
	inline ID3D12DescriptorHeap* GetSrvDescriptorHeap() const;

private:
	bool BuildRootSignature();
	bool BuildDescriptorHeaps();
	bool BuildPSOs();
	bool MakePSOPipelineState(GraphicsPSO psoType);
	void MakeOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);
	void DrawRenderItems(CUploadBuffer* instanceBuffer,
		const std::vector<std::unique_ptr<RenderItem>>& ritems);

private:
	std::wstring m_resPath{};

	CDirectx3D* m_directx3D{ nullptr };
	std::unique_ptr<CShader> m_shader{ nullptr };

	ID3D12Device* m_device{ nullptr };
	ID3D12GraphicsCommandList* m_cmdList{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvDescHeap{ nullptr };

	std::vector<Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_psoList{};

	D3D12_VIEWPORT m_screenViewport{};
	D3D12_RECT m_scissorRect{};
};

inline CDirectx3D* CRenderer::GetDirectx3D() const											{ return m_directx3D; }
inline ID3D12Device* CRenderer::GetDevice() const											{	return m_device;		}
inline ID3D12GraphicsCommandList* CRenderer::GetCommandList() const		{	return m_cmdList;	}
inline ID3D12DescriptorHeap* CRenderer::GetSrvDescriptorHeap() const		{	return m_srvDescHeap.Get();		}