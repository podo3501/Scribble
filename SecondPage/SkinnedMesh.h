#pragma once

#include <string>
#include <vector>
#include <memory>

class CSkinnedData;
class CLoadM3D;
struct ModelProperty;
struct SkinnedVertex;
struct Subset;
struct M3dMaterial;

using SkinnedVertices = std::vector<SkinnedVertex>;
using Indices = std::vector<std::int32_t>;

class CSkinnedMesh
{
public:
	CSkinnedMesh(const std::wstring& resPath);
	~CSkinnedMesh();

	bool Read(const std::string& meshName, ModelProperty* mProperty);

private:
	std::wstring m_resPath{};
	const std::wstring m_filePath{ L"Meshes/" };

	std::unique_ptr<CSkinnedData> m_skinnedInfo;
	std::vector<Subset> m_skinnedSubsets;
	std::vector<M3dMaterial> m_skinnedMats;
};