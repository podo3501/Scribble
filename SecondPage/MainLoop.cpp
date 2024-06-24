#include "MainLoop.h"
#include <WinUser.h>
#include <windowsx.h>
#include "../Core/d3dUtil.h"
#include "../Core/Utility.h"
#include "../Core/Window.h"
#include "../Core/Directx3D.h"
#include "./UploadBuffer.h"
#include "./GameTimer.h"
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

bool CMainLoop::InitializeClass()
{
	m_camera = std::make_unique<CCamera>();
	m_camera->SetPosition(0.0f, 2.0f, -15.0f);
	m_camera->SetSpeed(10.0f);

	m_timer = std::make_unique<CGameTimer>();

	m_directx3D = std::make_unique<CDirectx3D>(m_window.get());
	ReturnIfFalse(m_directx3D->Initialize());

	m_renderer = std::make_shared<CRenderer>(m_resourcePath);
	ReturnIfFalse(m_renderer->Initialize(m_directx3D.get()));

	m_geometry = std::make_unique<CGeometry>();

	return true;
}

bool CMainLoop::Initialize(HINSTANCE hInstance, bool bShowWindow)
{
	m_window = std::make_unique<CWindow>(hInstance);
	ReturnIfFalse(m_window->Initialize(bShowWindow));
	m_window->AddWndProcListener([mainLoop = this](HWND wnd, UINT msg, WPARAM wp, LPARAM lp, LRESULT& lr)->bool {
		return mainLoop->MsgProc(wnd, msg, wp, lp, lr); });

	ReturnIfFalse(InitializeClass());	//초기화
	ReturnIfFalse(OnResize());

	ReturnIfFalse(BuildCpuMemory());	//데이터를 시스템 메모리에 올리기
	ReturnIfFalse(m_model->Convert(m_geometry.get()));		//그래픽 메모리에 올릴 준비단계
	BuildRenderItems();
	
	ReturnIfFalse(BuildGraphicMemory());		//시스템 메모리에서 그래픽 메모리에 데이터 올리기
	ReturnIfFalse(MakeFrameResource());		//한 프레임에 쓰일 리소스를 만듦

	AddKeyListener();	//키 리스너 등록

	return true;
}

bool CMainLoop::MakeFrameResource()
{
	m_frameResources = std::make_unique<CFrameResources>();
	ReturnIfFalse(m_frameResources->BuildFrameResources(m_renderer->GetDevice(),
		1, 125, static_cast<UINT>(m_material->GetCount(TextureType::Total))));

	return true;
}

bool CMainLoop::BuildCpuMemory()
{
	m_material = std::make_unique<CMaterial>();
	m_material->Build();
	m_model = std::make_unique<CModel>(m_resourcePath);
	ReturnIfFalse(m_model->Read());

	return true;
}

bool CMainLoop::BuildGraphicMemory()
{
	m_texture = std::make_unique<CTexture>(m_renderer.get(), m_resourcePath);
	ReturnIfFalse(m_geometry->LoadGraphicMemory(m_directx3D.get()));
	ReturnIfFalse(m_texture->LoadGraphicMemory());

	return true;
}

void CMainLoop::BuildRenderItems()
{
	const auto& renderName = m_model->GetName();
	auto geo = m_geometry->GetMesh(renderName);

	auto rItem = std::make_unique<RenderItem>();
	auto MakeRenderItem = [&, objIdx{ 0 }](std::string&& smName, std::string&& matName,
		const XMMATRIX& world, const XMMATRIX& texTransform) mutable {
			auto& sm = geo->drawArgs[smName];
			rItem->geo = geo;
			rItem->mat = m_material->GetMaterial(matName);
			rItem->objCBIndex = objIdx++;
			rItem->primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			XMStoreFloat4x4(&rItem->world, world);
			XMStoreFloat4x4(&rItem->texTransform, texTransform);
			rItem->startIndexLocation = sm.startIndexLocation;
			rItem->baseVertexLocation = sm.baseVertexLocation;
			rItem->indexCount = sm.indexCount;
			rItem->boundingBox = sm.boundingBox;
			rItem->boundingSphere = sm.boundingSphere; };
	MakeRenderItem(m_model->GetSubmeshName(), "tile0", XMMatrixIdentity(), XMMatrixIdentity());
	
	const int n = 5;
	const int matCount = static_cast<int>(m_material->GetCount(TextureType::Texture));
	const int startIndex = m_material->GetStartIndex(TextureType::Texture);

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
				auto instance = std::make_shared<InstanceData>();
				const XMFLOAT3 pos(x + j * dx, y + i * dy, z + k * dz);
				instance->world = XMMATRIX(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					pos.x, pos.y, pos.z, 1.0f);
				
				int index = k * n * n + i * n + j;
				instance->texTransform = XMMatrixScaling(2.0f, 2.0f, 1.0f);
				instance->matIndex = index % matCount;// +startIndex;
				m_instances.emplace_back(std::move(instance));
			}
		}
	}

	m_AllRItems[renderName].emplace_back(std::move(rItem));
}

