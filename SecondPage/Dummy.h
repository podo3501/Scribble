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

class CDummy
{
public:
	CDummy() = default;

	CDummy(const CDummy&) = delete;
	CDummy& operator=(const CDummy&) = delete;

	void CreateInstanceData(CMaterial* material, const std::string& geoName, const std::string& meshName);
	InstanceDataList GetInstanceDummyData(const std::string& geoName, const std::string& meshName);

private:
	std::unordered_map<std::string, std::unordered_map<std::string, InstanceDataList>> m_instances{};
};

