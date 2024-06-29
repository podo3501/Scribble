#pragma once

#include <wrl.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <DirectXCollision.h>

interface IRenderer;
class CGeometry;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct InstanceData;
struct Vertex;
struct RenderItem;

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
	using Vertices = std::vector<Vertex>;
	using Indices = std::vector<std::int32_t>;

	struct MeshData
	{
		std::string name{};

		Vertices vertices{};
		Indices indices{};

		DirectX::BoundingBox boundingBox{};
		DirectX::BoundingSphere boundingSphere{};
	};

	using AllRenderItems = std::unordered_map<std::string, std::unique_ptr<RenderItem>>;
	using Offsets = std::pair<UINT, UINT>;
	using MeshDataList = std::vector<std::unique_ptr<MeshData>>;
	using AllMeshDataList = std::unordered_map<std::string, MeshDataList>;

public:
	template<typename T>
	CModel(T&& resPath)
		: m_resPath(std::forward<T>(resPath))
	{}

	CModel(const CModel&) = delete;
	CModel& operator=(const CModel&) = delete;

	bool LoadGeometryList(const ModelTypeList& modelTypeList);
	bool LoadGraphicMemory(IRenderer* renderer, AllRenderItems* outRenderItems);

private:
	bool LoadGeometry(const ModelType& type);
	bool ReadFile(const std::wstring& filename, MeshData* outData);
	void Generator(MeshData* outData);

	CModel::Offsets SetSubmesh(RenderItem* renderItem, Offsets& offsets, MeshData* data);
	void SetSubmeshList(RenderItem* renderItem, const MeshDataList& meshDataList,
		Vertices& totalVertices, Indices& totalIndices);
	bool Convert(const MeshDataList& meshDataList, Vertices& totalVertices, Indices& totalIndices, RenderItem* renderItem);
	bool Load(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, 
		Vertices& totalVertices, Indices& totalIndices, RenderItem* renderItem);

private:
	//std::string m_name{};
	std::wstring m_resPath{};
	const std::wstring m_filePath{ L"Models/" };

	AllMeshDataList m_AllMeshDataList{};
};
