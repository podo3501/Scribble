#include "Model.h"
#include <DirectXMath.h>
#include "../Core/d3dUtil.h"
#include "./FrameResource.h"
#include "./Geometry.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

bool CModel::LoadGeometry(ModelType type, std::string&& name, std::wstring&& filename)
{
	std::wstring fullFilename = m_resPath + m_filePath + filename;
	std::ifstream fin(fullFilename);
	if (fin.bad())
		return false;

	auto meshData = std::make_unique<MeshData>();
	switch (type)
	{
	case ModelType::Common:		ReadCommon(fin, meshData.get());			break;
	default: return false;
	}
	fin.close();

	meshData->name = std::move(name);
	m_meshDataList.emplace_back(std::move(meshData));

	return true;
}

void CModel::ReadCommon(std::ifstream& fin, MeshData* outData)
{
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

void CModel::SetSubmeshList(Geometry* geo, std::vector<Vertex>& totalVertices, std::vector<std::int32_t>& totalIndices)
{
	Offsets offsets{ 0, 0 };
	for_each(m_meshDataList.begin(), m_meshDataList.end(),
		[model = this, &offsets, geo, &totalVertices, &totalIndices](auto& data) { 
			offsets = model->SetSubmesh(geo, offsets, data.get());
			std::copy(data->vertices.begin(), data->vertices.end(), std::back_inserter(totalVertices));
			std::copy(data->indices.begin(), data->indices.end(), std::back_inserter(totalIndices)); 
		});
}

bool CModel::Convert(CGeometry* geomtry)
{
	auto geo = geomtry->GetGeometry();
	std::vector<Vertex> totalVertices{};
	std::vector<std::int32_t> totalIndices{};
	SetSubmeshList(geo, totalVertices, totalIndices);

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

//bool CModel::AddData(std::string& meshName, const MeshData& meshData)
//{
//	auto meshGeo = std::make_unique<Geometry>();
//
//	//SubmeshGeometry 값을 다 채운다음 move로 옮기자
//	SubmeshGeometry submesh;
//	//auto& submesh = meshGeo->drawArgs[m_submeshName];
//	submesh.indexCount = static_cast<UINT>(meshData.indices.size());
//	submesh.startIndexLocation = 0;
//	submesh.baseVertexLocation = 0;
//	submesh.boundingBox = meshData.boundingBox;
//	submesh.boundingSphere = meshData.boundingSphere;
//
//	m_submeshes.insert(std::make_pair(name, submesh));
//	/*meshGeo->drawArgs.insert(std::make_pair())
//
//	ReturnIfFalse(geometry->SetMesh(std::move(meshGeo)));*/
//
//	return true;
//}