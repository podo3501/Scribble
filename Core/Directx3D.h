#pragma once

#include<wtypes.h>
#include"CoreInterface.h"
#include<memory>

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class CWindow;

class CDirectx3D
{
public:
	CDirectx3D(HINSTANCE hInstance);

	bool Initialize(WNDPROC wndProc);

public:
	CDirectx3D(const CDirectx3D&) = delete;
	CDirectx3D& operator=(const CDirectx3D&) = delete;

private:
	bool InitDirect3D();

private:
	std::unique_ptr<CWindow> m_window{ nullptr };
};