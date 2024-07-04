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
{}
CMaterial::~CMaterial() = default;

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
	auto updateMaterials = m_materialList | std::ranges::views::filter([](auto& iter) { return iter.get()->numFramesDirty > 0; });

	std::vector<MaterialBuffer> materialBufferDatas{};
	std::ranges::transform(updateMaterials, std::back_inserter(materialBufferDatas), 
		[this, diffuseIndex{ 0u }, curType{ eTextureType::None }](auto& m) mutable {
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
