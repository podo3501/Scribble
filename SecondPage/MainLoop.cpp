#include "MainLoop.h"
#include <WinUser.h>
#include <windowsx.h>
#include "../Core/d3dUtil.h"
#include "../Core/Utility.h"
#include "../Core/Window.h"
#include "./UploadBuffer.h"
#include "./GameTimer.h"
#include "./RendererDefine.h"
#include "./RenderItem.h"
#include "./Shader.h"
#include "./interface.h"
#include "./Material.h"
#include "./Model.h"
#include "./FrameResource.h"
#include "./FrameResourceData.h"
#include "./Texture.h"
#include "./Camera.h"
#include "./KeyInputManager.h"
#include "./Instance.h"

using namespace DirectX;

RenderItem::RenderItem()
	: NumFramesDirty{ gFrameResourceCount }
{}

CMainLoop::~CMainLoop() = default;

CMainLoop::CMainLoop(std::wstring resourcePath)
{
	m_resourcePath = resourcePath;
}

bool CMainLoop::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& lr)
{
	switch (msg)
	{
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
		if (m_iRenderer->GetDevice())
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

bool CMainLoop::Initialize(CWindow* window, IRenderer* renderer)
{
	m_window = window;
	m_iRenderer = renderer;

	m_window->AddWndProcListener([this](HWND wnd, UINT msg, WPARAM wp, LPARAM lp, LRESULT& lr)->bool {
		return MsgProc(wnd, msg, wp, lp, lr); });

	AddKeyListener();	//키 리스너 등록

	//local에서 작업할 것들
	ReturnIfFalse(InitializeClass());	//초기화
	ReturnIfFalse(OnResize());
	ReturnIfFalse(BuildCpuMemory());	//데이터를 시스템 메모리에 올리기
	ReturnIfFalse(BuildGraphicMemory());		//시스템 메모리에서 그래픽 메모리에 데이터 올리기
	ReturnIfFalse(MakeFrameResource());		//한 프레임에 쓰일 리소스를 만듦

	//view에서 작업할 것들(run은 실시간으로 projection과 관련한다)
	m_instance->CreateInstanceData(nullptr, "nature", "cube");
	m_instance->CreateInstanceData(m_material.get(), "things", "skull");

	ReturnIfFalse(m_instance->FillRenderItems(m_AllRenderItems));
	
	return true;
}

bool CMainLoop::InitializeClass()
{
	m_camera = std::make_unique<CCamera>();
	m_camera->SetPosition(0.0f, 2.0f, -15.0f);
	m_camera->SetSpeed(10.0f);

	m_timer = std::make_unique<CGameTimer>();

	m_material = std::make_unique<CMaterial>();
	m_material->Build();

	m_instance = std::make_unique<CInstance>();

	return true;
}

bool CMainLoop::MakeFrameResource()
{
	m_frameResources = std::make_unique<CFrameResources>();
	ReturnIfFalse(m_frameResources->BuildFrameResources(m_iRenderer->GetDevice(),
		gPassCBCount, gInstanceBufferCount, gMaterialBufferCount));

	return true;
}

bool CMainLoop::BuildCpuMemory()
{
	ModelTypeList modelTypeList
	{
		ModelType(CreateType::Generator, "nature", "cube"),
		ModelType(CreateType::ReadFile, "things", "skull", L"skull.txt")
	};

	m_model = std::make_unique<CModel>(m_resourcePath);
	ReturnIfFalse(m_model->LoadGeometryList(modelTypeList));

	return true;
}

bool CMainLoop::BuildGraphicMemory()
{
	ReturnIfFalse(m_model->LoadGraphicMemory(m_iRenderer, &m_AllRenderItems));
	m_texture = std::make_unique<CTexture>(m_iRenderer, m_resourcePath);
	ReturnIfFalse(m_texture->LoadGraphicMemory());
	
	return true;
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

void CMainLoop::FindVisibleSubRenderItems(
	SubRenderItems& subRenderItems, int* instanceStartIndex, InstanceDataList& visibleInstance)
{
	XMMATRIX view = m_camera->GetView();
	XMMATRIX invView = XMMatrixInverse(&RvToLv(XMMatrixDeterminant(view)), view);

	for (auto& iterSubItem : subRenderItems)
	{
		auto& subRenderItem = iterSubItem.second;
		auto& instanceInfo = subRenderItem.instanceInfo;
		auto& instanceList = instanceInfo.instanceDataList;
		if (instanceInfo.cullingFrustum)
		{
			std::ranges::copy_if(instanceList, std::back_inserter(visibleInstance),
				[this, &invView, &subRenderItem](auto& instance) {
					return IsInsideFrustum(subRenderItem.subItem.boundingSphere, invView, instance->world);
				});
			m_windowCaption = SetWindowCaption(visibleInstance.size(), instanceList.size());
		}
		else
			std::ranges::copy(instanceList, std::back_inserter(visibleInstance));

		subRenderItem.instanceCount = static_cast<UINT>(visibleInstance.size());
		subRenderItem.startIndexInstance = (*instanceStartIndex);
		(*instanceStartIndex) += subRenderItem.instanceCount;
	}
}

void CMainLoop::UpdateRenderItems()
{
	//처리 안할것을 먼저 골라낸다.
	InstanceDataList visibleInstance{};
	int instanceStartIndex{ 0 };
	for (auto& e : m_AllRenderItems)
	{
		auto& subRenderItems = e.second->subRenderItems;
	
		//보여지는 서브 아이템을 찾아낸다.
		FindVisibleSubRenderItems(subRenderItems, &instanceStartIndex, visibleInstance);
	}
	UpdateInstanceBuffer(visibleInstance);
}

void CMainLoop::UpdateInstanceBuffer(const InstanceDataList& visibleInstance)
{
	auto instanceBuffer = m_frameResources->GetUploadBuffer(eBufferType::Instance);

	int visibleCount{ 0 };
	for (auto& instanceData : visibleInstance)
	{
		InstanceBuffer curInsBuf{};
		XMStoreFloat4x4(&curInsBuf.world, XMMatrixTranspose(instanceData->world));
		XMStoreFloat4x4(&curInsBuf.texTransform, XMMatrixTranspose(instanceData->texTransform));
		curInsBuf.materialIndex = instanceData->matIndex;

		instanceBuffer->CopyData(visibleCount++, curInsBuf);
	}
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
	ReturnIfFalse(m_iRenderer->OnResize(width, height));
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
	m_keyInputManager->AddListener([this](std::vector<int> keyList) {
		PressedKey(keyList); });
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
	m_keyInputManager->PressedKeyList([]()->std::vector<int> {
		std::vector<int> keyList{ 'W', 'S', 'D', 'A', '1', '2' };
		std::vector<int> pressedKeyList{};
		std::ranges::copy_if(keyList, std::back_inserter(pressedKeyList),
			[](int vKey) { return GetAsyncKeyState(vKey) & 0x8000; });
		return pressedKeyList; });
}

bool CMainLoop::Run(IRenderer* renderer)
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

			ReturnIfFalse(m_frameResources->PrepareFrame(m_iRenderer));

			UpdateRenderItems();
			UpdateMaterialBuffer();
			UpdateMainPassCB();

			ReturnIfFalse(renderer->Draw(m_timer.get(), m_frameResources.get(), m_AllRenderItems));
		}
	}

	return true;
}