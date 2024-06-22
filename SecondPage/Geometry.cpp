#include "Geometry.h"
#include "../Core/d3dUtil.h"
#include "../Core/Directx3D.h"

CGeometry::CGeometry()
{}

bool CGeometry::Load(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Geometry* meshGeo)
{
	ReturnIfFalse(CoreUtil::CreateDefaultBuffer(
		device, cmdList,
		meshGeo->VertexBufferCPU->GetBufferPointer(),
		meshGeo->VertexBufferByteSize,
		meshGeo->VertexBufferUploader,
		&meshGeo->VertexBufferGPU));

	ReturnIfFalse(CoreUtil::CreateDefaultBuffer(
		device, cmdList,
		meshGeo->IndexBufferCPU->GetBufferPointer(),
		meshGeo->IndexBufferByteSize,
		meshGeo->IndexBufferUploader,
		&meshGeo->IndexBufferGPU));

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
	auto find = m_geometries.find(meshGeo->Name);
	if (find != m_geometries.end())
		return false;

	m_geometries[meshGeo->Name] = std::move(meshGeo);
	return true;
}

Geometry* CGeometry::GetMesh(const std::string& meshName)
{
	auto find = m_geometries.find(meshName);
	if (find == m_geometries.end())
		return nullptr;

	return find->second.get();
}