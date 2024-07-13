#include "Texture.h"
#include "./DDSTextureLoader.h"
#include "./d3dUtil.h"
#include "./Directx3D.h"
#include "./Renderer.h"
#include "../Include/Types.h"

using namespace DirectX;

CTexture::TextureMemory::TextureMemory()
	: resource{ nullptr }
	, uploadHeap{ nullptr }
{}
CTexture::TextureMemory::~TextureMemory() = default;

CTexture::CTexture(std::wstring resPath)
	: m_resPath(std::move(resPath))
	, m_filePath{ L"Textures/" }
	, m_textureMemories{}
{}
CTexture::~CTexture() = default;

bool CTexture::Upload(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const TextureList& textureList)
{
	return std::ranges::all_of(textureList, [this, device, cmdList](auto& tex) {
		auto& type = tex.first;
		auto& filename = tex.second;
		auto texMemory = std::make_unique<TextureMemory>();

		texMemory->path = m_resPath + m_filePath;
		texMemory->filename = filename;
		std::wstring fullFilename = texMemory->path + texMemory->filename;

		ReturnIfFailed(CreateDDSTextureFromFile12(device, cmdList,
			fullFilename.c_str(), texMemory->resource, texMemory->uploadHeap));
		m_textureMemories.emplace_back(std::make_pair(type, std::move(texMemory)));
		return true; });
}

void CTexture::CreateShaderResourceView(CRenderer* renderer)
{
	std::ranges::for_each(m_textureMemories,
		[renderer](auto& curTex) mutable {
			auto& type = curTex.first;
			auto& curTexMemory = curTex.second;
			auto& curTexRes = curTexMemory->resource;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = curTexRes->GetDesc().Format;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MipLevels = curTexRes->GetDesc().MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

			if (type == eTextureType::TextureCube)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srvDesc.TextureCube.MostDetailedMip = 0;
				srvDesc.TextureCube.MipLevels = curTexRes->GetDesc().MipLevels;
				srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
				srvDesc.Format = curTexRes->GetDesc().Format;
			}

			renderer->CreateShaderResourceView(type, curTexMemory->filename, curTexRes.Get(), &srvDesc);
		});
}