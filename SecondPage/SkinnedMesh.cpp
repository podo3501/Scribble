#include "./SkinnedMesh.h"
#include <DirectXMath.h>
#include "../Include/Interface.h"
#include "../Include/FrameResourceData.h"
#include "../Include/RenderItem.h"
#include "../Include/Types.h"
#include <numbers>
#include <vector>
#include <ranges>
#include <algorithm>
#include <DirectXMath.h>
#include "./SkinnedData.h"
#include "./SetupData.h"
#include "./LoadM3D.h"
#include "./Material.h"
#include "./Helper.h"
#include "./Utility.h"

CSkinnedMesh::CSkinnedMesh(const std::wstring& resPath)
	: m_resPath{resPath}
	, m_skinnedVertices{}
	, m_indices{}
	, m_skinnedInfo{ std::make_unique<CSkinnedData>() }
	, m_skinnedSubsets{}
	, m_skinnedMats{}
	, m_skinnedModelInst{ std::make_unique<SkinnedModelInstance>() }
{}

CSkinnedMesh::~CSkinnedMesh() = default;

bool CSkinnedMesh::Read(const std::string& meshName, ModelProperty* mProperty)
{
	std::wstring filename{ mProperty->filename };

	std::wstring fullFilename = m_resPath + m_filePath + filename;

	CLoadM3D loadM3d;
	loadM3d.Read(fullFilename, m_skinnedVertices, m_indices,
		m_skinnedSubsets, m_skinnedMats, m_skinnedInfo.get());

	m_skinnedModelInst->skinnedInfo = m_skinnedInfo.get();
	m_skinnedModelInst->finalTransforms.resize(m_skinnedInfo->BoneCount());
	m_skinnedModelInst->clipName = "Take1";
	m_skinnedModelInst->timePos = 0.0f;

	return true;
}

bool CSkinnedMesh::LoadMeshIntoVRAM(IRenderer* renderer, CMaterial* material, AllRenderItems* outRenderItems)
{
	if (m_skinnedVertices.empty() || m_indices.empty()) return true;

	auto renderItem = MakeRenderItem(*outRenderItems, GraphicsPSO::SkinnedOpaque);

	ReturnIfFalse(LoadVRAM(renderer, renderItem));
	ReturnIfFalse(InsertSubmesh(renderItem));
	ReturnIfFalse(InsertMaterial(material));

	return true;
}

void CSkinnedMesh::UpdateAnimation(IRenderer* renderer, float deltaTime)
{
	if (m_skinnedModelInst->skinnedInfo == nullptr) return;

	m_skinnedModelInst->UpdateSkinnedAnimation(deltaTime);

	SkinnedConstants skinnedConstants;
	std::ranges::copy( m_skinnedModelInst->finalTransforms, &skinnedConstants.boneTransforms[0]);

	renderer->SetUploadBuffer(eBufferType::SkinnedCB, &skinnedConstants, 1);
}

bool CSkinnedMesh::LoadVRAM(IRenderer* renderer, RenderItem* renderItem)
{
	UINT vbByteSize = static_cast<UINT>(m_skinnedVertices.size()) * sizeof(SkinnedVertex);
	UINT ibByteSize = static_cast<UINT>(m_indices.size()) * sizeof(std::int32_t);

	renderItem->vertexBufferView.SizeInBytes = vbByteSize;
	renderItem->vertexBufferView.StrideInBytes = sizeof(Vertex);

	renderItem->indexBufferView.SizeInBytes = ibByteSize;
	renderItem->indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	//그래픽 메모리에 올린다.
	ReturnIfFalse(renderer->LoadSkinnedMesh(m_skinnedVertices, m_indices, renderItem));

	return true;
}

bool CSkinnedMesh::InsertSubmesh(RenderItem* renderItem)
{
	std::ranges::for_each(m_skinnedSubsets, [this, renderItem, idx{ 0 }](auto& subset) mutable {
		std::string name = "sm_" + std::to_string(idx++);
		SubRenderItem subRenderItem;
		SetTransform(name, subRenderItem);
		
		SubItem& subItem = subRenderItem.subItem;
		subItem.indexCount = static_cast<UINT>(subset.faceCount) * 3;
		subItem.startIndexLocation = static_cast<UINT>(subset.faceStart) * 3;
		subItem.baseVertexLocation = 0;

		renderItem->subRenderItems.insert(std::make_pair(name, subRenderItem)); });

	return true;
}

bool CSkinnedMesh::SetTransform(const std::string& matName, SubRenderItem& subRItem)
{
	subRItem.instanceCount = 1;
	std::shared_ptr<InstanceData> instanceData = std::make_shared<InstanceData>();
	
	DirectX::XMMATRIX modelScale = DirectX::XMMatrixScaling(0.05f, 0.05f, -0.05f);
	DirectX::XMMATRIX modelRot = DirectX::XMMatrixRotationY(static_cast<float>(std::numbers::pi));
	DirectX::XMMATRIX modelOffset = DirectX::XMMatrixTranslation(0.0f, 0.0f, -5.0f);

	instanceData->matName = matName;
	instanceData->world = modelScale * modelRot * modelOffset;
	instanceData->texTransform = DirectX::XMMatrixIdentity();

	subRItem.instanceDataList.emplace_back(std::move(instanceData));

	return true;
}

bool CSkinnedMesh::InsertMaterial(CMaterial* material)
{
	MaterialList materialList;
	std::ranges::for_each(m_skinnedMats, [material, &materialList](auto& skinnedMat) {
		auto mat = std::make_shared<Material>();
		mat->name = skinnedMat.name;
		mat->type = eTextureType::Texture2D;
		mat->diffuseName.assign(skinnedMat.diffuseMapName.begin(), skinnedMat.diffuseMapName.end());
		mat->normalName.assign(skinnedMat.normalMapName.begin(), skinnedMat.normalMapName.end());
		mat->diffuseAlbedo = skinnedMat.diffuseAlbedo;
		mat->fresnelR0 = skinnedMat.fresnelR0;
		mat->roughness = skinnedMat.roughness; 
		materialList.emplace_back(std::move(mat)); });

	material->SetMaterialList(materialList);

	return true;
}