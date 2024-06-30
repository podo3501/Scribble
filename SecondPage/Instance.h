#pragma once

#include <wrl.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <DirectXMath.h>

class CMaterial;
struct RenderItem;
struct InstanceData;
struct InstanceInfo;

using InstanceDataList = std::vector<std::shared_ptr<InstanceData>>;

class CInstance
{
	using InstanceInfos = std::unordered_map<std::string, InstanceInfo>;
	using AllInstances = std::unordered_map<std::string, InstanceInfos>;

public:
	CInstance();
	~CInstance();

	CInstance(const CInstance&) = delete;
	CInstance& operator=(const CInstance&) = delete;

	void CreateInstanceData(CMaterial* material, const std::string& geoName, const std::string& meshName);
	bool FillRenderItems(std::unordered_map<std::string, std::unique_ptr<RenderItem>>& renderItems);

private:
	AllInstances m_instances{};
};

