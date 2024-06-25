#include "Model.h"
#include <DirectXMath.h>
#include "../Core/d3dUtil.h"
#include "./FrameResource.h"
#include "./Geometry.h"
#include "./GeometryGenerator.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

bool CModel::LoadGeometry(ModelType type, std::string&& geoName, std::string&& subName, std::wstring&& filename)
{
	auto meshData = std::make_unique<MeshData>();

	auto result = true;
	switch (type)
	{
	case ModelType::Generator:		Generator(meshData.get());									break;
	case ModelType::ReadFile:			result = ReadFile(filename, meshData.get());		break;
	default: return false;
	}
	if (!result) return result;

	meshData->name = std::move(subName);
	m_meshDataList[geoName].emplace_back(std::move(meshData));

	return true;
}

void CModel::Generator(MeshData* outData)
{
	CGeometryGenerator geoGen{};
	CGeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
	
	std::transform(box.Vertices.begin(), box.Vertices.end(), std::back_inserter(outData->vertices),
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

CModel::Offsets CModel::SetSubmesh(Geometry* geo, Offsets& offsets, MeshData* data)
{
	UINT indexCount = static_cast<UINT>(data->indices.size());

	SubmeshGeometry submesh{};
	submesh.baseVertexLocation = offsets.first;
	submesh.startIndexLocation = offsets.second;
	submesh.boundingBox = data->boundingBox;
	submesh.boundingSphere = data->boundingSphere;
	submesh.indexCount = indexCount;

	geo->drawArgs.insert(std::make_pair(data->name, std::move(submesh)));

	return Offsets(
		offsets.first + static_cast<UINT>(data->vertices.size()),
		offsets.second + indexCount);
}

void CModel::SetSubmeshList(Geometry* geo, const std::vector<std::unique_ptr<MeshData>>& meshDataList, 
	std::vector<Vertex>& totalVertices, std::vector<std::int32_t>& totalIndices)
{
	Offsets offsets{ 0, 0 };
	for_each(meshDataList.begin(), meshDataList.end(),
		[model = this, &offsets, geo, &totalVertices, &totalIndices](auto& data) { 
			offsets = model->SetSubmesh(geo, offsets, data.get());
			std::copy(data->vertices.begin(), data->vertices.end(), std::back_inserter(totalVertices));
			std::copy(data->indices.begin(), data->indices.end(), std::back_inserter(totalIndices)); 
		});
}

bool CModel::ConvertGeometry(Geometry* geo, const std::vector<std::unique_ptr<MeshData>>& meshDataList)
{
	std::vector<Vertex> totalVertices{};
	std::vector<std::int32_t> totalIndices{};
	SetSubmeshList(geo, meshDataList, totalVertices, totalIndices);

	UINT vbByteSize = static_cast<UINT>(totalVertices.size()) * sizeof(Vertex);
	UINT ibByteSize = static_cast<UINT>(totalIndices.size()) * sizeof(std::int32_t);

	geo->vertexBufferByteSize = vbByteSize;
	geo->vertexByteStride = sizeof(Vertex);

	geo->indexBufferByteSize = ibByteSize;
	geo->indexFormat = DXGI_FORMAT_R32_UINT;

	auto& vertexBuffer = geo->vertexBufferCPU;
	auto& indexBuffer = geo->indexBufferCPU;

	ReturnIfFailed(D3DCreateBlob(vbByteSize, &vertexBuffer));
	CopyMemory(vertexBuffer->GetBufferPointer(), totalVertices.data(), vbByteSize);

	ReturnIfFailed(D3DCreateBlob(ibByteSize, &indexBuffer));
	CopyMemory(indexBuffer->GetBufferPointer(), totalIndices.data(), ibByteSize);

	return true;
}

bool CModel::Convert(CGeometry* geometry)
{
	return std::all_of(m_meshDataList.begin(), m_meshDataList.end(),
		[model = this, geometry](auto& iter) {
			auto geo = geometry->GetGeometry(iter.first);
			return model->ConvertGeometry(geo, iter.second);
		});
}
