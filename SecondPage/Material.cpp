#include "./Material.h"
#include "../Core/d3dUtil.h"
#include "./UploadBuffer.h"
#include "./FrameResourceData.h"

using namespace DirectX;

CMaterial::CMaterial()
{}

void CMaterial::Build()
{
	auto MakeMaterial = [&](std::string&& name, int matCBIdx, int diffuseSrvHeapIdx,
		XMFLOAT4 diffuseAlbedo, XMFLOAT3 fresnelR0, float rough) {
			auto curMat = std::make_unique<Material>();
			curMat->name = name;
			curMat->matCBIndex = matCBIdx;
			curMat->diffuseSrvHeapIndex = diffuseSrvHeapIdx;
			curMat->diffuseAlbedo = diffuseAlbedo;
			curMat->fresnelR0 = fresnelR0;
			curMat->roughness = rough;
			m_materials[name] = std::move(curMat);
		};

	MakeMaterial("bricks0", 0, 0, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.002f, 0.002f, 0.02f }, 0.1f);
	MakeMaterial("stone0", 1, 1, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.3f);
	MakeMaterial("tile0", 2, 2, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.02f, 0.02f, 0.02f }, 0.3f);
	MakeMaterial("checkboard0", 3, 3, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f);
	MakeMaterial("ice0", 4, 4, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 0.0f);
	MakeMaterial("grass0", 5, 5, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f);
	MakeMaterial("skullMat", 6, 6, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.5f);
}

void CMaterial::MakeMaterialBuffer(CUploadBuffer** outMatBuffer)
{
	for (auto& e : m_materials)
	{
		Material* m = e.second.get();
		if (m->numFramesDirty <= 0)
			continue;

		MaterialBuffer matData;
		matData.diffuseAlbedo = m->diffuseAlbedo;
		matData.fresnelR0 = m->fresnelR0;
		matData.roughness = m->roughness;
		XMStoreFloat4x4(&matData.matTransform, XMMatrixTranspose(m->transform));
		matData.diffuseMapIndex = m->diffuseSrvHeapIndex;

		(*outMatBuffer)->CopyData(m->matCBIndex, matData);

		m->numFramesDirty--;
	}
}

Material* CMaterial::GetMaterial(const std::string& matName)
{
	auto find = m_materials.find(matName);
	if (find == m_materials.end())
		return nullptr;

	return find->second.get();
}

size_t CMaterial::GetCount()
{
	return m_materials.size();
}
