#include "MainLoop.h"
#include <WinUser.h>
#include <windowsx.h>
#include "../Core/d3dUtil.h"
#include "../Core/Window.h"
#include "../Core/Directx3D.h"
#include "./Shader.h"
#include "./Renderer.h"
#include "./RendererDefine.h"
#include "./Material.h"
#include "./Model.h"
#include "./FrameResource.h"
#include "./Texture.h"
#include "./Camera.h"
#include "./KeyInputManager.h"
#include "../Core/GameTimer.h"

using namespace DirectX;

bool CMainLoop::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& lr)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_appPaused = true;
			m_timer->Stop();
		}
		else
		{
			m_appPaused = false;
			m_timer->Start();
		}
		return true;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		if (m_renderer->GetDevice())
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_appPaused = true;
				m_minimized = true;
				m_maximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_appPaused = false;
				m_minimized = false;
				m_maximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (m_minimized)
				{
					m_appPaused = false;
					m_minimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if (m_maximized)
				{
					m_appPaused = false;
					m_maximized = false;
					OnResize();
				}
				else if (m_resizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return true;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		m_appPaused = true;
		m_resizing = true;
		m_timer->Stop();
		return true;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		m_appPaused = false;
		m_resizing = false;
		m_timer->Start();
		OnResize();
		return true;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return true;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return true;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return true;
	}

	return false;
}

HRESULT CMainLoop::Initialize(HINSTANCE hInstance)
{
	m_window = std::make_unique<CWindow>(hInstance);
	ReturnIfFailed(m_window->Initialize());
	m_window->AddWndProcListener([mainLoop = this](HWND wnd, UINT msg, WPARAM wp, LPARAM lp, LRESULT& lr)->bool {
		return mainLoop->MsgProc(wnd, msg, wp, lp, lr); });

	m_directx3D = std::make_unique<CDirectx3D>(m_window.get());
	ReturnIfFailed(m_directx3D->Initialize());;

	m_renderer = std::make_shared<CRenderer>(m_directx3D.get());
	ReturnIfFalse(m_renderer->Initialize());

	m_camera = std::make_unique<CCamera>();
	m_camera->SetPosition(0.0f, 2.0f, -15.0f);

	//데이터를 시스템 메모리에 올리기
	m_material = std::make_unique<CMaterial>();
	m_material->Build();

	m_texture = std::make_unique<CTexture>(m_resourcePath + L"Textures/");
	m_model = std::make_unique<CModel>();

	auto geo = std::make_unique<MeshGeometry>();
	m_model->Read(geo.get());
	m_model->BuildRenderItems(geo.get(), m_material.get(), m_renderItems);
	std::string geoName = geo->Name;
	m_geometries[geoName] = std::move(geo);

	BuildFrameResource();			//프레임당 쓰이는 데이터 공간을 확보
	BuildGraphicMemory();			//시스템 메모리에서 그래픽 메모리에 데이터 올리기
	AddKeyListener();

	return S_OK;
}

void CMainLoop::BuildFrameResource()
{
	for (auto i{ 0 }; i < gFrameResourceCount; ++i)
	{
		auto frameRes = std::make_unique<FrameResource>(m_renderer->GetDevice(), 1,
			125, static_cast<UINT>(m_material->GetCount()));
		m_frameResources.emplace_back(std::move(frameRes));
	}
}

void CMainLoop::BuildGraphicMemory()
{
	//시스템 메모리에서 그래픽 메모리에 데이터 올리기
	m_directx3D->ResetCommandLists();

	m_texture->Load(m_renderer.get());
	for_each(m_geometries.begin(), m_geometries.end(), [mainLoop = this](auto& geometry) {
		auto& meshGeo = geometry.second;
		mainLoop->Load(meshGeo.get()); });

	m_directx3D->ExcuteCommandLists();
	m_directx3D->FlushCommandQueue();
}

void CMainLoop::Load(MeshGeometry* meshGeo)
{
	meshGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(
		m_renderer->GetDevice(), m_renderer->GetCommandList(),
		meshGeo->VertexBufferCPU->GetBufferPointer(),
		meshGeo->VertexBufferByteSize,
		meshGeo->VertexBufferUploader);

	meshGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(
		m_renderer->GetDevice(), m_renderer->GetCommandList(),
		meshGeo->IndexBufferCPU->GetBufferPointer(),
		meshGeo->IndexBufferByteSize,
		meshGeo->IndexBufferUploader);

	return;
}

