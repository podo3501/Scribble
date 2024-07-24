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
enum class SrvOffset : int;
enum class GraphicsPSO : int;
enum class ShaderType : int;

using ShaderFileList = std::map<GraphicsPSO, std::vector<std::pair<ShaderType, std::wstring>>>;
using CreateModelNames = std::map<GraphicsPSO, std::vector<std::string>>;

std::unique_ptr<Material> MakeMaterial(std::string&& name, SrvOffset type, std::vector<std::wstring> texFilenames,
	DirectX::XMFLOAT4 diffuseAlbedo, DirectX::XMFLOAT3 fresnelR0, float rough);
ModelProperty CreateMock(const std::string& meshName);
CreateModelNames MakeMockData();
void GetMockLight(PassConstants* outPc);
ShaderFileList GetShaderFileList();
