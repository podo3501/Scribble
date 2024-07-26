#pragma once

interface IRenderer;
struct MaterialBuffer;
enum class SrvOffset : int;

struct Material
{
	Material();

	std::string name{};
	SrvOffset type;
	std::wstring diffuseName{};
	std::wstring normalName{};

	DirectX::XMFLOAT4 diffuseAlbedo{ 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 fresnelR0{ 0.01f, 0.01f, 0.01f };
	float roughness{ .25f };

	DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity();

	int numFramesDirty;
};

using MaterialList = std::vector<std::shared_ptr<Material>>;

class CMaterial
{
	using TextureList = std::vector<std::pair<SrvOffset, std::wstring>>;
	
public:
	CMaterial();
	~CMaterial();

	CMaterial(const CMaterial&) = delete;
	CMaterial& operator=(const CMaterial&) = delete;

	void SetMaterialList(const MaterialList& materialList);
	bool LoadTextureIntoVRAM(IRenderer* renderer);
	void MakeMaterialBuffer(IRenderer* renderer);

	int GetSrvTextureIndex(const std::wstring& filename);
	int GetMaterialIndex(const std::string& matName);

private:
	MaterialBuffer ConvertUploadBuffer(Material* material);
	void InsertTexture(SrvOffset type, const std::wstring& filename);

private:
	MaterialList m_materialList;
	TextureList m_textureList;
	std::vector<std::wstring> m_srvTextureList{};
};
