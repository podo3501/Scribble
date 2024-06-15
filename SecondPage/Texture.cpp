#include "Texture.h"
#include "../Core/DDSTextureLoader.h"
#include "../Core/d3dUtil.h"
#include "../SecondPage/Renderer.h"

using namespace DirectX;

void CTexture::Load(CRenderer* renderer)
{
	std::vector<std::wstring> filenames = { L"bricks.dds", L"stone.dds", L"tile.dds", L"WoodCrate01.dds",
		L"ice.dds", L"grass.dds", L"white1x1.dds" };
	
	ID3D12Device* device = renderer->GetDevice();
	ID3D12GraphicsCommandList* cmdList = renderer->GetCommandList();

	for_each(filenames.begin(), filenames.end(), [&](auto& curFilename) {
		auto tex = std::make_unique<Texture>();
		tex->Filename = m_filePath + curFilename;
		ThrowIfFailed(CreateDDSTextureFromFile12(device, cmdList,
			tex->Filename.c_str(), tex->Resource, tex->UploadHeap));
		m_textureList.emplace_back(std::move(tex));
		});

	CreateShaderResourceView(device, renderer->GetSrvDescriptorHeap());
}

void CTexture::CreateShaderResourceView(ID3D12Device* device, ID3D12DescriptorHeap* srvDescHeap)
{
	UINT cbvSrvUavDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for_each(m_textureList.begin(), m_textureList.end(), [&, index{ 0 }](auto& curTex) mutable {
		auto& curTexRes = curTex->Resource;
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = curTexRes->GetDesc().Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = curTexRes->GetDesc().MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDesc{ srvDescHeap->GetCPUDescriptorHandleForHeapStart() };
		hCpuDesc.Offset(index++, cbvSrvUavDescSize);
		device->CreateShaderResourceView(curTex->Resource.Get(), &srvDesc, hCpuDesc);
		});
}