#include "Geometry.h"
#include "../Core/d3dUtil.h"
#include "../Core/Directx3D.h"

CGeometry::CGeometry()
{}

bool CGeometry::Load(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Geometry* meshGeo)
{
	ReturnIfFalse(CoreUtil::CreateDefaultBuffer(
		device, cmdList,
		meshGeo->vertexBufferCPU->GetBufferPointer(),
		meshGeo->vertexBufferByteSize,
		meshGeo->vertexBufferUploader,
		&meshGeo->vertexBufferGPU));

	ReturnIfFalse(CoreUtil::CreateDefaultBuffer(
		device, cmdList,
		meshGeo->indexBufferCPU->GetBufferPointer(),
		meshGeo->indexBufferByteSize,
		meshGeo->indexBufferUploader,
		&meshGeo->indexBufferGPU));

	return true;
}

bool CGeometry::LoadGraphicMemory(CDirectx3D* directx3D)
{
	return (directx3D->LoadData(
		[geo = this](ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)->bool {
			return std::all_of(geo->m_geometries.begin(), geo->m_geometries.end(), 
				[&](auto& meshGeo) { return geo->Load(device, cmdList, meshGeo.second.get());
				});
		}));
}

bool CGeometry::SetMesh(std::unique_ptr<Geometry>&& meshGeo)
{
	auto find = m_geometries.find(meshGeo->name);
	if (find != m_geometries.end())
		return false;

	m_geometries[meshGeo->name] = std::move(meshGeo);
	return true;
}

Geometry* CGeometry::GetMesh(const std::string& meshName)
{
	auto find = m_geometries.find(meshName);
	if (find == m_geometries.end())
		return nullptr;

	return find->second.get();
}