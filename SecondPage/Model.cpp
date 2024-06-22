#include "Model.h"
#include <DirectXMath.h>
#include "../Core/d3dUtil.h"
#include "./FrameResource.h"
#include "./Geometry.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

bool CModel::Read()
{
	std::wstring filename = m_resPath + m_filePath + L"skull.txt";
	std::ifstream fin(filename);
	if (fin.bad())
	{
		MessageBox(0, filename.c_str(), 0, 0);
		return false;
	}

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
		Vertex curVertex;
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

		m_vertices.emplace_back(std::move(curVertex));
	}

	XMStoreFloat3(&m_boundingBox.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&m_boundingBox.Extents, 0.5f * (vMax - vMin));

	XMStoreFloat3(&m_boundingSphere.Center, 0.5f * (vMin + vMax));
	m_boundingSphere.Radius = XMVectorGetX(XMVector3Length(0.5f * (vMax - vMin)));

	fin >> ignore >> ignore >> ignore;
	
	std::int32_t readIdx{ 0 };
	for (auto iter{ 0u }; iter < iCount * 3; ++iter)
	{
		fin >> readIdx;
		m_indices.emplace_back(readIdx);
	}
	
	fin.close();

	return true;
}

bool CModel::Convert(CGeometry* geometry)
{
	UINT vbByteSize = static_cast<UINT>(m_vertices.size()) * sizeof(Vertex);
	UINT ibByteSize = static_cast<UINT>(m_indices.size()) * sizeof(std::int32_t);

	auto meshGeo = std::make_unique<Geometry>();
	meshGeo->name = m_name;

	auto& submesh = meshGeo->drawArgs[m_submeshName];
	submesh.indexCount = static_cast<UINT>(m_indices.size());
	submesh.startIndexLocation = 0;
	submesh.baseVertexLocation = 0;
	submesh.boundingBox = m_boundingBox;
	submesh.boundingSphere = m_boundingSphere;

	meshGeo->vertexBufferByteSize = vbByteSize;
	meshGeo->vertexByteStride = sizeof(Vertex);

	ReturnIfFailed(D3DCreateBlob(vbByteSize, &meshGeo->vertexBufferCPU));
	CopyMemory(meshGeo->vertexBufferCPU->GetBufferPointer(), m_vertices.data(), vbByteSize);

	ReturnIfFailed(D3DCreateBlob(ibByteSize, &meshGeo->indexBufferCPU));
	CopyMemory(meshGeo->indexBufferCPU->GetBufferPointer(), m_indices.data(), ibByteSize);

	meshGeo->indexFormat = DXGI_FORMAT_R32_UINT;
	meshGeo->indexBufferByteSize = ibByteSize;

	ReturnIfFalse(geometry->SetMesh(std::move(meshGeo)));

	return true;
}