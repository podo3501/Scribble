#include "pch.h"
#include "Texture.h"
#include "./d3dUtil.h"
#include "./Directx3D.h"
#include "./Renderer.h"
#include "../Include/Types.h"
#include "./DescriptorHeap.h"

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

bool CTexture::Upload(ID3D12Device* device, ID3D12CommandQueue* cmdQueue, const TextureList& textureList)
{
	return std::ranges::all_of(textureList, [this, device, cmdQueue](auto& tex) {
		auto& type = tex.first;
		auto& filename = tex.second;
		auto texMemory = std::make_unique<TextureMemory>();

		texMemory->path = m_resPath + m_filePath;
		texMemory->filename = filename;
		std::wstring fullFilename = texMemory->path + texMemory->filename;

		ResourceUploadBatch resourceUpload(device);
		resourceUpload.Begin();
		
		ReturnIfFailed(CreateDDSTextureFromFile(device, resourceUpload,
			fullFilename.c_str(), texMemory->resource.ReleaseAndGetAddressOf()));// , texMemory->uploadHeap));

		auto uploadResourcesFinished = resourceUpload.End(cmdQueue);
		uploadResourcesFinished.wait();

		m_textureMemories.emplace_back(std::make_pair(type, std::move(texMemory)));

		return true; 
		});
}

void CTexture::CreateShaderResourceView(CDescriptorHeap* descHeap)
{
	std::ranges::for_each(m_textureMemories,
		[descHeap, this](auto& curTex) mutable {
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

			if (type == SrvOffset::TextureCube)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srvDesc.TextureCube.MostDetailedMip = 0;
				srvDesc.TextureCube.MipLevels = curTexRes->GetDesc().MipLevels;
				srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
				srvDesc.Format = curTexRes->GetDesc().Format;
			}

			UINT index = static_cast<UINT>(m_srvTexture2DFilename.size());
			descHeap->CreateShaderResourceView(type, index, &srvDesc, curTexRes.Get());

			if(type == SrvOffset::Texture2D)
				m_srvTexture2DFilename.emplace_back(curTexMemory->filename);
		});
}