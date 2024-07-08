#include "./Material.h"
#include <ranges>
#include <algorithm>
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

bool CMaterial::CheckSameFilename(const std::wstring& filename)
{
	return std::ranges::count_if(m_textureList, [&filename](auto& tex) {
		return tex.second == filename; });
}

void CMaterial::SetMaterialList(const MaterialList& materialList)
{
	std::ranges::copy_if(materialList, std::back_inserter(m_materialList), [this](auto& mat) {
		auto find = std::ranges::find_if(m_materialList, [&mat](auto& mMat) {
			return mMat->name == mat->name; });
		return (find == m_materialList.end()); });

	auto uniqNameMatList = m_materialList |
		std::views::filter([this](auto& iter) { return CheckSameFilename(iter->filename) == false; });

	std::ranges::transform(uniqNameMatList, std::back_inserter(m_textureList), [this](auto& mat) {
		return std::make_pair(mat->type, mat->filename); });
}

bool CMaterial::LoadTextureIntoVRAM(IRenderer* renderer)
{
	renderer->LoadTexture(m_textureList);

	return true;
}

int CMaterial::GetDiffuseIndex(const std::wstring& filename)
{
	auto find = std::ranges::find_if(m_textureList, [&filename](auto& tex) {
		return tex.second == filename; });
	
	if (find == m_textureList.end())
		return -1;

	return static_cast<int>(std::distance(m_textureList.begin(), find));
}

int CMaterial::GetMaterialIndex(const std::string& matName)
{
	auto find = std::ranges::find_if(m_materialList, [&matName](auto& mat) {
		return mat->name == matName; });
	if (find == m_materialList.end())
		return -1;

	return static_cast<int>(std::distance(m_materialList.begin(), find));
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
	std::vector<MaterialBuffer> materialBufferDatas{};
	std::ranges::for_each(m_materialList, [this, &materialBufferDatas](auto& m) {
		Material* mat = m.get();
		if (mat->numFramesDirty <= 0) return;

		int diffuseIndex = GetDiffuseIndex(mat->filename);
		materialBufferDatas.emplace_back(ConvertUploadBuffer(diffuseIndex, mat));
		mat->numFramesDirty--; });

	if (materialBufferDatas.empty())
		return; 

	renderer->SetUploadBuffer(eBufferType::Material, materialBufferDatas.data(), materialBufferDatas.size());
}
