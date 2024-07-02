#pragma once

#include <DirectXMath.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <combaseapi.h>

interface IRenderer;
struct MaterialBuffer;

class CMaterial
{
public:
	CMaterial();
	~CMaterial();

	CMaterial(const CMaterial&) = delete;
	CMaterial& operator=(const CMaterial&) = delete;

	void Build();
	void MakeMaterialBuffer(IRenderer* renderer);

private:
	struct Material;
	MaterialBuffer ConvertUploadBuffer(UINT diffuseIndex, Material* material);

private:
	using MaterialData = std::pair<std::string, std::unique_ptr<Material>>;
	std::vector<MaterialData> m_materials;
};
