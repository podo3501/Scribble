#pragma once

#include <functional>
#include <memory>
#include <string>
#include <Windows.h>
#include <DirectXCollision.h>

class CWindow;
class CRenderer;
class CDirectx3D;
class CCamera;
class CMaterial;
class CTexture;
class CModel;
class CFrameResources;
class CKeyInputManager;
class CGameTimer;
class CGeometry;
class CInstance;
struct InstanceData;
struct RenderItem;
struct Geometry;
struct FrameResource;

class CMainLoop
{
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
	void BuildRenderItems(
		const std::string& geoName,
		const std::string& meshName,
		const std::string& matName);

	bool MakeFrameResource();
	bool IsInsideFrustum(
		const DirectX::BoundingSphere& bSphere, 
		const DirectX::XMMATRIX& invView, 
		const DirectX::XMMATRIX& world);

	void UpdateRenderItems();
	void UpdateInstanceBuffer(const std::vector<std::shared_ptr<InstanceData>>& visibleInstance);
	void UpdateMaterialBuffer();
	void UpdateMainPassCB();

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
	std::unique_ptr<CDirectx3D> m_directx3D{ nullptr };
	std::shared_ptr<CRenderer> m_renderer{ nullptr };
	std::unique_ptr<CCamera> m_camera{ nullptr };
	std::unique_ptr<CMaterial> m_material{ nullptr };
	std::unique_ptr<CFrameResources> m_frameResources{ nullptr };
	std::unique_ptr<CTexture> m_texture{ nullptr };
	std::unique_ptr<CModel> m_model{ nullptr };
	std::unique_ptr<CGameTimer> m_timer{ nullptr };
	std::unique_ptr<CKeyInputManager> m_keyInputManager{ nullptr };
	std::unique_ptr<CGeometry> m_geometry{ nullptr };
	std::unique_ptr<CInstance> m_instance{ nullptr };

	//랜더링시 필요한 데이터들
	std::unordered_map<std::string, std::vector<std::unique_ptr<RenderItem>>> m_AllRItems{};

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