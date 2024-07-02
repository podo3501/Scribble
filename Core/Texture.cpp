#include "Texture.h"
#include "./DDSTextureLoader.h"
#include "./d3dUtil.h"
#include "./Directx3D.h"
#include "./Renderer.h"
#include "../Include/Types.h"

using namespace DirectX;

CTexture::CTexture(std::wstring resPath)
	: m_resPath(std::move(resPath))
{}
CTexture::~CTexture() = default;

bool CTexture::Upload(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, eTextureType type, std::vector<std::wstring>& filenames)
{
	auto result = std::ranges::all_of(filenames, [this, device, cmdList, type](auto& curFilename) {
			auto texMemory = std::make_unique<TextureMemory>();
			texMemory->filename = m_resPath + m_filePath + curFilename;
			ReturnIfFailed(CreateDDSTextureFromFile12(device, cmdList,
				texMemory->filename.c_str(), texMemory->resource, texMemory->uploadHeap));
			m_texMemories[type].emplace_back(std::move(texMemory));
			return true; });

	if (!result) return result;

	return true;
}

void CTexture::CreateShaderResourceView(CRenderer* renderer, eTextureType type)
{
	auto device = renderer->GetDevice();
	UINT cbvSrvUavDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto srvDescHeap = renderer->GetSrvDescriptorHeap();
	std::ranges::for_each(m_texMemories[type],
		[this, srvDescHeap, cbvSrvUavDescSize, device, type](auto& curTex) mutable {
			auto& curTexRes = curTex->resource;
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = curTexRes->GetDesc().Format;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MipLevels = curTexRes->GetDesc().MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

			if (type == eTextureType::Cube)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srvDesc.TextureCube.MostDetailedMip = 0;
				srvDesc.TextureCube.MipLevels = curTexRes->GetDesc().MipLevels;
				srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
				srvDesc.Format = curTexRes->GetDesc().Format;
				m_skyTexHeapIndex = m_offsetIndex;
			}

			CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDesc{ srvDescHeap->GetCPUDescriptorHandleForHeapStart() };
			hCpuDesc.Offset(m_offsetIndex++, cbvSrvUavDescSize);
			device->CreateShaderResourceView(curTex->resource.Get(), &srvDesc, hCpuDesc);
		});
}