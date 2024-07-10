//
// pch.h
//

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

#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "../Lib/DirectXTK12_d.lib")
#else
#pragma comment(lib, "../Lib/DirectXTK12.lib")
#endif
