#include "./Instance.h"
#include "../Core/Utility.h"
#include "./Material.h"
#include "./RenderItem.h"

using namespace DirectX;

InstanceDataList CreateSkullInstanceData(CMaterial* material)
{
	InstanceDataList instances{};

	const int n = 5;
	const int matCount = static_cast<int>(material->GetCount(TextureType::Texture));
	const int startIndex = material->GetStartIndex(TextureType::Texture);

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;

	float x = -0.5f * width;
	float y = -0.5f * height;
	float z = -0.5f * depth;
	float dx = width / (n - 1);
	float dy = height / (n - 1);
	float dz = depth / (n - 1);

	for (int k = 0; k < n; ++k)
	{
		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				auto instance = std::make_unique<InstanceData>();
				const XMFLOAT3 pos(x + j * dx, y + i * dy, z + k * dz);
				instance->world = XMMATRIX(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					pos.x, pos.y, pos.z, 1.0f);

				int index = k * n * n + i * n + j;
				instance->texTransform = XMMatrixScaling(2.0f, 2.0f, 1.0f);
				instance->matIndex = WrapAround(index + startIndex, startIndex, matCount + startIndex);
				instances.emplace_back(std::move(instance));
			}
		}
	}

	return instances;
}

InstanceDataList CreateSkyCubeInstanceData()
{
	//ÇÏ´Ã¸ÊÀº material°ú texTransformÀ» ¾²Áö ¾Ê°í return gCubeMap.Sample(gsamLinearWrap, pin.PosL);
	InstanceDataList instances{};
	auto instance = std::make_unique<InstanceData>();
	instance->world = XMMatrixIdentity();
	instances.emplace_back(std::move(instance));

	return instances;
}

void CInstance::CreateInstanceData(CMaterial* material, const std::string& geoName, const std::string& meshName)
{
	if (meshName == "skull")
	{
		m_instances[geoName][meshName] = CreateSkullInstanceData(material);
		m_cullingFrustumList[geoName][meshName] = true;
	}
	else if (meshName == "cube")
	{
		m_instances[geoName][meshName] = CreateSkyCubeInstanceData();
		m_cullingFrustumList[geoName][meshName] = false;
	}
}

InstanceDataList CInstance::GetInstanceDummyData(const std::string& geoName, const std::string& meshName)
{
	auto findGeo = m_instances.find(geoName);
	if (findGeo == m_instances.end())
		return {};
	auto findmesh = findGeo->second.find(meshName);
	if (findmesh == findGeo->second.end())
		return {};

	return findmesh->second;
}

bool CInstance::GetCullingFrustum(const std::string& geoName, const std::string& meshName)
{
	return m_cullingFrustumList[geoName][meshName];
}