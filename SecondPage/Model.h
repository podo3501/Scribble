#pragma once

#include <wrl.h>
#include <vector>
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
	Common,
	Cube,
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
	//CModel(std::wstring& resPath)
	//	: m_resPath(resPath)
	//{}

	CModel(const CModel&) = delete;
	CModel& operator=(const CModel&) = delete;

	bool LoadGeometry(ModelType type, std::string&& name, std::wstring&& filename);
	bool Convert(CGeometry* geomtry);

	//inline std::string GetName() { return m_name; };
	//inline std::string GetSubmeshName() { return m_submeshName; };

private:
	using Offsets = std::pair<UINT, UINT>;

private:
	void ReadCommon(std::ifstream& fin, MeshData* outData);
	void SetSubmeshList(Geometry* geo, std::vector<Vertex>& totalVertices, std::vector<std::int32_t>& totalIndices);
	Offsets SetSubmesh(Geometry* geo, Offsets& offsets, MeshData* data);
	//bool AddData(std::string& meshName, const MeshData& meshData);

private:
	//std::string m_name{};
	std::wstring m_resPath{};
	const std::wstring m_filePath{ L"Models/" };

	std::vector<std::unique_ptr<MeshData>> m_meshDataList;
	
	//std::string m_submeshName{};
	//std::vector<Vertex> m_vertices{};
	//std::vector<std::int32_t> m_indices{};
	//DirectX::BoundingBox m_boundingBox{};
};
