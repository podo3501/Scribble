#include "./Material.h"
#include <ranges>
#include <algorithm>
#include "../Core/d3dUtil.h"
#include "../Include/FrameResourceData.h"
#include "../Include/RendererDefine.h"
#include "../Include/Interface.h"
#include "./Texture.h"

using namespace DirectX;

struct CMaterial::Material
{
	eTextureType type{ eTextureType::None };
	int normalSrvHeapIndex{ -1 };	//normal map

	DirectX::XMFLOAT4 diffuseAlbedo{ 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 fresnelR0{ 0.01f, 0.01f, 0.01f };
	float roughness{ .25f };
	DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity();

	int numFramesDirty{ gFrameResourceCount };
};

CMaterial::CMaterial()
	: m_materials{}
{}
CMaterial::~CMaterial() = default;

void CMaterial::Build()
{
	auto MakeMaterial = [&](std::string&& name, eTextureType type, XMFLOAT4 diffuseAlbedo, 
		XMFLOAT3 fresnelR0, float rough) {
			auto curMat = std::make_unique<Material>();
			curMat->type = type;
			curMat->diffuseAlbedo = diffuseAlbedo;
			curMat->fresnelR0 = fresnelR0;
			curMat->roughness = rough;
			m_materials.emplace_back(std::make_pair(name, std::move(curMat)));
		};

	MakeMaterial("sky", eTextureType::Cube, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 1.0f);
	MakeMaterial("bricks0", eTextureType::Common, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.002f, 0.002f, 0.02f }, 0.1f);
	MakeMaterial("stone0", eTextureType::Common, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.3f);
	MakeMaterial("tile0", eTextureType::Common, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.02f, 0.02f, 0.02f }, 0.3f);
	MakeMaterial("checkboard0", eTextureType::Common, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f);
	MakeMaterial("ice0", eTextureType::Common, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 0.0f);
	MakeMaterial("grass0", eTextureType::Common, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f);
	MakeMaterial("skullMat", eTextureType::Common, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.5f);
}

MaterialBuffer CMaterial::ConvertUploadBuffer(UINT diffuseIndex, Material* material)
{
	MaterialBuffer matData;
	matData.diffuseMapIndex = diffuseIndex;
	matData.diffuseAlbedo = material->diffuseAlbedo;
	matData.fresnelR0 = material->fresnelR0;
	matData.roughness = material->roughness;
	XMStoreFloat4x4(&matData.matTransform, XMMatrixTranspose(material->transform));

	return matData;
}

void CMaterial::MakeMaterialBuffer(IRenderer* renderer)
{
	auto updateMaterials = m_materials | std::ranges::views::filter([](auto& iter) { return iter.second.get()->numFramesDirty > 0; });

	std::vector<MaterialBuffer> materialBufferDatas{};
	std::ranges::transform(updateMaterials, std::back_inserter(materialBufferDatas), 
		[this, diffuseIndex{ 0u }, curType{ eTextureType::None }](auto& m) mutable {
			Material* mat = m.second.get();
			if (curType != mat->type)
			{
				curType = mat->type;
				diffuseIndex = 0;
			}
			mat->numFramesDirty--;

			return ConvertUploadBuffer(diffuseIndex++, mat); });
	if (materialBufferDatas.empty())
		return; 

	renderer->SetUploadBuffer(eBufferType::Material, materialBufferDatas.data(), materialBufferDatas.size());
}
