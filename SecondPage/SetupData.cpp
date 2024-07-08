#include "./SetupData.h"
#include <algorithm>
#include <ranges>
#include "../Include/RenderItem.h"
#include "../Include/FrameResourceData.h"
#include "../Include/Interface.h"
#include "./Utility.h"
#include "./Material.h"
#include "./Mesh.h"
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
			SubRenderItem* subRenderItem = GetSubRenderItem((*renderItems), geoProp.first, meshProp.first);
			subRenderItem->instanceDataList = meshProp.second.instanceDataList;
			subRenderItem->cullingFrustum = meshProp.second.cullingFrustum;
			});
		});

	return true;
}

bool CSetupData::InsertModelProperty(const std::string& geoName, const std::string& meshName, ModelProperty&& mProperty, CMaterial* material)
{
	if (mProperty.meshData != nullptr && mProperty.meshData->vertices.empty()) return false;

	material->SetMaterialList(mProperty.materialList);

	auto& mesh = m_allModelProperty[geoName];
	if (mesh.find(meshName) != mesh.end())
		return false;

	m_allModelProperty[geoName].insert(std::make_pair(meshName, std::move(mProperty)));

	return true;
}

bool CSetupData::LoadMesh(CMesh* mesh, const std::string& geoName, MeshProperty& meshProp)
{
	return std::ranges::all_of(meshProp, [mesh, &geoName](auto& mProp) {
		return mesh->LoadGeometry(geoName, mProp.first, &mProp.second);});
}

bool CSetupData::LoadMesh(CMesh* mesh, AllRenderItems* renderItems)
{
	ReturnIfFalse( std::ranges::all_of(m_allModelProperty, [this, mesh](auto& geoProp) {
		auto& geoName = geoProp.first;
		return LoadMesh(mesh, geoName, geoProp.second); }));

	return FillRenderItems(renderItems);
}