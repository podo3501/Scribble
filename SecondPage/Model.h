#pragma once

#include <wrl.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <DirectXCollision.h>

class CGeometry;
class CDirectx3D;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct InstanceData;
struct Geometry;
struct Vertex;
struct MeshData;
struct NRenderItem;

enum class CreateType : int
{
	None,
	Generator,
	ReadFile,
};

struct ModelType
{
	ModelType(CreateType _type, 
		const std::string& _geoName, 
		const std::string& _submeshName, 
		const std::wstring& _filename = L"")
		: createType{ _type }
		, geometryName{ _geoName }
		, submeshName{ _submeshName }
		, filename{ _filename }
	{}
	ModelType& operator=(const ModelType&) = delete;

	CreateType createType{ CreateType::None };
	std::string geometryName{};
	std::string submeshName{};
	std::wstring filename{};
};

using ModelTypeList = std::vector<ModelType>;

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

	bool LoadGeometryList(const ModelTypeList& modelTypeList);
	bool Convert(CGeometry* geomtry);
	bool LoadGraphicMemory(CDirectx3D* directx3D,
		std::unordered_map<std::string, std::unique_ptr<NRenderItem>>* outRenderItems);

private:
	using Offsets = std::pair<UINT, UINT>;
	using MeshDataList = std::vector<std::unique_ptr<MeshData>>;

private:
	bool LoadGeometry(const ModelType& type);
	bool ReadFile(const std::wstring& filename, MeshData* outData);
	void Generator(MeshData* outData);
	void SetSubmeshList(Geometry* geo, const MeshDataList& meshDataList,
		std::vector<Vertex>& totalVertices, std::vector<std::int32_t>& totalIndices);
	bool ConvertGeometry(Geometry* geo, const MeshDataList& meshDataList);
	Offsets SetSubmesh(Geometry* geo, Offsets& offsets, MeshData* data);

	CModel::Offsets SetSubmesh(NRenderItem* renderItem, Offsets& offsets, MeshData* data);
	void SetSubmeshList(NRenderItem* renderItem, const MeshDataList& meshDataList,
		std::vector<Vertex>& totalVertices, std::vector<std::int32_t>& totalIndices);
	bool Convert(const MeshDataList& meshDataList,
		std::vector<Vertex>& totalVertices, std::vector<std::int32_t>& totalIndices, NRenderItem* renderItem);
	bool Load(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
		std::vector<Vertex>& totalVertices, std::vector<std::int32_t>& totalIndices, NRenderItem* renderItem);

private:
	//std::string m_name{};
	std::wstring m_resPath{};
	const std::wstring m_filePath{ L"Models/" };

	std::unordered_map<std::string, MeshDataList> m_meshDataList;
};
