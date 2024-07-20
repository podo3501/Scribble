#include "LoadM3D.h"
#include "SkinnedData.h"
#include <fstream>
#include "../Include/FrameResourceData.h"
#include "./SkinnedData.h"

using namespace DirectX;

bool CLoadM3D::Read(const std::wstring& filename, 
						std::vector<Vertex>& vertices,
						std::vector<std::int32_t>& indices,
						std::vector<Subset>& subsets,
						std::vector<M3dMaterial>& mats)
{
	std::ifstream fin(filename);

	UINT numMaterials = 0;
	UINT numVertices  = 0;
	UINT numTriangles = 0;
	UINT numBones     = 0;
	UINT numAnimationClips = 0;

	std::string ignore;

	if( fin )
	{
		fin >> ignore; // file header text
		fin >> ignore >> numMaterials;
		fin >> ignore >> numVertices;
		fin >> ignore >> numTriangles;
		fin >> ignore >> numBones;
		fin >> ignore >> numAnimationClips;
 
		ReadMaterials(fin, numMaterials, mats);
		ReadSubsetTable(fin, numMaterials, subsets);
	    ReadVertices(fin, numVertices, vertices);
	    ReadTriangles(fin, numTriangles, indices);
 
		return true;
	 }
    return false;
}

bool CLoadM3D::Read(const std::wstring& filename,
						std::vector<SkinnedVertex>& vertices,
						std::vector<std::int32_t>& indices,
						std::vector<Subset>& subsets,
						std::vector<M3dMaterial>& mats,
						CSkinnedData* skinInfo)
{
    std::ifstream fin(filename);

	UINT numMaterials = 0;
	UINT numVertices  = 0;
	UINT numTriangles = 0;
	UINT numBones     = 0;
	UINT numAnimationClips = 0;

	std::string ignore;

	if( fin )
	{
		fin >> ignore; // file header text
		fin >> ignore >> numMaterials;
		fin >> ignore >> numVertices;
		fin >> ignore >> numTriangles;
		fin >> ignore >> numBones;
		fin >> ignore >> numAnimationClips;
 
		std::vector<XMFLOAT4X4> boneOffsets;
		std::vector<int> boneIndexToParentIndex;
		std::unordered_map<std::string, AnimationClip> animations;

		ReadMaterials(fin, numMaterials, mats);
		ReadSubsetTable(fin, numMaterials, subsets);
	    ReadSkinnedVertices(fin, numVertices, vertices);
	    ReadTriangles(fin, numTriangles, indices);
		ReadBoneOffsets(fin, numBones, boneOffsets);
	    ReadBoneHierarchy(fin, numBones, boneIndexToParentIndex);
	    ReadAnimationClips(fin, numBones, numAnimationClips, animations);
 
		skinInfo->Set(boneIndexToParentIndex, boneOffsets, animations);

	    return true;
	}
    return false;
}

void CLoadM3D::ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<M3dMaterial>& mats)
{
	 std::string ignore;
     mats.resize(numMaterials);

	 std::string diffuseMapName;
	 std::string normalMapName;

     fin >> ignore; // materials header text
	 for(UINT i = 0; i < numMaterials; ++i)
	 {
         fin >> ignore >> mats[i].name;
		 fin >> ignore >> mats[i].diffuseAlbedo.x  >> mats[i].diffuseAlbedo.y  >> mats[i].diffuseAlbedo.z;
		 fin >> ignore >> mats[i].fresnelR0.x >> mats[i].fresnelR0.y >> mats[i].fresnelR0.z;
         fin >> ignore >> mats[i].roughness;
		 fin >> ignore >> mats[i].alphaClip;
		 fin >> ignore >> mats[i].materialTypeName;
		 fin >> ignore >> mats[i].diffuseMapName;
		 fin >> ignore >> mats[i].normalMapName;
		}
}

void CLoadM3D::ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<Subset>& subsets)
{
    std::string ignore;
	subsets.resize(numSubsets);

	fin >> ignore; // subset header text
	for(UINT i = 0; i < numSubsets; ++i)
	{
        fin >> ignore >> subsets[i].id;
		fin >> ignore >> subsets[i].vertexStart;
		fin >> ignore >> subsets[i].vertexCount;
		fin >> ignore >> subsets[i].faceStart;
		fin >> ignore >> subsets[i].faceCount;
    }
}

void CLoadM3D::ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex>& vertices)
{
	std::string ignore;
    vertices.resize(numVertices);

    fin >> ignore; // vertices header text
    for(UINT i = 0; i < numVertices; ++i)
    {
	    fin >> ignore >> vertices[i].pos.x      >> vertices[i].pos.y      >> vertices[i].pos.z;
		fin >> ignore >> vertices[i].tangentU.x >> vertices[i].tangentU.y >> vertices[i].tangentU.z;// >> vertices[i].tangentU.w;
	    fin >> ignore >> vertices[i].normal.x   >> vertices[i].normal.y   >> vertices[i].normal.z;
	    fin >> ignore >> vertices[i].texC.x     >> vertices[i].texC.y;
    }
}

