#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <memory>
#include <unordered_map>
#include <combaseapi.h>

class CWindow;
class CGameTimer;
class CFrameResources;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12DescriptorHeap;
struct RenderItem;
struct Vertex;
enum class eBufferType : int;
enum class eTextureType : int;
enum class GraphicsPSO : int;
enum class ShaderType : int;

using AllRenderItems = std::map<GraphicsPSO, std::unique_ptr<RenderItem>>;
using Vertices = std::vector<Vertex>;
using Indices = std::vector<std::int32_t>;
using TextureList = std::vector<std::pair<eTextureType, std::wstring>>;
using ShaderFileList = std::map<GraphicsPSO, std::vector<std::pair<ShaderType, std::wstring>>>;

interface IRenderer
{
	virtual ~IRenderer() {};

	virtual bool IsInitialize() = 0;
	virtual bool OnResize(int wndWidth, int wndHeight) = 0;
	virtual bool LoadMesh(GraphicsPSO pso, Vertices& totalVertices, Indices& totalIndices, RenderItem* renderItem) = 0;
	virtual bool LoadTexture(const TextureList& textureList) = 0;
	virtual bool LoadTexture(const TextureList& textureList, std::vector<std::wstring>* srvFilename) = 0;
	virtual bool SetUploadBuffer(eBufferType bufferType, const void* bufferData, size_t dataSize) = 0;
	virtual bool PrepareFrame() = 0;
	virtual bool Draw(AllRenderItems& renderItem) = 0;

	virtual void Set4xMsaaState(HWND hwnd, int widht, int height, bool value) = 0;
};

std::unique_ptr<IRenderer> CreateRenderer(std::wstring& resPath, HWND hwnd, int width, int height, const ShaderFileList& fileList);