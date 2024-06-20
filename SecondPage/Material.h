#pragma once

#include <DirectXMath.h>
#include <memory>
#include <unordered_map>
#include <string>

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

	Material* GetMaterial(const std::string& matName);
	size_t GetCount();
	inline const auto& GetTotalMaterial() { return m_materials; };

private:
	std::unordered_map<std::string, std::unique_ptr<Material>> m_materials{};
};
