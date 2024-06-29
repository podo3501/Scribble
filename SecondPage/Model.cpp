#include "Model.h"
#include <DirectXMath.h>
#include "../Core/d3dUtil.h"
#include "../Core/Directx3D.h"
#include "./FrameResourceData.h"
#include "./GeometryGenerator.h"
#include "./RenderItem.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

bool CModel::LoadGeometryList(const ModelTypeList& modelTypeList)
{
	return std::ranges::all_of(modelTypeList, [this](auto& modelType) {
		return LoadGeometry(modelType); });
}

bool CModel::LoadGeometry(const ModelType& type)
{
	auto meshData = std::make_unique<MeshData>();

	auto result = true;
	switch (type.createType)
	{
	case CreateType::Generator:		Generator(meshData.get());											break;
	case CreateType::ReadFile:			result = ReadFile(type.filename, meshData.get());		break;
	default: return false;
	}
	if (!result) return result;

	meshData->name = type.submeshName;
	m_AllMeshDataList[type.geometryName].emplace_back(std::move(meshData));

	return true;
}

void CModel::Generator(MeshData* outData)
{
	CGeometryGenerator geoGen{};
	CGeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
	
	std::ranges::transform(box.Vertices, std::back_inserter(outData->vertices),
		[](auto& gen) { return Vertex(gen.Position, gen.Normal, gen.TexC); });
	outData->indices.insert(outData->indices.end(), box.Indices32.begin(), box.Indices32.end());
}

bool CModel::ReadFile(const std::wstring& filename, MeshData* outData)
{
	std::wstring fullFilename = m_resPath + m_filePath + filename;
	std::ifstream fin(fullFilename);
	if (fin.bad())
		return false;

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

		outData->vertices.emplace_back(std::move(curVertex));
	}

	XMStoreFloat3(&outData->boundingBox.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&outData->boundingBox.Extents, 0.5f * (vMax - vMin));

	XMStoreFloat3(&outData->boundingSphere.Center, 0.5f * (vMin + vMax));
	outData->boundingSphere.Radius = XMVectorGetX(XMVector3Length(0.5f * (vMax - vMin)));

	fin >> ignore >> ignore >> ignore;

	for (auto iter{ 0u }; iter < iCount * 3; ++iter)
	{
		std::int32_t readIdx{ 0 };
		fin >> readIdx;
		outData->indices.emplace_back(std::move(readIdx));
	}
	fin.close();

	return true; 
}

CModel::Offsets CModel::SetSubmesh(RenderItem* renderItem, Offsets& offsets, MeshData* data)
{
	UINT indexCount = static_cast<UINT>(data->indices.size());

	SubItem subItem{};
	subItem.baseVertexLocation = offsets.first;
	subItem.startIndexLocation = offsets.second;
	subItem.boundingBox = data->boundingBox;
	subItem.boundingSphere = data->boundingSphere;
	subItem.indexCount = indexCount;

	SubRenderItem subRenderItem{};
	subRenderItem.subItem = subItem;

	renderItem->subRenderItems.insert(std::make_pair(data->name, std::move(subRenderItem)));

	return Offsets(
		offsets.first + static_cast<UINT>(data->vertices.size()),
		offsets.second + indexCount);
}

void CModel::SetSubmeshList(RenderItem* renderItem, const MeshDataList& meshDataList,
	Vertices& totalVertices, Indices& totalIndices)
{
	Offsets offsets{ 0, 0 };
	std::ranges::for_each(meshDataList,[this, &offsets, renderItem, &totalVertices, &totalIndices](auto& data) {
			offsets = SetSubmesh(renderItem, offsets, data.get());
			std::ranges::copy(data->vertices, std::back_inserter(totalVertices));
			std::ranges::copy(data->indices, std::back_inserter(totalIndices));
		});
}

bool CModel::Convert(const MeshDataList& meshDataList,
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

bool CModel::LoadGraphicMemory(CDirectx3D* directx3D, AllRenderItems* outRenderItems)
{
	return std::ranges::all_of(m_AllMeshDataList, [&outRenderItems, directx3D, this](auto& iter) {
			auto renderItem = std::make_unique<RenderItem>();
			auto pRenderItem = renderItem.get();
			(*outRenderItems).insert(std::make_pair(iter.first, std::move(renderItem)));

			//데이터를 채워 넣는다.
			std::vector<Vertex> totalVertices{};
			std::vector<std::int32_t> totalIndices{};
			ReturnIfFalse(Convert(iter.second, totalVertices, totalIndices, pRenderItem));

			//그래픽 메모리에 올린다.
			ReturnIfFalse(directx3D->LoadData(
				[&, this](ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)->bool {
					 return Load(device, cmdList, totalVertices, totalIndices, pRenderItem);	}));

			return true;
		}); 	
}

bool CModel::Load(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, 
	Vertices& totalVertices, Indices& totalIndices, RenderItem* renderItem)
{
	ReturnIfFalse(CoreUtil::CreateDefaultBuffer(
		device, cmdList,
		totalVertices.data(),
		renderItem->vertexBufferView.SizeInBytes,
		renderItem->vertexBufferUploader,
		&renderItem->vertexBufferGPU));
	renderItem->vertexBufferView.BufferLocation = renderItem->vertexBufferGPU->GetGPUVirtualAddress();

	ReturnIfFalse(CoreUtil::CreateDefaultBuffer(
		device, cmdList,
		totalIndices.data(),
		renderItem->indexBufferView.SizeInBytes,
		renderItem->indexBufferUploader,
		&renderItem->indexBufferGPU));
	renderItem->indexBufferView.BufferLocation = renderItem->indexBufferGPU->GetGPUVirtualAddress();

	return true;
}
