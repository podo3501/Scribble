#include "./Material.h"
#include <ranges>
#include <algorithm>
#include "../Core/d3dUtil.h"
#include "../Include/FrameResourceData.h"
#include "../Include/RendererDefine.h"
#include "../Include/Interface.h"

using namespace DirectX;

Material::Material()
	: numFramesDirty{ gFrameResourceCount }
{}

CMaterial::CMaterial() = default;
CMaterial::~CMaterial() = default;

void CMaterial::Build()
{
	auto MakeMaterial = [&](std::string&& name, TextureType type, XMFLOAT4 diffuseAlbedo, 
		XMFLOAT3 fresnelR0, float rough) {
			auto curMat = std::make_unique<Material>();
			curMat->type = type;
			curMat->name = name;
			curMat->diffuseAlbedo = diffuseAlbedo;
			curMat->fresnelR0 = fresnelR0;
			curMat->roughness = rough;
			m_materials.emplace_back(std::move(curMat));
		};

	MakeMaterial("sky", TextureType::CubeTexture, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 1.0f);
	MakeMaterial("bricks0", TextureType::Texture, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.002f, 0.002f, 0.02f }, 0.1f);
	MakeMaterial("stone0", TextureType::Texture, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.3f);
	MakeMaterial("tile0", TextureType::Texture, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.02f, 0.02f, 0.02f }, 0.3f);
	MakeMaterial("checkboard0", TextureType::Texture, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f);
	MakeMaterial("ice0", TextureType::Texture, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 0.0f);
	MakeMaterial("grass0", TextureType::Texture, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f);
	MakeMaterial("skullMat", TextureType::Texture, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.5f);
}

MaterialBuffer ConvertUploadBuffer(UINT diffuseIndex, Material* material)
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
	auto updateMaterials = m_materials | std::ranges::views::filter([](auto& iter) { return iter.get()->numFramesDirty > 0; });

	std::vector<MaterialBuffer> materialBufferDatas{};
	std::ranges::transform(updateMaterials, std::back_inserter(materialBufferDatas), 
		[diffuseIndex{ 0u }, curType{ TextureType::None }](auto& m) mutable {
			Material* mat = m.get();
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

Material* CMaterial::GetMaterial(const std::string& matName)
{
	auto findIter = std::ranges::find_if(m_materials, [&matName](auto& mat) {
		return mat->name == matName; });
	if (findIter == m_materials.end())
		return nullptr;

	return findIter->get();
}

size_t CMaterial::GetCount(TextureType type)
{
	switch (type)
	{
	case TextureType::CubeTexture:
	case TextureType::Texture:
		return std::ranges::count_if(m_materials, [type](auto& iter) { return iter->type == type; });
	case TextureType::Total:
		return m_materials.size();
	}

	return 0;
}

int CMaterial::GetStartIndex(TextureType type)
{
	auto find = std::ranges::find_if(m_materials, [type](auto& mat) { return mat->type == type; });
	if (find == m_materials.end()) return -1;

	return static_cast<int>(std::distance(m_materials.begin(), find));
}
