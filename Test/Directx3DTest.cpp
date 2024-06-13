#include "pch.h"
#include "../Core/Directx3D.h"
#include "../Core/Window.h"
#include "../SecondPage/Camera.h"
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>

LRESULT CALLBACK
TestWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

class CRenderer
{
public:
	CRenderer(std::unique_ptr<CDirectx3D>&& directx3d, std::shared_ptr<CCamera>& camera);
	bool Initialize();

	CRenderer(const CRenderer&) = delete;
	CRenderer& operator=(const CRenderer&) = delete;

private:
	std::unique_ptr<CDirectx3D> m_directx3D{ nullptr };
	std::shared_ptr<CCamera> m_camera{ nullptr };
};

CRenderer::CRenderer(std::unique_ptr<CDirectx3D>&& directx3d, std::shared_ptr<CCamera>& camera)
	: m_directx3D(std::move(directx3d))
	, m_camera(camera)
{}

bool CRenderer::Initialize()
{
	m_directx3D->ResetCommandLists();
	return true;
}

namespace core
{
	TEST(Renderer, Initialize)
	{
		std::unique_ptr<CDirectx3D> directx3d = std::make_unique<CDirectx3D>(GetModuleHandle(nullptr));
		EXPECT_EQ(directx3d->Initialize(TestWndProc), true);

		std::shared_ptr<CCamera> camera = std::make_unique<CCamera>();
		camera->SetPosition(0.0f, 2.0f, -15.0f);

		std::shared_ptr<CRenderer> renderer = std::make_shared<CRenderer>(std::move(directx3d), camera);
		EXPECT_EQ(renderer->Initialize(), true);
	}
}

