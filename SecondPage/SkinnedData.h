#pragma once
#include <DirectXMath.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <wrl.h>

class CSkinnedData;

struct Subset
{
	UINT id = -1;
	UINT vertexStart{ 0u };
	UINT vertexCount{ 0u };
	UINT faceStart{ 0u };
	UINT faceCount{ 0u };
};

struct M3dMaterial
{
	std::string name;

	DirectX::XMFLOAT4 diffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 fresnelR0 = { 0.01f, 0.01f, 0.01f };
	float roughness{ 0.8f };
	bool alphaClip{ false };

	std::string materialTypeName{};
	std::string diffuseMapName{};
	std::string normalMapName{};
};

struct Keyframe
{
	Keyframe();
	~Keyframe();

	float TimePos{ 0.0f };
	DirectX::XMFLOAT3 Translation{};
	DirectX::XMFLOAT3 Scale{};
	DirectX::XMFLOAT4 RotationQuat{};
};

struct BoneAnimation
{
	float GetStartTime() const;
	float GetEndTime() const;

	void Interpolate(float t, DirectX::XMFLOAT4X4& M) const;

	std::vector<Keyframe> Keyframes;
};

struct AnimationClip
{
	float GetClipStartTime() const;
	float GetClipEndTime() const;

	void Interpolate(float t, std::vector<DirectX::XMFLOAT4X4>& boneTransforms) const;

	std::vector<BoneAnimation> BoneAnimations;
};

struct SkinnedModelInstance
{
	CSkinnedData* skinnedInfo = nullptr;
	std::vector<DirectX::XMFLOAT4X4> finalTransforms;
	std::string clipName;
	float timePos{ 0.0f };

	void UpdateSkinnedAnimation(float dt);
};

class CSkinnedData
{
public:
	UINT BoneCount() const;

	float GetClipStartTime(const std::string& clipName) const;
	float GetClipEndTime(const std::string& clipName) const;

	void Set(
		std::vector<int>& boneHierarchy,
		std::vector<DirectX::XMFLOAT4X4>& boneOffsets,
		std::unordered_map<std::string, AnimationClip>& animations);

	void GetFinalTransforms(const std::string& clipName, float timePos,
		std::vector<DirectX::XMFLOAT4X4>& finalTransforms) const;

private:
	std::vector<int> mBoneHierarchy;
	std::vector<DirectX::XMFLOAT4X4> mBoneOffsets;
	std::unordered_map<std::string, AnimationClip> mAnimations;
};