bool CMainLoop::IsInsideFrustum(const DirectX::BoundingSphere& bSphere, const XMMATRIX& invView, const XMMATRIX& world)
{
	BoundingFrustum frustum{};
	XMMATRIX invWorld = XMMatrixInverse(&RvToLv(XMMatrixDeterminant(world)), world);
	XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

	m_camFrustum.Transform(frustum, viewToLocal);

	const bool isInside = (frustum.Contains(bSphere) != DirectX::DISJOINT);
	return (isInside || !m_frustumCullingEnabled);
}

std::wstring SetWindowCaption(std::size_t visibleCount, std::size_t totalCount)
{
	std::wostringstream outs;
	outs.precision(6);
	outs << L"Instancing and Culling Demo" <<
		L"    " << visibleCount <<
		L" objects visible out of " << totalCount;
	return outs.str();
}

void CMainLoop::UpdateRenderItems()
{
	XMMATRIX view = m_camera->GetView();
	XMMATRIX invView = XMMatrixInverse(&RvToLv(XMMatrixDeterminant(view)), view);
	auto instanceBuffer = m_frameResources->GetUploadBuffer(eBufferType::Instance);

	std::vector<std::shared_ptr<InstanceData>> insides{};

	//처리 안할것을 먼저 골라낸다.
	auto& item = (*m_AllRItems[m_model->GetName()].begin());
	std::copy_if(m_instances.begin(), m_instances.end(), std::back_inserter(insides),
		[mainLoop = this, &invView, &item](auto& instance) {
			return mainLoop->IsInsideFrustum(item->boundingSphere, invView, instance->world);
		});
	item->instanceCount = static_cast<UINT>(insides.size());
	
	int visibleCount{ 0 };
	for (auto& instanceData : insides)
	{
		InstanceBuffer curInsBuf{};
		XMStoreFloat4x4(&curInsBuf.world, XMMatrixTranspose(instanceData->world));
		XMStoreFloat4x4(&curInsBuf.texTransform, XMMatrixTranspose(instanceData->texTransform));
		curInsBuf.materialIndex = instanceData->matIndex;

		instanceBuffer->CopyData(visibleCount++, curInsBuf);
	}

	m_windowCaption = SetWindowCaption(insides.size(), m_instances.size());
}

void CMainLoop::UpdateMaterialBuffer()
{
	auto materialBuffer = m_frameResources->GetUploadBuffer(eBufferType::Material);
	m_material->MakeMaterialBuffer(&materialBuffer);
}

void CMainLoop::UpdateMainPassCB()
{
	auto passCB = m_frameResources->GetUploadBuffer(eBufferType::PassCB);
	
	PassConstants pc;
	m_camera->GetPassCB(&pc);
	m_timer->GetPassCB(&pc);

	float width = (float)m_window->GetWidth();
	float height = (float)m_window->GetHeight();
	pc.renderTargetSize = { width, height };
	pc.invRenderTargetSize = { 1.0f / width, 1.0f / height };

	pc.nearZ = 1.0f;
	pc.farZ = 1000.0f;
	pc.ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	pc.lights[0].direction = { 0.57735f, -0.57735f, 0.57735f };
	pc.lights[0].strength = { 0.8f, 0.8f, 0.8f };
	pc.lights[1].direction = { -0.57735f, -0.57735f, 0.57735f };
	pc.lights[1].strength = { 0.4f, 0.4f, 0.4f };
	pc.lights[2].direction = { 0.0f, -0.707f, -0.707f };
	pc.lights[2].strength = { 0.2f, 0.2f, 0.2f };

	passCB->CopyData(0, pc);
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

	const bool IsOverOneSecond = ((m_timer->TotalTime() - _timeElapsed) >= 1.0f);
	if (!IsOverOneSecond) return;
	
	float fps = (float)_frameCnt; // fps = frameCnt / 1
	float mspf = 1000.0f / fps;

	std::wstring fpsStr = std::to_wstring(fps);
	std::wstring mspfStr = std::to_wstring(mspf);
	m_windowCaption += L"    fps: " + fpsStr + L"   mspf: " + mspfStr;
	m_window->SetText(m_windowCaption);

	_frameCnt = 0;
	_timeElapsed += 1.0f;
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

			if (m_appPaused)
				Sleep(100);
			
			CalculateFrameStats();
			OnKeyboardInput();

			m_camera->Update(m_timer->DeltaTime());

			ReturnIfFalse(m_frameResources->PrepareFrame(m_directx3D.get()));

			UpdateRenderItems();
			UpdateMaterialBuffer();
			UpdateMainPassCB();

			ReturnIfFalse(m_renderer->Draw(m_timer.get(), m_frameResources.get(), m_AllRItems[m_model->GetName()]));
		}
	}

	return true;
}