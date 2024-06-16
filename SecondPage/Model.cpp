#include "Model.h"
#include "../Core/d3dUtil.h"
#include <DirectXMath.h>
#include "FrameResource.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

CModel::CModel()
{}

bool CModel::Read(MeshGeometry* meshGeo)
{
	std::ifstream fin("../Resource/Models/skull.txt");
	if (fin.bad())
	{
		MessageBox(0, L"Models/Skull.txt not found", 0, 0);
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

	std::vector<Vertex> vertices(vCount);
	for (auto i{ 0u }; i < vCount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;

		XMVECTOR P = XMLoadFloat3(&vertices[i].Pos);

		XMFLOAT3 spherePos;
		XMStoreFloat3(&spherePos, XMVector3Normalize(P));

		float theta = atan2f(spherePos.z, spherePos.x);

		if (theta < 0.0f) theta += XM_2PI;

		float phi = acosf(spherePos.y);

		float u = theta / (2.0f * XM_PI);
		float v = phi / XM_PI;

		vertices[i].TexC = { u, v };

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	BoundingBox boundingBoxBounds;
	XMStoreFloat3(&boundingBoxBounds.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&boundingBoxBounds.Extents, 0.5f * (vMax - vMin));

	BoundingSphere boundingSphere;
	XMStoreFloat3(&boundingSphere.Center, 0.5f * (vMin + vMax));
	boundingSphere.Radius = XMVectorGetX(XMVector3Length(0.5f * (vMax - vMin)));

	fin >> ignore >> ignore >> ignore;

	std::vector<std::int32_t> indices(iCount * 3);
	for (auto i{ 0u }; i < iCount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}
	fin.close();

	UINT vbByteSize = static_cast<UINT>(vertices.size()) * sizeof(Vertex);
	UINT ibByteSize = static_cast<UINT>(indices.size()) * sizeof(std::int32_t);

	meshGeo->Name = "skullGeo";

	auto& submesh = meshGeo->DrawArgs["skull"];
	submesh.IndexCount = static_cast<UINT>(indices.size());
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	submesh.BBounds = boundingBoxBounds;
	submesh.BSphere = boundingSphere;

	meshGeo->VertexBufferByteSize = vbByteSize;
	meshGeo->VertexByteStride = sizeof(Vertex);

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &meshGeo->VertexBufferCPU));
	CopyMemory(meshGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &meshGeo->IndexBufferCPU));
	CopyMemory(meshGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	meshGeo->IndexFormat = DXGI_FORMAT_R32_UINT;
	meshGeo->IndexBufferByteSize = ibByteSize;

	return true;

	///////

	//auto geo = std::make_unique<MeshGeometry>();
	//geo->Name = "skullGeo";

	//auto& submesh = geo->DrawArgs["skull"];
	//submesh.IndexCount = static_cast<UINT>(indices.size());
	//submesh.StartIndexLocation = 0;
	//submesh.BaseVertexLocation = 0;
	//submesh.BBounds = boundingBoxBounds;
	//submesh.BSphere = boundingSphere;

	//geo->VertexBufferByteSize = vbByteSize;
	//geo->VertexByteStride = sizeof(Vertex);
	//geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(
	//	m_renderer->GetDevice(), m_renderer->GetCommandList(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	//ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	//CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	//ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	//CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	//geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	//geo->IndexBufferByteSize = ibByteSize;
	//geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(
	//	m_renderer->GetDevice(), m_renderer->GetCommandList(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	//m_Geometries[geo->Name] = std::move(geo);
}