#pragma once

#include <string>
#include <memory>
#include <DirectXMath.h>

class CSetupData;
class CMaterial;
enum class eTextureType : int;
struct Material;
struct ModelProperty;

std::unique_ptr<Material> MakeMaterial(std::string&& name, eTextureType type, std::wstring&& filename,
	DirectX::XMFLOAT4 diffuseAlbedo, DirectX::XMFLOAT3 fresnelR0, float rough);
ModelProperty CreateMock(const std::string& meshName);
bool MakeMockData(CSetupData* setupData, CMaterial* material);
