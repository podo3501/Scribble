#include "pch.h"
#include "../Core/Directx3D.h"
#include "../Core/Window.h"
#include "../SecondPage/Camera.h"
#include "../SecondPage/Texture.h"
#include "../SecondPage/Renderer.h"
#include "../SecondPage/Shader.h"
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>

LRESULT CALLBACK
TestWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

namespace core
{
	TEST(Renderer, Initialize)
	{
		const std::wstring resourcePath = L"../Resource/";
		//초기화
		std::unique_ptr<CDirectx3D> directx3D = std::make_unique<CDirectx3D>(GetModuleHandle(nullptr));
		EXPECT_EQ(directx3D->Initialize(TestWndProc), true);

		std::shared_ptr<CRenderer> renderer = std::make_shared<CRenderer>(directx3D.get());
		EXPECT_EQ(renderer->Initialize(), true);

		//데이터를 시스템 메모리에 올리기
		std::unique_ptr<CTexture> texture = std::make_unique<CTexture>(resourcePath +L"Textures/");
		.
		//std::unique_ptr<CModel> model = std::make_unique<CModel>();

		//시스템 메모리에서 그래픽 메모리에 데이터 올리기
		directx3D->ResetCommandLists();

		texture->Load(renderer.get());

		directx3D->ExcuteCommandLists();
		directx3D->FlushCommandQueue();

		std::unique_ptr<CCamera> camera = std::make_unique<CCamera>();
		camera->SetPosition(0.0f, 2.0f, -15.0f);
	}
}

