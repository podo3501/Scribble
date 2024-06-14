#include "Renderer.h"
#include "../Core/Directx3D.h"
#include "../Core/d3dx12.h"
#include "../Core/d3dUtil.h"

using Microsoft::WRL::ComPtr;

//7인 이유는 텍스춰를 7장을 다 올린다음 동적으로 선택하기 위함이다.  Texture2D gDiffuseMap[7] : register(t0)
constexpr UINT DescriptorHeapSize{ 7 };

CRenderer::CRenderer(CDirectx3D* directx3D)
	: m_directx3D(directx3D)
	, m_device(directx3D->GetDevice())
	, m_cmdList(directx3D->GetCommandList())
{}

bool CRenderer::Initialize()
{
	m_directx3D->ResetCommandLists();

	BuildRootSignature();
	BuildDescriptorHeaps();

	m_directx3D->ExcuteCommandLists();
	m_directx3D->FlushCommandQueue();
	return true;
}

void CRenderer::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, DescriptorHeapSize, 0, 0);

	CD3DX12_ROOT_PARAMETER slotRootParameter[4];
	slotRootParameter[0].InitAsShaderResourceView(0, 1);
	slotRootParameter[1].InitAsShaderResourceView(1, 1);
	slotRootParameter[2].InitAsConstantBufferView(0);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = d3dUtil::GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(slotRootParameter), slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serialized = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serialized.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)));
}

void CRenderer::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = DescriptorHeapSize;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_srvDescHeap)));
}
