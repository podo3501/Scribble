#include "./Material.h"
#include "../Core/d3dUtil.h"
#include "./UploadBuffer.h"
#include "./FrameResourceData.h"
#include "./RendererDefine.h"

using namespace DirectX;

CMaterial::CMaterial()
{}

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

void CMaterial::MakeMaterialBuffer(CUploadBuffer** outMatBuffer)
{
	int matCBIndex = { 0 };
	TextureType curType{ TextureType::None };
	int diffuseIndex = { 0 };
	for (auto& e : m_materials)
	{
		Material* m = e.get();
		if (m->numFramesDirty <= 0)
			continue;

		if (curType != m->type)
		{
			curType = m->type;
			diffuseIndex = 0;
		}

		MaterialBuffer matData;
		matData.diffuseMapIndex = diffuseIndex++;
		matData.diffuseAlbedo = m->diffuseAlbedo;
		matData.fresnelR0 = m->fresnelR0;
		matData.roughness = m->roughness;
		XMStoreFloat4x4(&matData.matTransform, XMMatrixTranspose(m->transform));

		(*outMatBuffer)->CopyData(matCBIndex++, matData);

		m->numFramesDirty--;
	}
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
