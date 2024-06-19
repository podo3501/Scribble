#include "./Material.h"
#include "../Core/d3dUtil.h"
#include "../Core/UploadBuffer.h"
#include "./FrameResource.h"

using namespace DirectX;

CMaterial::CMaterial()
{}

void CMaterial::Build()
{
	auto MakeMaterial = [&](std::string&& name, int matCBIdx, int diffuseSrvHeapIdx,
		XMFLOAT4 diffuseAlbedo, XMFLOAT3 fresnelR0, float rough) {
			auto curMat = std::make_unique<Material>();
			curMat->Name = name;
			curMat->MatCBIndex = matCBIdx;
			curMat->DiffuseSrvHeapIndex = diffuseSrvHeapIdx;
			curMat->DiffuseAlbedo = diffuseAlbedo;
			curMat->FresnelR0 = fresnelR0;
			curMat->Roughness = rough;
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

void CMaterial::UpdateMaterialBuffer(UploadBuffer* materialBuffer)
{
	for (auto& e : m_materials)
	{
		Material* m = e.second.get();
		if (m->NumFramesDirty <= 0)
			continue;

		XMMATRIX matTransform = XMLoadFloat4x4(&m->MatTransform);

		MaterialData matData;
		matData.DiffuseAlbedo = m->DiffuseAlbedo;
		matData.FresnelR0 = m->FresnelR0;
		matData.Roughness = m->Roughness;
		XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
		matData.DiffuseMapIndex = m->DiffuseSrvHeapIndex;

		materialBuffer->CopyData(m->MatCBIndex, matData);

		m->NumFramesDirty--;
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
