#include "MainLoop.h"
#include <WinUser.h>
#include <windowsx.h>
#include "../Core/d3dUtil.h"
#include "../Core/Utility.h"
#include "../Core/Window.h"
#include "../Core/Directx3D.h"
#include "../Core/UploadBuffer.h"
#include "../Core/GameTimer.h"
#include "./Shader.h"
#include "./Renderer.h"
#include "./RendererDefine.h"
#include "./Material.h"
#include "./Model.h"
#include "./FrameResource.h"
#include "./Texture.h"
#include "./Camera.h"
#include "./KeyInputManager.h"

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
	m_camera = std::make_unique<CCamera>();
	m_camera->SetPosition(0.0f, 2.0f, -15.0f);
	m_camera->SetSpeed(eMove::Forward, 10.0f);
	m_camera->SetSpeed(eMove::Back, 10.0f);
	m_camera->SetSpeed(eMove::Right, 10.0f);
	m_camera->SetSpeed(eMove::Left, 10.0f);

	m_window = std::make_unique<CWindow>(hInstance);
	ReturnIfFailed(m_window->Initialize());
	m_window->AddWndProcListener([mainLoop = this](HWND wnd, UINT msg, WPARAM wp, LPARAM lp, LRESULT& lr)->bool {
		return mainLoop->MsgProc(wnd, msg, wp, lp, lr); });

	m_directx3D = std::make_unique<CDirectx3D>(m_window.get());
	ReturnIfFailed(m_directx3D->Initialize());

	m_renderer = std::make_shared<CRenderer>(m_directx3D.get());
	ReturnIfFalse(m_renderer->Initialize());

	OnResize();

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

	m_frameResources = std::make_unique<CFrameResources>();
	m_frameResources->BuildFrameResources(m_renderer->GetDevice(), 
		1, 125, static_cast<UINT>(m_material->GetCount()));
	
	BuildGraphicMemory();			//시스템 메모리에서 그래픽 메모리에 데이터 올리기
	AddKeyListener();

	return S_OK;
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
	int width = m_window->GetWidth();
	int height = m_window->GetHeight();
	m_renderer->OnResize(width, height);
	auto aspectRatio = static_cast<float>(width) / static_cast<float>(height);
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

namespace HelpMatrix
{
	void StoreMatrix4x4(XMFLOAT4X4& dest, XMFLOAT4X4& src) { XMStoreFloat4x4(&dest, XMMatrixTranspose(XMLoadFloat4x4(&src))); }
	void StoreMatrix4x4(XMFLOAT4X4& dest, XMMATRIX src) { XMStoreFloat4x4(&dest, XMMatrixTranspose(src)); }
	XMMATRIX Multiply(XMFLOAT4X4& m1, XMFLOAT4X4 m2) { return XMMatrixMultiply(XMLoadFloat4x4(&m1), XMLoadFloat4x4(&m2)); }
	XMMATRIX Inverse(XMMATRIX& m) { return XMMatrixInverse(nullptr, m); }
	XMMATRIX Inverse(XMFLOAT4X4& src) { return Inverse(RvToLv(XMLoadFloat4x4(&src))); }
};

using namespace HelpMatrix;

void CMainLoop::UpdateMainPassCB(const CGameTimer* gt)
{
	auto passCB = m_frameResources->GetUploadBuffer(eBufferType::PassCB);
	XMMATRIX view = m_camera->GetView();
	XMMATRIX proj = m_camera->GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	PassConstants pc;
	StoreMatrix4x4(pc.View, view);
	StoreMatrix4x4(pc.InvView, Inverse(view));
	StoreMatrix4x4(pc.Proj, proj);
	StoreMatrix4x4(pc.InvProj, Inverse(proj));
	StoreMatrix4x4(pc.ViewProj, viewProj);
	StoreMatrix4x4(pc.InvViewProj, Inverse(viewProj));
	pc.EyePosW = m_camera->GetPosition3f();
	pc.RenderTargetSize = { (float)m_window->GetWidth(), (float)m_window->GetHeight()};
	pc.InvRenderTargetSize = { 1.0f / (float)m_window->GetWidth(), 1.0f / (float)m_window->GetHeight() };
	pc.NearZ = 1.0f;
	pc.FarZ = 1000.0f;
	pc.TotalTime = gt->TotalTime();
	pc.DeltaTime = gt->DeltaTime();
	pc.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	pc.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	pc.Lights[0].Strength = { 0.8f, 0.8f, 0.8f };
	pc.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	pc.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
	pc.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	pc.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

	passCB->CopyData(0, pc);
}

int CMainLoop::Run()
{
	MSG msg = { 0 };
	m_timer = std::make_unique<CGameTimer>();
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
				m_frameResources->Synchronize(m_directx3D->GetFence());

				m_model->Update(m_timer.get(), m_camera.get(), 
					m_frameResources->GetUploadBuffer(eBufferType::Instance),
					m_camFrustum, m_frustumCullingEnabled, m_renderItems);
				m_material->UpdateMaterialBuffer(m_frameResources->GetUploadBuffer(eBufferType::Material));

				UpdateMainPassCB(m_timer.get());

				m_renderer->Draw(m_timer.get(), m_frameResources.get(), m_renderItems);
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}