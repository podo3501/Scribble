#pragma once

#include <wrl.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <DirectXMath.h>

class CMaterial;
struct InstanceData;

using InstanceDataList = std::vector<std::shared_ptr<InstanceData>>;

class CInstance
{
public:
	CInstance() = default;

	CInstance(const CInstance&) = delete;
	CInstance& operator=(const CInstance&) = delete;

	void CreateInstanceData(CMaterial* material, const std::string& geoName, const std::string& meshName);
	InstanceDataList GetInstanceDummyData(const std::string& geoName, const std::string& meshName);
	bool GetCullingFrustum(const std::string& geoName, const std::string& meshName);

private:
	std::unordered_map<std::string, std::unordered_map<std::string, InstanceDataList>> m_instances{};
	std::unordered_map<std::string, std::unordered_map<std::string, bool>> m_cullingFrustumList{};
};

