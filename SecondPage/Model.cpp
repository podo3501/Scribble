#include "Model.h"
#include <DirectXMath.h>
#include "../Core/d3dUtil.h"
#include "../Core/Utility.h"
#include "../Core/GameTimer.h"
#include "../Core/UploadBuffer.h"
#include "./FrameResource.h"
#include "./RendererData.h"
#include "./Material.h"
#include "./Camera.h"


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

	ReturnIfFailed(D3DCreateBlob(vbByteSize, &meshGeo->VertexBufferCPU));
	CopyMemory(meshGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ReturnIfFailed(D3DCreateBlob(ibByteSize, &meshGeo->IndexBufferCPU));
	CopyMemory(meshGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	meshGeo->IndexFormat = DXGI_FORMAT_R32_UINT;
	meshGeo->IndexBufferByteSize = ibByteSize;

	return true;
}

void CModel::BuildRenderItems(
	MeshGeometry* pGeo, CMaterial* pMaterial, std::vector<std::unique_ptr<RenderItem>>& renderItems)
{
	auto rItem = std::make_unique<RenderItem>();
	auto MakeRenderItem = [&, objIdx{ 0 }](std::string&& smName, std::string&& matName,
		const XMMATRIX& world, const XMMATRIX& texTransform) mutable {
			auto& sm = pGeo->DrawArgs[smName];
			rItem->Geo = pGeo;
			rItem->Mat = pMaterial->GetMaterial(matName);
			rItem->ObjCBIndex = objIdx++;
			rItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			XMStoreFloat4x4(&rItem->World, world);
			XMStoreFloat4x4(&rItem->TexTransform, texTransform);
			rItem->StartIndexLocation = sm.StartIndexLocation;
			rItem->BaseVertexLocation = sm.BaseVertexLocation;
			rItem->IndexCount = sm.IndexCount;
			rItem->BoundingBoxBounds = sm.BBounds;
			rItem->BoundingSphere = sm.BSphere; };
	MakeRenderItem("skull", "tile0", XMMatrixIdentity(), XMMatrixIdentity());

	const int n = 5;
	rItem->Instances.resize(n * n * n);

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;

	float x = -0.5f * width;
	float y = -0.5f * height;
	float z = -0.5f * depth;
	float dx = width / (n - 1);
	float dy = height / (n - 1);
	float dz = depth / (n - 1);
	for (int k = 0; k < n; ++k)
	{
		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				int index = k * n * n + i * n + j;
				rItem->Instances[index].World = XMFLOAT4X4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x + j * dx, y + i * dy, z + k * dz, 1.0f);

				XMStoreFloat4x4(&rItem->Instances[index].TexTransform, XMMatrixScaling(2.0f, 2.0f, 1.0f));
				rItem->Instances[index].MaterialIndex = index % pMaterial->GetCount();
			}
		}
	}

	renderItems.emplace_back(std::move(rItem));
}


void CModel::Update(const CGameTimer* gt,
	const CCamera* camera,
	UploadBuffer* instanceBuffer,
	DirectX::BoundingFrustum& camFrustum,
	bool frustumCullingEnabled,
	std::vector<std::unique_ptr<RenderItem>>& renderItems)
{
	XMMATRIX view = camera->GetView();
	XMMATRIX invView = XMMatrixInverse(&RvToLv(XMMatrixDeterminant(view)), view);

	for (auto& e : renderItems)
	{
		const auto& instanceData = e->Instances;

		int visibleInstanceCount = 0;

		for (UINT i = 0; i < (UINT)instanceData.size(); ++i)
		{
			XMMATRIX world = XMLoadFloat4x4(&instanceData[i].World);
			XMMATRIX texTransform = XMLoadFloat4x4(&instanceData[i].TexTransform);

			XMMATRIX invWorld = XMMatrixInverse(&RvToLv(XMMatrixDeterminant(world)), world);

			// View space to the object's local space.
			XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

			// Transform the camera frustum from view space to the object's local space.
			BoundingFrustum localSpaceFrustum;
			camFrustum.Transform(localSpaceFrustum, viewToLocal);

			// Perform the box/frustum intersection test in local space.
			//if ((localSpaceFrustum.Contains(e->BoundingBoxBounds) != DirectX::DISJOINT) || (mFrustumCullingEnabled == false))
			if ((localSpaceFrustum.Contains(e->BoundingSphere) != DirectX::DISJOINT) || (frustumCullingEnabled == false))
			{
				InstanceData data;
				XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
				data.MaterialIndex = instanceData[i].MaterialIndex;

				// Write the instance data to structured buffer for the visible objects.
				instanceBuffer->CopyData(visibleInstanceCount++, data);
			}
		}

		e->InstanceCount = visibleInstanceCount;

		//std::wostringstream outs;
		//outs.precision(6);
		//outs << L"Instancing and Culling Demo" <<
		//	L"    " << e->InstanceCount <<
		//	L" objects visible out of " << e->Instances.size();
		//mMainWndCaption = outs.str();
	}
}