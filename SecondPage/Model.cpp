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

CModel::CModel()
	: m_material{ nullptr }
	, m_mesh{ nullptr }
	, m_setupData{ nullptr }
{}
CModel::~CModel() = default;

bool CModel::Initialize(const std::wstring& resPath, const CreateModelNames& createModelNames)
{
	m_material = std::make_unique<CMaterial>();
	m_setupData = std::make_unique<CSetupData>();
	m_mesh = std::make_unique<CMesh>(resPath);

	return std::ranges::all_of(createModelNames, [this](auto& name) {
		const auto& pso = name.first;
		for (auto& meshName : name.second)
		{
			ReturnIfFalse(m_setupData->InsertModelProperty(
				pso, meshName, CreateMock(meshName), m_material.get()));
		}
		return true; });
}

bool CModel::LoadMemory(IRenderer* renderer, AllRenderItems& allRenderItems)
{
	ReturnIfFalse(m_material->LoadTextureIntoVRAM(renderer));
	ReturnIfFalse(m_setupData->LoadMesh(m_mesh.get(), &allRenderItems));
	ReturnIfFalse(m_mesh->LoadMeshIntoVRAM(renderer, &allRenderItems));

	return true;
}

void CModel::UpdateRenderItems(IRenderer* renderer, CCamera* camera, AllRenderItems& allRenderItems)
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
	UpdateInstanceBuffer(renderer, totalVisibleInstance);
}

void CModel::UpdateInstanceBuffer(IRenderer* renderer, const InstanceDataList& visibleInstance)
{
	std::vector<InstanceBuffer> instanceBufferDatas{};
	std::ranges::transform(visibleInstance, std::back_inserter(instanceBufferDatas), [this](auto& visibleData) {
		InstanceBuffer curInsBuf{};
		XMStoreFloat4x4(&curInsBuf.world, DirectX::XMMatrixTranspose(visibleData->world));
		XMStoreFloat4x4(&curInsBuf.texTransform, XMMatrixTranspose(visibleData->texTransform));
		curInsBuf.materialIndex = m_material->GetMaterialIndex(visibleData->matName);
		return std::move(curInsBuf); });

	renderer->SetUploadBuffer(eBufferType::Instance, instanceBufferDatas.data(), instanceBufferDatas.size());
}

void CModel::Update(IRenderer* renderer, CCamera* camera, AllRenderItems& allRenderItems)
{
	m_material->MakeMaterialBuffer(renderer);
	UpdateRenderItems(renderer, camera, allRenderItems);
}

