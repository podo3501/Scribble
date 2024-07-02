#pragma once

#include <functional>
#include <string>
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
enum class eBufferType : int;
enum class eTextureType : int;

using AllRenderItems = std::unordered_map<std::string, std::unique_ptr<RenderItem>>;

interface IRenderer
{
	virtual ~IRenderer() {};

	virtual bool IsInitialize() { return false; };
	virtual bool OnResize(int wndWidth, int wndHeight) { return true; };
	virtual bool LoadData(std::function<bool(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)> loadGraphicMemory)
	{ return true; };
	virtual bool LoadTexture(eTextureType type, std::vector<std::wstring>& filenames) { return true; };
	virtual bool SetUploadBuffer(eBufferType bufferType, const void* bufferData, size_t dataSize) { return true; };
	virtual bool PrepareFrame() { return true; };
	virtual bool Draw(AllRenderItems& renderItem) { return true; };
	virtual bool WaitUntilGpuFinished(UINT64 fenceCount) { return true; };
};

std::unique_ptr<IRenderer> CreateRenderer(std::wstring resPath, CWindow* window);