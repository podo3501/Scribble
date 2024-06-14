#pragma once

#include<memory>
#include <wrl.h>

class CDirectx3D;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;

class CRenderer
{
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

private:
	ID3D12Device* m_device{ nullptr };
	ID3D12GraphicsCommandList* m_cmdList{ nullptr };

	CDirectx3D* m_directx3D{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvDescHeap{ nullptr };
};

inline ID3D12Device* CRenderer::GetDevice() const
{
	return m_device;
}

inline ID3D12GraphicsCommandList* CRenderer::GetCommandList() const
{
	return m_cmdList;
}

inline ID3D12DescriptorHeap* CRenderer::GetSrvDescriptorHeap() const
{
	return m_srvDescHeap.Get();
}