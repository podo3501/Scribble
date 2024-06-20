#include "MainLoop.h"
#include <WinUser.h>
#include <windowsx.h>
#include "../Core/d3dUtil.h"
#include "../Core/Utility.h"
#include "../Core/Window.h"
#include "../Core/Directx3D.h"
#include "../Core/UploadBuffer.h"
#include "../Core/GameTimer.h"
#include "./RendererData.h"
#include "./Shader.h"
#include "./Renderer.h"
#include "./Material.h"
#include "./Model.h"
#include "./FrameResource.h"
#include "./Texture.h"
#include "./Camera.h"
#include "./KeyInputManager.h"
#include "./Geometry.h"

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

bool CMainLoop::Initialize(HINSTANCE hInstance)
{
	m_camera = std::make_unique<CCamera>();
	m_camera->SetPosition(0.0f, 2.0f, -15.0f);
	m_camera->SetSpeed(eMove::Forward, 10.0f);
	m_camera->SetSpeed(eMove::Back, 10.0f);
	m_camera->SetSpeed(eMove::Right, 10.0f);
	m_camera->SetSpeed(eMove::Left, 10.0f);

	m_timer = std::make_unique<CGameTimer>();

	m_window = std::make_unique<CWindow>(hInstance);
	ReturnIfFalse(m_window->Initialize());
	m_window->AddWndProcListener([mainLoop = this](HWND wnd, UINT msg, WPARAM wp, LPARAM lp, LRESULT& lr)->bool {
		return mainLoop->MsgProc(wnd, msg, wp, lp, lr); });

	m_directx3D = std::make_unique<CDirectx3D>(m_window.get());
	ReturnIfFalse(m_directx3D->Initialize());

	m_renderer = std::make_shared<CRenderer>(m_directx3D.get());
	ReturnIfFalse(m_renderer->Initialize());

	m_geometry = std::make_unique<CGeometry>();

	ReturnIfFalse(OnResize());

	//데이터를 시스템 메모리에 올리기
	m_material = std::make_unique<CMaterial>();
	m_material->Build();

	m_texture = std::make_unique<CTexture>(m_resourcePath + L"Textures/");
	m_model = std::make_unique<CModel>();

	ReturnIfFalse(m_model->Read(m_geometry.get()));
	BuildRenderItems();

	m_frameResources = std::make_unique<CFrameResources>();
	ReturnIfFalse(m_frameResources->BuildFrameResources(m_renderer->GetDevice(),
		1, 125, static_cast<UINT>(m_material->GetCount())));
	
	ReturnIfFalse(BuildGraphicMemory());			//시스템 메모리에서 그래픽 메모리에 데이터 올리기
	AddKeyListener();

	return true;
}

bool CMainLoop::BuildGraphicMemory()
{
	ReturnIfFalse(m_geometry->LoadGraphicMemory(m_directx3D.get()));
	ReturnIfFalse(m_texture->LoadGraphicMemory(m_directx3D.get(), m_renderer.get()));

	return true;
}

void CMainLoop::BuildRenderItems()
{
	auto geo = m_geometry->GetMesh(m_model->GetName());

	auto rItem = std::make_unique<RenderItem>();
	auto MakeRenderItem = [&, objIdx{ 0 }](std::string&& smName, std::string&& matName,
		const XMMATRIX& world, const XMMATRIX& texTransform) mutable {
			auto& sm = geo->DrawArgs[smName];
			rItem->Geo = geo;
			rItem->Mat = m_material->GetMaterial(matName);
			rItem->ObjCBIndex = objIdx++;
			rItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			XMStoreFloat4x4(&rItem->World, world);
			XMStoreFloat4x4(&rItem->TexTransform, texTransform);
			rItem->StartIndexLocation = sm.StartIndexLocation;
			rItem->BaseVertexLocation = sm.BaseVertexLocation;
			rItem->IndexCount = sm.IndexCount;
			rItem->BoundingBoxBounds = sm.BBox;
			rItem->BoundingSphere = sm.BSphere; };
	MakeRenderItem(m_model->GetSubmeshName(), "tile0", XMMatrixIdentity(), XMMatrixIdentity());
	
	const int n = 5;
	const int matCount = static_cast<int>(m_material->GetCount());
	rItem->Instances.resize(n * n * n);

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;

	float x = -0.5f * width;
	float y = -0.5f * height;
	float z = -0.5f * depth;
	float dx = width / (n - 1);
	float dy = height / (n - 1);
	float dz = depth / (n - 1);
	for (int k = 0; k < n; ++k)
	{
		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				int index = k * n * n + i * n + j;
				rItem->Instances[index].World = XMFLOAT4X4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x + j * dx, y + i * dy, z + k * dz, 1.0f);

				XMStoreFloat4x4(&rItem->Instances[index].TexTransform, XMMatrixScaling(2.0f, 2.0f, 1.0f));
				rItem->Instances[index].MaterialIndex = index % matCount;
			}
		}
	}

	m_renderItems.emplace_back(std::move(rItem));
}

