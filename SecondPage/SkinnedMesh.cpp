#include "./SkinnedMesh.h"
#include "../Include/FrameResourceData.h"
#include <vector>
#include <DirectXMath.h>
#include "./SkinnedData.h"
#include "./SetupData.h"
#include "./LoadM3D.h"

CSkinnedMesh::CSkinnedMesh(const std::wstring& resPath)
	: m_resPath{resPath}
	, m_skinnedInfo{ nullptr }
	, m_skinnedSubsets{}
	, m_skinnedMats{}
{}

CSkinnedMesh::~CSkinnedMesh() = default;

struct SkinnedModelInstance
{
	CSkinnedData* skinnedInfo = nullptr;
	std::vector<DirectX::XMFLOAT4X4> finalTransforms;
	std::string clipName;
	float timePos{ 0.0f };

	void UpdateSkinnedAnimation(float dt)
	{
		timePos += dt;

		if (timePos > skinnedInfo->GetClipEndTime(clipName))
			timePos = 0.0f;

		skinnedInfo->GetFinalTransforms(clipName, timePos, finalTransforms);
	}
};

bool CSkinnedMesh::Read(const std::string& meshName, ModelProperty* mProperty)
{
	std::wstring filename{ mProperty->filename };

	SkinnedVertices skinnedVertices{};
	Indices indices{};

	std::wstring fullFilename = m_resPath + m_filePath + filename;

	CLoadM3D loadM3d;
	loadM3d.Read(fullFilename, skinnedVertices, indices,
		m_skinnedSubsets, m_skinnedMats, m_skinnedInfo.get());

	std::unique_ptr<SkinnedModelInstance> mSkinnedModelInst{ nullptr };
	mSkinnedModelInst = std::make_unique<SkinnedModelInstance>();
	mSkinnedModelInst->skinnedInfo = m_skinnedInfo.get();
	mSkinnedModelInst->finalTransforms.resize(m_skinnedInfo->BoneCount());
	mSkinnedModelInst->clipName = "Take1";
	mSkinnedModelInst->timePos = 0.0f;

	return true;
}