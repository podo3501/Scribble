#include "Texture.h"
#include "../Core/DDSTextureLoader.h"
#include "../Core/d3dUtil.h"
#include "../Core/Directx3D.h"
#include "../SecondPage/Renderer.h"

using namespace DirectX;

enum class eType : int
{
	Common = 0,
	Cube,
};

bool CTexture::Upload(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, eType type, std::vector<std::wstring>& filenames)
{
	auto result = std::all_of(filenames.begin(), filenames.end(),
		[texture = this, device, cmdList, type](auto& curFilename) {
			auto texMemory = std::make_unique<TextureMemory>();
			texMemory->filename = texture->m_resPath + texture->m_filePath + curFilename;
			ReturnIfFailed(CreateDDSTextureFromFile12(device, cmdList,
				texMemory->filename.c_str(), texMemory->resource, texMemory->uploadHeap));
			texture->m_texMemories[type].emplace_back(std::move(texMemory));
			return true; });

	if (!result) return result;

	return true;
}

void CTexture::CreateShaderResourceView(eType type)
{
	auto device = m_renderer->GetDevice();
	UINT cbvSrvUavDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto srvDescHeap = m_renderer->GetSrvDescriptorHeap();
	for_each(m_texMemories[type].begin(), m_texMemories[type].end(),
		[texture = this, srvDescHeap, cbvSrvUavDescSize, device, type](auto& curTex) mutable {
			auto& curTexRes = curTex->resource;
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = curTexRes->GetDesc().Format;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MipLevels = curTexRes->GetDesc().MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

			if (type == eType::Cube)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srvDesc.TextureCube.MostDetailedMip = 0;
				srvDesc.TextureCube.MipLevels = curTexRes->GetDesc().MipLevels;
				srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
				srvDesc.Format = curTexRes->GetDesc().Format;
				texture->m_skyTexHeapIndex = texture->m_offsetIndex;
			}

			CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDesc{ srvDescHeap->GetCPUDescriptorHandleForHeapStart() };
			hCpuDesc.Offset(texture->m_offsetIndex++, cbvSrvUavDescSize);
			device->CreateShaderResourceView(curTex->resource.Get(), &srvDesc, hCpuDesc);
		});
}

bool CTexture::LoadGraphicMemory()
{
	std::vector<std::wstring> commonFilenames = { L"bricks.dds", L"stone.dds", L"tile.dds", L"WoodCrate01.dds",
		L"ice.dds", L"grass.dds", L"white1x1.dds" };
	std::vector<std::wstring> cubeFilenames = { L"grasscube1024.dds" };

	ReturnIfFalse(LoadTexture(eType::Common, commonFilenames));
	//ReturnIfFalse(LoadTexture(eType::Cube, cubeFilenames));
	
	return true;
}

bool CTexture::LoadTexture(eType type, std::vector<std::wstring>& filenames)
{
	ReturnIfFalse(m_renderer->GetDirectx3D()->LoadData(
		[texture = this, &filenames, type](ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)->bool {
			ReturnIfFalse(texture->Upload(device, cmdList, type, filenames));
			return true; }));

	CreateShaderResourceView(type);

	return true;
}