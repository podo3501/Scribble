#pragma once
#include <d3d12.h>
#include <wrl.h>

class CRenderer;
enum class eTextureType : int;

D3D12_GPU_DESCRIPTOR_HANDLE GetGpuSrvHandle(CRenderer* renderer, eTextureType type);
