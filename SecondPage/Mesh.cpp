#include "Mesh.h"
#include <algorithm>
#include <ranges>
#include <fstream>
#include "./Utility.h"
#include "../Include/Interface.h"
#include "../Include/FrameResourceData.h"
#include "../Include/RenderItem.h"
#include "./GeometryGenerator.h"
#include "./SetupData.h"
#include "./MathHelper.h"
#include "./Helper.h"

using namespace DirectX;

CMesh::CMesh(std::wstring resPath)
	: m_resPath(std::move(resPath))
{}

CMesh::~CMesh() = default;

bool CMesh::LoadGeometry(const std::string& geoName, const std::string& meshName, ModelProperty* mProperty)
{
	std::unique_ptr<MeshData> meshData{ nullptr };
	switch (mProperty->createType)
	{
	case ModelProperty::CreateType::Generator:	meshData = std::move(mProperty->meshData);		break;
	case ModelProperty::CreateType::ReadFile:		meshData = ReadFile(mProperty->filename);			break;
	default: return false;
	}
	if (meshData == nullptr) return false;

	meshData->name = meshName;
	m_AllMeshDataList[geoName].emplace_back(std::move(meshData));

	return true;
}

std::unique_ptr<MeshData> CMesh::ReadFile(const std::wstring& filename)
{
	std::unique_ptr<MeshData> meshData = std::make_unique<MeshData>();

	std::wstring fullFilename = m_resPath + m_filePath + filename;
	std::ifstream fin(fullFilename);
	if (fin.fail())
		return nullptr;

	UINT vCount = 0;
	UINT iCount = 0;
	std::string ignore;
	fin >> ignore >> vCount;
	fin >> ignore >> iCount;
	fin >> ignore >> ignore >> ignore >> ignore;

	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

	for (auto i{ 0u }; i < vCount; ++i)
	{
		Vertex curVertex{};
		fin >> curVertex.pos.x >> curVertex.pos.y >> curVertex.pos.z;
		fin >> curVertex.normal.x >> curVertex.normal.y >> curVertex.normal.z;

		XMVECTOR P = XMLoadFloat3(&curVertex.pos);

		XMFLOAT3 spherePos;
		XMStoreFloat3(&spherePos, XMVector3Normalize(P));

		float theta = atan2f(spherePos.z, spherePos.x);

		if (theta < 0.0f) theta += XM_2PI;

		float phi = acosf(spherePos.y);

		float u = theta / (2.0f * XM_PI);
		float v = phi / XM_PI;

		curVertex.texC = { u, v };

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);

		meshData->vertices.emplace_back(std::move(curVertex));
	}

	XMStoreFloat3(&meshData->boundingBox.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&meshData->boundingBox.Extents, 0.5f * (vMax - vMin));

	XMStoreFloat3(&meshData->boundingSphere.Center, 0.5f * (vMin + vMax));
	meshData->boundingSphere.Radius = XMVectorGetX(XMVector3Length(0.5f * (vMax - vMin)));

	fin >> ignore >> ignore >> ignore;

	for (auto iter{ 0u }; iter < iCount * 3; ++iter)
	{
		std::int32_t readIdx{ 0 };
		fin >> readIdx;
		meshData->indices.emplace_back(std::move(readIdx));
	}
	fin.close();

	return std::move(meshData);
}

CMesh::Offsets CMesh::SetSubmesh(RenderItem* renderItem, Offsets& offsets, MeshData* data)
{
	SubRenderItem* subRenderItem = GetSubRenderItem(renderItem, data->name);

	auto& subItem = subRenderItem->subItem;
	subItem.baseVertexLocation = offsets.first;
	subItem.startIndexLocation = offsets.second;
	subItem.boundingBox = data->boundingBox;
	subItem.boundingSphere = data->boundingSphere;
	subItem.indexCount = static_cast<UINT>(data->indices.size());

	return Offsets(
		offsets.first + static_cast<UINT>(data->vertices.size()),
		offsets.second + subItem.indexCount);
}

void CMesh::SetSubmeshList(RenderItem* renderItem, const MeshDataList& meshDataList,
	Vertices& totalVertices, Indices& totalIndices)
{
	Offsets offsets{ 0, 0 };
	std::ranges::for_each(meshDataList, [this, &offsets, renderItem, &totalVertices, &totalIndices](auto& data) {
			offsets = SetSubmesh(renderItem, offsets, data.get());
			std::ranges::copy(data->vertices, std::back_inserter(totalVertices));
			std::ranges::copy(data->indices, std::back_inserter(totalIndices));
		});
}

bool CMesh::Convert(const MeshDataList& meshDataList,
	Vertices& totalVertices, Indices& totalIndices, RenderItem* renderItem)
{
	SetSubmeshList(renderItem, meshDataList, totalVertices, totalIndices);

	UINT vbByteSize = static_cast<UINT>(totalVertices.size()) * sizeof(Vertex);
	UINT ibByteSize = static_cast<UINT>(totalIndices.size()) * sizeof(std::int32_t);

	renderItem->vertexBufferView.SizeInBytes = vbByteSize;
	renderItem->vertexBufferView.StrideInBytes = sizeof(Vertex);

	renderItem->indexBufferView.SizeInBytes = ibByteSize;
	renderItem->indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	return true;
}

bool CMesh::LoadMeshIntoVRAM(IRenderer* renderer, AllRenderItems* outRenderItems)
{
	return std::ranges::all_of(m_AllMeshDataList, [&outRenderItems, renderer, this](auto& iter) {
		auto pRenderItem = GetRenderItem(*outRenderItems, iter.first);

		//데이터를 채워 넣는다.
		std::vector<Vertex> totalVertices{};
		std::vector<std::int32_t> totalIndices{};
		ReturnIfFalse(Convert(iter.second, totalVertices, totalIndices, pRenderItem));

		//그래픽 메모리에 올린다.
		ReturnIfFalse(renderer->LoadMesh(totalVertices, totalIndices, pRenderItem));

		return true;	}); 	
}