void CMainLoop::UpdateRenderItems()
{
	XMMATRIX view = m_camera->GetView();
	XMMATRIX invView = XMMatrixInverse(&RvToLv(XMMatrixDeterminant(view)), view);
	auto instanceBuffer = m_frameResources->GetUploadBuffer(eBufferType::Instance);

	for (auto& e : m_renderItems)
	{
		const auto& instanceData = e->Instances;

		int visibleInstanceCount = 0;

		for (UINT i = 0; i < (UINT)instanceData.size(); ++i)
		{
			XMMATRIX world = XMLoadFloat4x4(&instanceData[i].World);
			XMMATRIX texTransform = XMLoadFloat4x4(&instanceData[i].TexTransform);

			XMMATRIX invWorld = XMMatrixInverse(&RvToLv(XMMatrixDeterminant(world)), world);

			// View space to the object's local space.
			XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

			// Transform the camera frustum from view space to the object's local space.
			BoundingFrustum localSpaceFrustum;
			m_camFrustum.Transform(localSpaceFrustum, viewToLocal);

			// Perform the box/frustum intersection test in local space.
			if ((localSpaceFrustum.Contains(e->BoundingSphere) != DirectX::DISJOINT) || (m_frustumCullingEnabled == false))
			{
				InstanceData data;
				XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
				data.MaterialIndex = instanceData[i].MaterialIndex;

				// Write the instance data to structured buffer for the visible objects.
				instanceBuffer->CopyData(visibleInstanceCount++, data);
			}
		}

		e->InstanceCount = visibleInstanceCount;
		
		std::wostringstream outs;
		outs.precision(6);
		outs << L"Instancing and Culling Demo" <<
			L"    " << e->InstanceCount <<
			L" objects visible out of " << e->Instances.size();
		m_windowCaption = outs.str();
	}
}

void CMainLoop::UpdateMaterialBuffer()
{
	auto materialBuffer = m_frameResources->GetUploadBuffer(eBufferType::Material);
	const auto& materials = m_material->GetTotalMaterial();
	for (auto& e : materials)
	{
		Material* m = e.second.get();
		if (m->NumFramesDirty <= 0)
			continue;

		XMMATRIX matTransform = XMLoadFloat4x4(&m->MatTransform);

		MaterialData matData;
		matData.DiffuseAlbedo = m->DiffuseAlbedo;
		matData.FresnelR0 = m->FresnelR0;
		matData.Roughness = m->Roughness;
		XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
		matData.DiffuseMapIndex = m->DiffuseSrvHeapIndex;

		materialBuffer->CopyData(m->MatCBIndex, matData);

		m->NumFramesDirty--;
	}
}

bool CMainLoop::OnResize()
{
	int width = m_window->GetWidth();
	int height = m_window->GetHeight();
	ReturnIfFalse(m_renderer->OnResize(width, height));
	auto aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	m_camera->SetLens(0.25f * MathHelper::Pi, aspectRatio, 1.0f, 1000.f);

	BoundingFrustum::CreateFromMatrix(m_camFrustum, m_camera->GetProj());

	return true;
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

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);
		m_windowCaption += L"    fps: " + fpsStr + L"   mspf: " + mspfStr;
		m_window->SetText(m_windowCaption);

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

void StoreMatrix4x4(XMFLOAT4X4& dest, XMFLOAT4X4& src) { XMStoreFloat4x4(&dest, XMMatrixTranspose(XMLoadFloat4x4(&src))); }
void StoreMatrix4x4(XMFLOAT4X4& dest, XMMATRIX src) { XMStoreFloat4x4(&dest, XMMatrixTranspose(src)); }
XMMATRIX Multiply(XMFLOAT4X4& m1, XMFLOAT4X4 m2) { return XMMatrixMultiply(XMLoadFloat4x4(&m1), XMLoadFloat4x4(&m2)); }
XMMATRIX Inverse(XMMATRIX& m) { return XMMatrixInverse(nullptr, m); }
XMMATRIX Inverse(XMFLOAT4X4& src) { return Inverse(RvToLv(XMLoadFloat4x4(&src))); }

void CMainLoop::UpdateMainPassCB()
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
	pc.TotalTime = m_timer->TotalTime();
	pc.DeltaTime = m_timer->DeltaTime();
	pc.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	pc.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	pc.Lights[0].Strength = { 0.8f, 0.8f, 0.8f };
	pc.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	pc.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
	pc.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	pc.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

	passCB->CopyData(0, pc);
}

bool CMainLoop::Run()
{
	m_timer->Reset();
	MSG msg = { 0 };
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
				ReturnIfFalse(m_frameResources->Synchronize(m_directx3D->GetFence()));

				UpdateRenderItems();
				UpdateMaterialBuffer();
				UpdateMainPassCB();

				ReturnIfFalse(m_renderer->Draw(m_timer.get(), m_frameResources.get(), m_renderItems));
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return true;
}