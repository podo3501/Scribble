#pragma once

#include <wrl.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <DirectXCollision.h>

class CGeometry;
struct Geometry;
struct Vertex;
struct MeshData;

enum class ModelType : int
{
	None,
	Generator,
	ReadFile,
};

class CModel
{
	struct MeshData
	{
		std::string name{};

		std::vector<Vertex> vertices{};
		std::vector<std::int32_t> indices{};

		DirectX::BoundingBox boundingBox{};
		DirectX::BoundingSphere boundingSphere{};
	};

public:
	template<typename T>
	CModel(T&& resPath)
		: m_resPath(std::forward<T>(resPath))
	{}

	CModel(const CModel&) = delete;
	CModel& operator=(const CModel&) = delete;

	bool LoadGeometry(ModelType type, std::string&& geoName, std::string&& subName, std::wstring&& filename = L"");
	bool Convert(CGeometry* geomtry);

private:
	using Offsets = std::pair<UINT, UINT>;

private:
	bool ReadFile(const std::wstring& filename, MeshData* outData);
	void Generator(MeshData* outData);
	void SetSubmeshList(Geometry* geo, const std::vector<std::unique_ptr<MeshData>>& meshDataList,
		std::vector<Vertex>& totalVertices, std::vector<std::int32_t>& totalIndices);
	bool ConvertGeometry(Geometry* geo, const std::vector<std::unique_ptr<MeshData>>& meshDataList);
	Offsets SetSubmesh(Geometry* geo, Offsets& offsets, MeshData* data);

private:
	//std::string m_name{};
	std::wstring m_resPath{};
	const std::wstring m_filePath{ L"Models/" };

	std::unordered_map<std::string, std::vector<std::unique_ptr<MeshData>>> m_meshDataList;
};
