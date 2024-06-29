#pragma once

#include <functional>
#include <memory>
#include <string>
#include <Windows.h>
#include <DirectXCollision.h>

interface IRenderer;
class CWindow;
class CCamera;
class CMaterial;
class CTexture;
class CModel;
class CFrameResources;
class CKeyInputManager;
class CGameTimer;
class CGeometry;
class CInstance;
struct RenderItem;
struct FrameResource;
struct InstanceData;
struct RenderItem;
struct SubRenderItem;

class CMainLoop
{
	using AllRenderItems = std::unordered_map<std::string, std::unique_ptr<RenderItem>>;
	using InstanceDataList = std::vector<std::shared_ptr<InstanceData>>;
	using SubRenderItems = std::unordered_map<std::string, SubRenderItem>;

public:
	template<typename T>
	CMainLoop(T&& resourcePath);
	~CMainLoop() = default;

	CMainLoop() = delete;
	CMainLoop(const CMainLoop&) = delete;
	CMainLoop& operator=(const CMainLoop&) = delete;

	bool Initialize(HINSTANCE hInstance, bool bShowWindow = true);
	bool Run();

private:
	bool MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& lr);

	bool InitializeClass();

	bool BuildCpuMemory();
	bool BuildGraphicMemory();

	bool MakeFrameResource();
	bool IsInsideFrustum(
		const DirectX::BoundingSphere& bSphere, 
		const DirectX::XMMATRIX& invView, 
		const DirectX::XMMATRIX& world);

	void UpdateRenderItems();
	void UpdateInstanceBuffer(const InstanceDataList& visibleInstance);
	void UpdateMaterialBuffer();
	void UpdateMainPassCB();
	void FindVisibleSubRenderItems(
		SubRenderItems& subRenderItems, int* instanceStartIndex, InstanceDataList& visibleInstance);

	bool OnResize();

	void AddKeyListener();
	void PressedKey(std::vector<int> keyList);
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

	void CalculateFrameStats();
	void OnKeyboardInput();

private:
	std::wstring m_resourcePath{};
	std::unique_ptr<CWindow> m_window{ nullptr };
	std::shared_ptr<IRenderer> m_iRenderer{ nullptr };

	std::unique_ptr<CCamera> m_camera{ nullptr };
	std::unique_ptr<CFrameResources> m_frameResources{ nullptr };
	std::unique_ptr<CGameTimer> m_timer{ nullptr };
	std::unique_ptr<CKeyInputManager> m_keyInputManager{ nullptr };

	std::unique_ptr<CMaterial> m_material{ nullptr };
	std::unique_ptr<CTexture> m_texture{ nullptr };
	std::unique_ptr<CModel> m_model{ nullptr };
	std::unique_ptr<CInstance> m_instance{ nullptr };
	
	//랜더링시 필요한 데이터들
	AllRenderItems m_AllRenderItems{};

	DirectX::BoundingFrustum m_camFrustum{};
	bool m_frustumCullingEnabled{ true };

	bool m_appPaused{ false };
	bool m_minimized{ false };
	bool m_maximized{ false };
	bool m_resizing{ false };

	POINT m_lastMousePos{};
	std::wstring m_windowCaption{};
};

template<typename T>
CMainLoop::CMainLoop(T&& resourcePath)
{
	m_resourcePath = std::forward<T>(resourcePath);
}