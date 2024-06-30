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

interface IRenderer
{
	virtual ~IRenderer() {};

	virtual bool OnResize(int wndWidth, int wndHeight) { return true; };
	virtual bool LoadData(std::function<bool(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)> loadGraphicMemory)
	{ return true; };
	virtual bool Draw(CGameTimer* gt, CFrameResources* frameResources,
		std::unordered_map<std::string, std::unique_ptr<RenderItem>>& renderItem) { return true; };
	virtual bool WaitUntilGpuFinished(UINT64 fenceCount) { return true; };

	virtual inline ID3D12Device* GetDevice() const { return nullptr; };
	virtual inline ID3D12DescriptorHeap* GetSrvDescriptorHeap() const { return nullptr;	};
};

std::unique_ptr<IRenderer> CreateRenderer(std::wstring resPath, CWindow* window);