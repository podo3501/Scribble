#pragma once

#include "gtest/gtest.h"

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

#include <winsdkver.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <wrl/client.h>
#include <wrl/event.h>

#ifdef USING_DIRECTX_HEADERS
#include <directx/dxgiformat.h>
#include <directx/d3d12.h>
#else
#include <d3d12.h>
#endif

#include <dxgi1_6.h>

#include <DirectXColors.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#define D3DX12_NO_STATE_OBJECT_HELPERS
#define D3DX12_NO_CHECK_FEATURE_SUPPORT_CLASS
#include "../Include/d3dx12.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <exception>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <vector>

// To use graphics and CPU markup events with the latest version of PIX, change this to include <pix3.h>
// then add the NuGet package WinPixEventRuntime to the project.
#include <pix.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include "../DirectXTK12/Inc/Audio.h"
#include "../DirectXTK12/Inc/BufferHelpers.h"
#include "../DirectXTK12/Inc/CommonStates.h"
#include "../DirectXTK12/Inc/DirectXHelpers.h"
#include "../DirectXTK12/Inc/DDSTextureLoader.h"
#include "../DirectXTK12/Inc/DescriptorHeap.h"
#include "../DirectXTK12/Inc/Effects.h"
#include "../DirectXTK12/Inc/GamePad.h"
#include "../DirectXTK12/Inc/GeometricPrimitive.h"
#include "../DirectXTK12/Inc/GraphicsMemory.h"
#include "../DirectXTK12/Inc/Keyboard.h"
#include "../DirectXTK12/Inc/Model.h"
#include "../DirectXTK12/Inc/Mouse.h"
#include "../DirectXTK12/Inc/PrimitiveBatch.h"
#include "../DirectXTK12/Inc/ResourceUploadBatch.h"
#include "../DirectXTK12/Inc/RenderTargetState.h"
#include "../DirectXTK12/Inc/SimpleMath.h"
#include "../DirectXTK12/Inc/SpriteBatch.h"
#include "../DirectXTK12/Inc/SpriteFont.h" 
#include "../DirectXTK12/Inc/VertexTypes.h"