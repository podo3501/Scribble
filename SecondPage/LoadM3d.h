#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <wrl.h>
#include <DirectXMath.h>

class CSkinnedData;
struct Vertex;
struct Subset;
struct M3dMaterial;
struct SkinnedVertex;
struct AnimationClip;
struct BoneAnimation;

class CLoadM3D
{
public:
	bool Read(const std::wstring& filename, 
		std::vector<Vertex>& vertices,
		std::vector<std::int32_t>& indices,
		std::vector<Subset>& subsets,
		std::vector<M3dMaterial>& mats);
	bool Read(const std::wstring& filename,
		std::vector<SkinnedVertex>& vertices,
		std::vector<std::int32_t>& indices,
		std::vector<Subset>& subsets,
		std::vector<M3dMaterial>& mats,
		CSkinnedData* skinInfo);

private:
	void ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<M3dMaterial>& mats);
	void ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<Subset>& subsets);
	void ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex>& vertices);
	void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<SkinnedVertex>& vertices);
	void ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<std::int32_t>& indices);
	void ReadBoneOffsets(std::ifstream& fin, UINT numBones, std::vector<DirectX::XMFLOAT4X4>& boneOffsets);
	void ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex);
	void ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips, std::unordered_map<std::string, AnimationClip>& animations);
	void ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation);
};
