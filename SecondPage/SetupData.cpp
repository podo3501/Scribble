#include "./SetupData.h"
#include <algorithm>
#include <ranges>
#include "../Include/RenderItem.h"
#include "../Include/FrameResourceData.h"
#include "../Include/Types.h"
#include "../Include/Interface.h"
#include "./Utility.h"
#include "./Material.h"
#include "./Model.h"
#include "./Helper.h"

using namespace DirectX;

CSetupData::CSetupData() {}
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

bool CSetupData::LoadMesh(CModel* model, const std::string& geoName, MeshProperty& meshProp)
{
	return std::ranges::all_of(meshProp, [model, &geoName](auto& mProp) {
		return model->LoadGeometry(geoName, mProp.first, &mProp.second);});
}

bool CSetupData::LoadModel(CModel* model, AllRenderItems* renderItems)
{
	ReturnIfFalse( std::ranges::all_of(m_allModelProperty, [this, model](auto& geoProp) {
		auto& geoName = geoProp.first;
		return LoadMesh(model, geoName, geoProp.second); }));

	return FillRenderItems(renderItems);
}

void CSetupData::GetPassCB(PassConstants* outPc)
{
	(*outPc).nearZ = 1.0f;
	(*outPc).farZ = 1000.0f;
	(*outPc).ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	(*outPc).lights[0].direction = { 0.57735f, -0.57735f, 0.57735f };
	(*outPc).lights[0].strength = { 0.8f, 0.8f, 0.8f };
	(*outPc).lights[1].direction = { -0.57735f, -0.57735f, 0.57735f };
	(*outPc).lights[1].strength = { 0.4f, 0.4f, 0.4f };
	(*outPc).lights[2].direction = { 0.0f, -0.707f, -0.707f };
	(*outPc).lights[2].strength = { 0.2f, 0.2f, 0.2f };
}