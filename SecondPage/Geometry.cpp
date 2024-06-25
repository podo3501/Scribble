#include "Geometry.h"
#include "../Core/d3dUtil.h"
#include "../Core/Directx3D.h"

CGeometry::CGeometry()
	: m_geometries{ std::make_unique<Geometry>() }
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
			return geo->Load(device, cmdList, geo->m_geometries.get());
		}));
}