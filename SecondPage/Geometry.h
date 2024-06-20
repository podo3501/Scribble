#pragma once

#include <memory>
#include <unordered_map>
#include <string>

class CDirectx3D;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct Geometry;

class CGeometry
{
public:
	CGeometry();

	CGeometry(const CGeometry&) = delete;
	CGeometry& operator=(const CGeometry&) = delete;

	bool LoadGraphicMemory(CDirectx3D* directx3D);
	bool SetMesh(std::unique_ptr<Geometry>&& meshGeo);
	Geometry* GetMesh(const std::string& meshName);

private:
	bool Load(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Geometry* meshGeo);

private:
	std::unordered_map<std::string, std::unique_ptr<Geometry>> m_geometries{};
};