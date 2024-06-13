#include "pch.h"
#include "../Core/Directx3D.h"
#include "../Core/Window.h"

LRESULT CALLBACK
TestWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

namespace core
{
	TEST(Directx3D, Initialize)
	{
		CDirectx3D directx3d(GetModuleHandle(nullptr));
		auto result = directx3d.Initialize(TestWndProc);
		EXPECT_EQ(result, true);
	}
}