void CLoadM3D::ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<SkinnedVertex>& vertices)
{
	std::string ignore;
    vertices.resize(numVertices);

    fin >> ignore; // vertices header text
	int boneIndices[4];
	float weights[4];
    for(UINT i = 0; i < numVertices; ++i)
    {
        float blah;
	    fin >> ignore >> vertices[i].Pos.x        >> vertices[i].Pos.y          >> vertices[i].Pos.z;
		fin >> ignore >> vertices[i].TangentU.x   >> vertices[i].TangentU.y     >> vertices[i].TangentU.z >> blah /*vertices[i].TangentU.w*/;
	    fin >> ignore >> vertices[i].Normal.x     >> vertices[i].Normal.y       >> vertices[i].Normal.z;
	    fin >> ignore >> vertices[i].TexC.x       >> vertices[i].TexC.y;
		fin >> ignore >> weights[0]     >> weights[1]     >> weights[2]     >> weights[3];
		fin >> ignore >> boneIndices[0] >> boneIndices[1] >> boneIndices[2] >> boneIndices[3];

		vertices[i].BoneWeights.x = weights[0];
		vertices[i].BoneWeights.y = weights[1];
		vertices[i].BoneWeights.z = weights[2];

		vertices[i].BoneIndices[0] = (BYTE)boneIndices[0]; 
		vertices[i].BoneIndices[1] = (BYTE)boneIndices[1]; 
		vertices[i].BoneIndices[2] = (BYTE)boneIndices[2]; 
		vertices[i].BoneIndices[3] = (BYTE)boneIndices[3]; 
    }
}

void CLoadM3D::ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<std::int32_t>& indices)
{
	std::string ignore;
    indices.resize(numTriangles*3);

    fin >> ignore; // triangles header text
    for(UINT i = 0; i < numTriangles; ++i)
    {
        fin >> indices[i*3+0] >> indices[i*3+1] >> indices[i*3+2];
    }
}
 
void CLoadM3D::ReadBoneOffsets(std::ifstream& fin, UINT numBones, std::vector<XMFLOAT4X4>& boneOffsets)
{
	std::string ignore;
    boneOffsets.resize(numBones);

    fin >> ignore; // BoneOffsets header text
    for(UINT i = 0; i < numBones; ++i)
    {
        fin >> ignore >> 
            boneOffsets[i](0,0) >> boneOffsets[i](0,1) >> boneOffsets[i](0,2) >> boneOffsets[i](0,3) >>
            boneOffsets[i](1,0) >> boneOffsets[i](1,1) >> boneOffsets[i](1,2) >> boneOffsets[i](1,3) >>
            boneOffsets[i](2,0) >> boneOffsets[i](2,1) >> boneOffsets[i](2,2) >> boneOffsets[i](2,3) >>
            boneOffsets[i](3,0) >> boneOffsets[i](3,1) >> boneOffsets[i](3,2) >> boneOffsets[i](3,3);
    }
}

void CLoadM3D::ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex)
{
	std::string ignore;
    boneIndexToParentIndex.resize(numBones);

    fin >> ignore; // BoneHierarchy header text
	for(UINT i = 0; i < numBones; ++i)
	{
	    fin >> ignore >> boneIndexToParentIndex[i];
	}
}

void CLoadM3D::ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips, 
								   std::unordered_map<std::string, AnimationClip>& animations)
{
	std::string ignore;
    fin >> ignore; // AnimationClips header text
    for(UINT clipIndex = 0; clipIndex < numAnimationClips; ++clipIndex)
    {
        std::string clipName;
        fin >> ignore >> clipName;
        fin >> ignore; // {

		AnimationClip clip;
		clip.BoneAnimations.resize(numBones);

        for(UINT boneIndex = 0; boneIndex < numBones; ++boneIndex)
        {
            ReadBoneKeyframes(fin, numBones, clip.BoneAnimations[boneIndex]);
        }
        fin >> ignore; // }

        animations[clipName] = clip;
    }
}

void CLoadM3D::ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation)
{
	std::string ignore;
    UINT numKeyframes = 0;
    fin >> ignore >> ignore >> numKeyframes;
    fin >> ignore; // {

    boneAnimation.Keyframes.resize(numKeyframes);
    for(UINT i = 0; i < numKeyframes; ++i)
    {
        float t    = 0.0f;
        XMFLOAT3 p(0.0f, 0.0f, 0.0f);
        XMFLOAT3 s(1.0f, 1.0f, 1.0f);
        XMFLOAT4 q(0.0f, 0.0f, 0.0f, 1.0f);
        fin >> ignore >> t;
        fin >> ignore >> p.x >> p.y >> p.z;
        fin >> ignore >> s.x >> s.y >> s.z;
        fin >> ignore >> q.x >> q.y >> q.z >> q.w;

	    boneAnimation.Keyframes[i].TimePos      = t;
        boneAnimation.Keyframes[i].Translation  = p;
	    boneAnimation.Keyframes[i].Scale        = s;
	    boneAnimation.Keyframes[i].RotationQuat = q;
    }

    fin >> ignore; // }
}