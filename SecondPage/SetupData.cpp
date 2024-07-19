#include "./SetupData.h"
#include <algorithm>
#include <ranges>
#include "../Include/RenderItem.h"
#include "../Include/FrameResourceData.h"
#include "../Include/Interface.h"
#include "../Include/Types.h"
#include "./Utility.h"
#include "./Material.h"
#include "./Mesh.h"
#include "./SkinnedMesh.h"
#include "./Helper.h"

using namespace DirectX;

CSetupData::CSetupData()
	: m_allModelProperty{}
{}
CSetupData::~CSetupData() = default;

bool CSetupData::FillRenderItems(AllRenderItems* renderItems)
{
	std::ranges::for_each(m_allModelProperty, [renderItems](auto& geoProp) {
		std::ranges::for_each(geoProp.second, [renderItems, &geoProp](auto& meshProp) {
			SubRenderItem* subRenderItem = MakeSubRenderItem((*renderItems), geoProp.first, meshProp.first);
			subRenderItem->instanceDataList = meshProp.second.instanceDataList;
			subRenderItem->cullingFrustum = meshProp.second.cullingFrustum;
			});
		});

	return true;
}

bool CSetupData::InsertModelProperty(GraphicsPSO pso, const std::string& meshName, ModelProperty&& mProperty, CMaterial* material)
{
	if (mProperty.meshData != nullptr && mProperty.meshData->vertices.empty()) return false;

	material->SetMaterialList(mProperty.materialList);

	auto& mesh = m_allModelProperty[pso];
	if (mesh.find(meshName) != mesh.end())
		return false;

	m_allModelProperty[pso].insert(std::make_pair(meshName, std::move(mProperty)));

	return true;
}

bool CSetupData::LoadMesh(CMesh* mesh, CSkinnedMesh* skinnedMesh, GraphicsPSO pso, ModelProperties& modelProp)
{
	return std::ranges::all_of(modelProp, [mesh, skinnedMesh, pso](auto& mProp) {
		if (pso == GraphicsPSO::SkinnedOpaque)
			return skinnedMesh->Read(mProp.first, &mProp.second);
		return mesh->LoadGeometry(pso, mProp.first, &mProp.second);});
}

bool CSetupData::LoadMesh(CMesh* mesh, CSkinnedMesh* skinnedMesh, AllRenderItems* renderItems)
{
	ReturnIfFalse( std::ranges::all_of(m_allModelProperty, [this, mesh, skinnedMesh](auto& geoProp) {
		auto& pso = geoProp.first;
		return LoadMesh(mesh, skinnedMesh, pso, geoProp.second); }));

	return FillRenderItems(renderItems);
}