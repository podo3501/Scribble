#include "./Model.h"
#include <algorithm>
#include <ranges>
#include <DirectXMath.h>
#include "../Include/Interface.h"
#include "../Include/RenderItem.h"
#include "../Include/FrameResourceData.h"
#include "./SetupData.h"
#include "./Mesh.h"
#include "./Material.h"
#include "./MockData.h"
#include "./Camera.h"
#include "./Utility.h"

CModel::CModel(IRenderer* renderer)
	: m_iRenderer(renderer)
{}
CModel::~CModel() = default;

bool CModel::Initialize(const std::wstring& resPath)
{
	m_material = std::make_unique<CMaterial>();
	m_setupData = std::make_unique<CSetupData>();
	m_mesh = std::make_unique<CMesh>(resPath);

	return MakeMockData(m_setupData.get(), m_material.get());
}

bool CModel::LoadMemory(AllRenderItems& allRenderItems)
{
	ReturnIfFalse(m_material->LoadTextureIntoVRAM(m_iRenderer));
	ReturnIfFalse(m_setupData->LoadMesh(m_mesh.get(), &allRenderItems));
	ReturnIfFalse(m_mesh->LoadMeshIntoVRAM(m_iRenderer, &allRenderItems));

	return true;
}

void CModel::UpdateRenderItems(CCamera* camera, const AllRenderItems& allRenderItems)
{
	//처리 안할것을 먼저 골라낸다.
	InstanceDataList totalVisibleInstance{};
	int instanceStartIndex{ 0 };
	for (auto& e : allRenderItems)
	{
		InstanceDataList visibleInstance{};
		auto renderItem = e.second.get();
		renderItem->startIndexInstance = instanceStartIndex;
		//보여지는 서브 아이템을 찾아낸다.
		camera->FindVisibleSubRenderItems(renderItem->subRenderItems, visibleInstance);
		instanceStartIndex += static_cast<int>(visibleInstance.size());

		std::ranges::move(visibleInstance, std::back_inserter(totalVisibleInstance));
	}
	UpdateInstanceBuffer(totalVisibleInstance);
}

void CModel::UpdateInstanceBuffer(const InstanceDataList& visibleInstance)
{
	std::vector<InstanceBuffer> instanceBufferDatas{};
	std::ranges::transform(visibleInstance, std::back_inserter(instanceBufferDatas), [this](auto& visibleData) {
		InstanceBuffer curInsBuf{};
		XMStoreFloat4x4(&curInsBuf.world, DirectX::XMMatrixTranspose(visibleData->world));
		XMStoreFloat4x4(&curInsBuf.texTransform, XMMatrixTranspose(visibleData->texTransform));
		curInsBuf.materialIndex = m_material->GetMaterialIndex(visibleData->matName);
		return std::move(curInsBuf); });

	m_iRenderer->SetUploadBuffer(eBufferType::Instance, instanceBufferDatas.data(), instanceBufferDatas.size());
}

void CModel::GetPassCB(PassConstants* outPc)
{
	m_setupData->GetPassCB(outPc);
}

void CModel::MakeMaterialBuffer()
{
	m_material->MakeMaterialBuffer(m_iRenderer);
}

