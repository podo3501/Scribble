#pragma once

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <DirectXMath.h>

class CSetupData;
class CMaterial;
struct Material;
struct ModelProperty;
struct PassConstants;
enum class eTextureType : int;
enum class GraphicsPSO : int;
enum class ShaderType : int;

using ShaderFileList = std::map<GraphicsPSO, std::vector<std::pair<ShaderType, std::wstring>>>;

std::unique_ptr<Material> MakeMaterial(std::string&& name, eTextureType type, std::wstring&& filename,
	DirectX::XMFLOAT4 diffuseAlbedo, DirectX::XMFLOAT3 fresnelR0, float rough);
ModelProperty CreateMock(const std::string& meshName);
bool MakeMockData(CSetupData* setupData, CMaterial* material);
void GetMockLight(PassConstants* outPc);
ShaderFileList GetShaderFileList();
