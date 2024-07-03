#pragma once

#include <functional>
#include <string>
#include <vector>
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

using AllRenderItems = std::unordered_map<std::string, std::unique_ptr<RenderItem>>;
using Vertices = std::vector<Vertex>;
using Indices = std::vector<std::int32_t>;

interface IRenderer
{
	virtual ~IRenderer() {};

	virtual bool IsInitialize() { return false; };
	virtual bool OnResize(int wndWidth, int wndHeight) { return true; };
	virtual bool LoadModel(Vertices& totalVertices, Indices& totalIndices, RenderItem* renderItem) { return true; };
	virtual bool LoadTexture(eTextureType type, std::vector<std::wstring>& filenames) { return true; };
	virtual bool SetUploadBuffer(eBufferType bufferType, const void* bufferData, size_t dataSize) { return true; };
	virtual bool PrepareFrame() { return true; };
	virtual bool Draw(AllRenderItems& renderItem) { return true; };

	virtual void Set4xMsaaState(HWND hwnd, int widht, int height, bool value) {};
};

std::unique_ptr<IRenderer> CreateRenderer(std::wstring resPath, HWND hwnd, int width, int height);