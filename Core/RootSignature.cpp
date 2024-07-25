#include "./RootSignature.h"
#include "./CoreDefine.h"
#include "./SsaoMap.h"
#include "./d3dUtil.h"
#include "./headerUtility.h"

using Microsoft::WRL::ComPtr;

CRootSignature::~CRootSignature() = default;
CRootSignature::CRootSignature()
{}

bool CRootSignature::Build(ID3D12Device* device)
{
	ReturnIfFalse(BuildMain(device));
	ReturnIfFalse(BuildSsao(device));

	return true;
}

template<typename T>
CD3DX12_ROOT_PARAMETER* GetRootParameter(
	std::vector<CD3DX12_ROOT_PARAMETER>& rootParameter, T type)
{
	rootParameter.resize(rootParameter.size() + 1);
	return &rootParameter[EtoV(type)];
};

bool CRootSignature::BuildMain(ID3D12Device* device)
{
	using enum MainRegisterType;

	CD3DX12_DESCRIPTOR_RANGE shadowTexTable{}, ssaoTexTable{}, cubeTexTable{}, texTable{};

	shadowTexTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, ShadowCount, 0, 0); //t0
	ssaoTexTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SsaoAmbientMap0Count, 1, 0); //t1
	cubeTexTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, CubeCount, 2, 0);	//t2
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, TextureCount, 3, 0);	//t3...t10(세번째인자)

	std::vector<CD3DX12_ROOT_PARAMETER> rp{};
	GetRootParameter(rp, Pass)->InitAsConstantBufferView(0);
	GetRootParameter(rp, Bone)->InitAsConstantBufferView(1);
	GetRootParameter(rp, Material)->InitAsShaderResourceView(0, 1);
	GetRootParameter(rp, Instance)->InitAsShaderResourceView(1, 1);
	GetRootParameter(rp, Shadow)->InitAsDescriptorTable(1, &shadowTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
	GetRootParameter(rp, Ssao)->InitAsDescriptorTable(1, &ssaoTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
	GetRootParameter(rp, Cube)->InitAsDescriptorTable(1, &cubeTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
	GetRootParameter(rp, Diffuse)->InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	return Create(device, RootSignature::Common, rp, CoreUtil::GetStaticSamplers());
}

bool CRootSignature::BuildSsao(ID3D12Device* device)
{
	using enum SsaoRegisterType;

	CD3DX12_DESCRIPTOR_RANGE normalTexTable{}, depthTexTable{}, randomVecTable{};

	normalTexTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SsaoNormalMapCount, 0, 0); //t0
	depthTexTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SsaoDepthMapCount, 1, 0); //t1
	randomVecTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SsaoRandomVectorCount, 2, 0);	//t2

	std::vector<CD3DX12_ROOT_PARAMETER> rp{};
	GetRootParameter(rp, Pass)->InitAsConstantBufferView(0);
	GetRootParameter(rp, Constants)->InitAsConstants(1, 1);
	GetRootParameter(rp, Normal)->InitAsDescriptorTable(1, &normalTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
	GetRootParameter(rp, Depth)->InitAsDescriptorTable(1, &depthTexTable, D3D12_SHADER_VISIBILITY_PIXEL);
	GetRootParameter(rp, RandomVec)->InitAsDescriptorTable(1, &randomVecTable, D3D12_SHADER_VISIBILITY_PIXEL);

	return Create(device, RootSignature::Ssao, rp, CoreUtil::GetSsaoSamplers());
}

bool CRootSignature::Create(ID3D12Device* device, RootSignature type,
	const std::vector<CD3DX12_ROOT_PARAMETER>& rootParamList,
	std::vector<D3D12_STATIC_SAMPLER_DESC> samplers)
{
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
		static_cast<UINT>(rootParamList.size()), rootParamList.data(),
		static_cast<UINT>(samplers.size()), samplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serialized = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serialized.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ReturnIfFailed(hr);

	ReturnIfFailed(device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignatures[EtoV(type)])));

	return true;
}

ID3D12RootSignature* CRootSignature::Get(RootSignature sigType)
{ 
	return m_rootSignatures[EtoV(sigType)].Get(); 
}