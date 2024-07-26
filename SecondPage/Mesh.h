#pragma once

interface IRenderer;
class CGeometry;
struct InstanceData;
struct Vertex;
struct RenderItem;
struct ModelProperty;
enum class GraphicsPSO : int;

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

class CMesh
{
	using AllRenderItems = std::map<GraphicsPSO, std::unique_ptr<RenderItem>>;
	using Offsets = std::pair<UINT, UINT>;
	using MeshDataList = std::vector<std::unique_ptr<MeshData>>;
	using AllMeshDataList = std::map<GraphicsPSO, MeshDataList>;

public:
	CMesh(std::wstring resPath);
	~CMesh();

	CMesh() = delete;
	CMesh(const CMesh&) = delete;
	CMesh& operator=(const CMesh&) = delete;

	bool LoadGeometry(GraphicsPSO pso, const std::string& meshName, ModelProperty* mProperty);
	bool LoadMeshIntoVRAM(IRenderer* renderer, AllRenderItems* outRenderItems);

private:
	std::unique_ptr<MeshData> ReadMeshType(GraphicsPSO pso, const std::wstring& filename);
	bool ReadFile(const std::wstring& filename, MeshData** outMeshData);
	bool ReadSkinnedFile(const std::wstring& filename, MeshData** outMeshData);

	CMesh::Offsets SetSubmesh(RenderItem* renderItem, Offsets& offsets, MeshData* data);
	void SetSubmeshList(RenderItem* renderItem, const MeshDataList& meshDataList,
		Vertices& totalVertices, Indices& totalIndices);
	bool Convert(const MeshDataList& meshDataList, Vertices& totalVertices, Indices& totalIndices, RenderItem* renderItem);

private:
	std::wstring m_resPath{};
	const std::wstring m_filePath{ L"Meshes/" };

	AllMeshDataList m_AllMeshDataList;
};
