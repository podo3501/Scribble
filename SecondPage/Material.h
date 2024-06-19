#pragma once

#include <DirectXMath.h>
#include <memory>
#include <unordered_map>
#include <string>

class UploadBuffer;
struct Material;
struct FrameResource;

class CMaterial
{
public:
	CMaterial();
	~CMaterial() = default;

	CMaterial(const CMaterial&) = delete;
	CMaterial& operator=(const CMaterial&) = delete;

	void Build();
	bool UpdateMaterialBuffer(UploadBuffer* materialBuffer);

	Material* GetMaterial(const std::string& matName);
	size_t GetCount();

private:
	std::unordered_map<std::string, std::unique_ptr<Material>> m_materials{};
};
