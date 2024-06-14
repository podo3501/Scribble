#pragma once

#include<memory>
#include <wrl.h>
#include<array>

class CDirectx3D;
class CShader;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;
struct D3D12_GRAPHICS_PIPELINE_STATE_DESC;
struct ID3D12PipelineState;

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
	bool Initialize();

	inline ID3D12Device* GetDevice() const;
	inline ID3D12GraphicsCommandList* GetCommandList() const;
	inline ID3D12DescriptorHeap* GetSrvDescriptorHeap() const;

	CRenderer() = delete;
	CRenderer(const CRenderer&) = delete;
	CRenderer& operator=(const CRenderer&) = delete;

private:
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildPSOs();
	void MakePSOPipelineState(GraphicsPSO psoType);
	void MakeOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);

private:
	CDirectx3D* m_directx3D{ nullptr };
	std::unique_ptr<CShader> m_shader{ nullptr };

	ID3D12Device* m_device{ nullptr };
	ID3D12GraphicsCommandList* m_cmdList{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvDescHeap{ nullptr };

	std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<size_t>(GraphicsPSO::Count)> m_psoList{};
};

inline ID3D12Device* CRenderer::GetDevice() const											{	return m_device;		}
inline ID3D12GraphicsCommandList* CRenderer::GetCommandList() const		{	return m_cmdList;	}
inline ID3D12DescriptorHeap* CRenderer::GetSrvDescriptorHeap() const		{	return m_srvDescHeap.Get();		}