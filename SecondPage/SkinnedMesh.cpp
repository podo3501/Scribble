#include "./SkinnedMesh.h"
#include "../Include/Interface.h"
#include "../Include/FrameResourceData.h"
#include "../Include/RenderItem.h"
#include "../Include/Types.h"
#include <vector>
#include <ranges>
#include <algorithm>
#include <DirectXMath.h>
#include "./SkinnedData.h"
#include "./SetupData.h"
#include "./LoadM3D.h"
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

bool CSkinnedMesh::LoadMeshIntoVRAM(IRenderer* renderer, AllRenderItems* outRenderItems)
{
	if (m_skinnedVertices.empty() || m_indices.empty()) return true;

	auto renderItem = MakeRenderItem(*outRenderItems, GraphicsPSO::SkinnedOpaque);

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