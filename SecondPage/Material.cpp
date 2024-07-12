#include "./Material.h"
#include <algorithm>
#include <ranges>
#include "../Include/FrameResourceData.h"
#include "../Include/RendererDefine.h"
#include "../Include/Interface.h"
#include "../Include/Types.h"

using namespace DirectX;

Material::Material()
	: numFramesDirty{ gFrameResourceCount }
	, type{ eTextureType::None }
{}

CMaterial::CMaterial()
	: m_materialList{}
	, m_textureList{}
{}
CMaterial::~CMaterial() = default;

void CMaterial::InsertTexture(eTextureType type, const std::wstring& filename)
{
	if (filename.empty()) return;

	auto find = std::ranges::find_if(m_textureList, [&filename](auto& tex) {
		return tex.second == filename; });
	if (find != m_textureList.end()) return;

	m_textureList.emplace_back(std::make_pair(type, filename));
}

void CMaterial::SetMaterialList(const MaterialList& materialList)
{
	std::ranges::copy_if(materialList, std::back_inserter(m_materialList), [this](auto& mat) {
		auto find = std::ranges::find_if(m_materialList, [&mat](auto& mMat) {
			return mMat->name == mat->name; });
		return (find == m_materialList.end()); });

	std::ranges::for_each(m_materialList, [this](auto& mat) {
		InsertTexture(mat->type, mat->diffuseName);
		InsertTexture(mat->type, mat->normalName); });
}

bool CMaterial::LoadTextureIntoVRAM(IRenderer* renderer)
{
	return renderer->LoadTexture(m_textureList, &m_srvTextureList);
}

int CMaterial::GetSrvTextureIndex(const std::wstring& filename)
{
	auto find = std::ranges::find(m_srvTextureList, filename);
	if (find == m_srvTextureList.end())
		return -1;

	return static_cast<int>(std::distance(m_srvTextureList.begin(), find));
}

int CMaterial::GetMaterialIndex(const std::string& matName)
{
	auto find = std::ranges::find_if(m_materialList, [&matName](auto& mat) {
		return mat->name == matName; });
	if (find == m_materialList.end())
		return -1;

	return static_cast<int>(std::distance(m_materialList.begin(), find));
}

MaterialBuffer CMaterial::ConvertUploadBuffer(Material* material)
{
	MaterialBuffer matData;
	matData.diffuseMapIndex = GetSrvTextureIndex(material->diffuseName);
	matData.NormalMapIndex = GetSrvTextureIndex(material->normalName);
	matData.diffuseAlbedo = material->diffuseAlbedo;
	matData.fresnelR0 = material->fresnelR0;
	matData.roughness = material->roughness;
	XMStoreFloat4x4(&matData.matTransform, XMMatrixTranspose(material->transform));

	return matData;
}

void CMaterial::MakeMaterialBuffer(IRenderer* renderer)
{
	std::vector<MaterialBuffer> materialBufferDatas{};
	std::ranges::for_each(m_materialList, [this, &materialBufferDatas](auto& m) {
		Material* mat = m.get();
		if (mat->numFramesDirty <= 0) return;

		materialBufferDatas.emplace_back(ConvertUploadBuffer(mat));
		mat->numFramesDirty--; });

	if (materialBufferDatas.empty())
		return; 

	renderer->SetUploadBuffer(eBufferType::Material, materialBufferDatas.data(), materialBufferDatas.size());
}
