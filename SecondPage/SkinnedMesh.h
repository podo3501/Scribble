#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <combaseapi.h>

interface IRenderer;
class CSkinnedData;
class CLoadM3D;
class CMaterial;
struct ModelProperty;
struct SkinnedVertex;
struct Subset;
struct M3dMaterial;
struct SkinnedModelInstance;
struct RenderItem;
struct SubRenderItem;
enum class GraphicsPSO : int;

class CSkinnedMesh
{
	using SkinnedVertices = std::vector<SkinnedVertex>;
	using Indices = std::vector<std::int32_t>;
	using AllRenderItems = std::map<GraphicsPSO, std::unique_ptr<RenderItem>>;

public:
	CSkinnedMesh(const std::wstring& resPath);
	~CSkinnedMesh();

	bool Read(const std::string& meshName, ModelProperty* mProperty);
	bool LoadMeshIntoVRAM(IRenderer* renderer, CMaterial* material, AllRenderItems* outRenderItems);
	void UpdateAnimation(IRenderer* renderer, float deltaTime);

private:
	bool LoadVRAM(IRenderer* renderer, RenderItem* outRenderItems);
	bool InsertSubmesh(RenderItem* outRenderItems);
	bool SetTransform(const std::string& matName, SubRenderItem& subRItem);
	bool InsertMaterial(CMaterial* material);

private:
	std::wstring m_resPath{};
	const std::wstring m_filePath{ L"Meshes/" };

	SkinnedVertices m_skinnedVertices;
	Indices m_indices;

	std::unique_ptr<CSkinnedData> m_skinnedInfo;
	std::vector<Subset> m_skinnedSubsets;
	std::vector<M3dMaterial> m_skinnedMats;
	std::unique_ptr<SkinnedModelInstance> m_skinnedModelInst;
};