#pragma once

enum class RootSignature : int
{
	Common = 0,
	Ssao,
};

class CRootSignature
{
public:
	CRootSignature();
	~CRootSignature();

	CRootSignature(const CRootSignature&) = delete;
	CRootSignature& operator=(const CRootSignature&) = delete;

	bool Build(ID3D12Device* device);
	ID3D12RootSignature* Get(RootSignature sigType);

private:
	bool BuildMain(ID3D12Device* device);
	bool BuildSsao(ID3D12Device* device);
	bool Create(ID3D12Device* device, RootSignature type,
		const std::vector<CD3DX12_ROOT_PARAMETER>& rootParamList,
		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers);

private:
	std::array<Microsoft::WRL::ComPtr<ID3D12RootSignature>, 2> m_rootSignatures;
};