void CMainLoop::OnResize()
{
	auto aspectRatio =
		static_cast<float>(m_window->GetWidth()) /
		static_cast<float>(m_window->GetHeight());
	m_camera->SetLens(0.25f * MathHelper::Pi, aspectRatio, 1.0f, 1000.f);

	BoundingFrustum::CreateFromMatrix(m_camFrustum, m_camera->GetProj());
}

void CMainLoop::AddKeyListener()
{
	m_keyInputManager = std::make_unique<CKeyInputManager>();
	m_keyInputManager->AddListener([&cam = m_camera](std::vector<int> keyList) {
		cam->PressedKey(keyList); });
	m_keyInputManager->AddListener([mainLoop = this](std::vector<int> keyList) {
		mainLoop->PressedKey(keyList); });
}

void CMainLoop::PressedKey(std::vector<int> keyList)
{
	for (auto vKey : keyList)
	{
		switch (vKey)
		{
		case '1':		m_frustumCullingEnabled = true;			break;
		case '2':		m_frustumCullingEnabled = false;		break;
		}
	}
}

void CMainLoop::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePos.x = x;
	m_lastMousePos.y = y;

	SetCapture(m_window->GetHandle());
}
void CMainLoop::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}
void CMainLoop::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_lastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_lastMousePos.y));

		m_camera->Move(eMove::Pitch, dy);
		m_camera->Move(eMove::RotateY, dx);
	}

	m_lastMousePos.x = x;
	m_lastMousePos.y = y;
}

int CMainLoop::Run()
{
	MSG msg = { 0 };
	m_timer->Reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_timer->Tick();

			if (!m_appPaused)
			{
				CalculateFrameStats();

				OnKeyboardInput();

				m_camera->Update(m_timer->DeltaTime());

				//m_frameResource, m_renderItems, m_geometries이 세개는 RAM->VRAM으로 가는 연결다리 변수이다 나중에 리팩토링 하자
				//ID3D12Fence* pFence = m_renderer->GetFence();
				//m_frameResIdx = (m_frameResIdx + 1) % gNumFrameResources;
				//m_curFrameRes = m_frameResources[m_frameResIdx].get();
				//if (m_curFrameRes->Fence != 0 && pFence->GetCompletedValue() < m_curFrameRes->Fence)
				//{
				//	HANDLE hEvent = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
				//	ThrowIfFailed(pFence->SetEventOnCompletion(m_curFrameRes->Fence, hEvent));
				//	WaitForSingleObject(hEvent, INFINITE);
				//	CloseHandle(hEvent);
				//}

				//m_model->Update(m_timer, m_camera, m_curFrameRes, m_camFrustum, m_frustumCullingEnabled);
				//m_model->UpdateMaterialBuffer(m_curFrameRes);

				//UpdateMainPassCB();

				//m_renderer->Draw(m_timer, m_curFrameRes, m_model->GetRenderItems());
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

void CMainLoop::CalculateFrameStats()
{
	static int _frameCnt = 0;
	static float _timeElapsed = 0.0f;

	_frameCnt++;

	if ((m_timer->TotalTime() - _timeElapsed) >= 1.0f)
	{
		float fps = (float)_frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wstring caption = L"d3d App";
		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);
		m_window->SetText(caption + L"    fps: " + fpsStr + L"   mspf: " + mspfStr);

		_frameCnt = 0;
		_timeElapsed += 1.0f;
	}
}

void CMainLoop::OnKeyboardInput()
{
	//임시로 GetAsyncKeyState로 키 눌림을 구현했다. 나중에 다른 input으로 바꿀 예정
	m_keyInputManager->PressedKeyList([]() {
		std::vector<int> keyList{ 'W', 'S', 'D', 'A', '1', '2' };
		std::vector<int> pressedKeyList;
		for_each(keyList.begin(), keyList.end(), [&pressedKeyList](int vKey)
			{
				bool bPressed = GetAsyncKeyState(vKey) & 0x8000;
				if (bPressed)
					pressedKeyList.emplace_back(vKey);
			});
		return pressedKeyList; });
}