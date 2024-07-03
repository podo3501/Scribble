#pragma once

#include <functional>
#include <memory>
#include <string>
#include <Windows.h>
#include <DirectXCollision.h>
#include <unordered_map>

interface IRenderer;
class CWindow;
class CCamera;
class CMaterial;
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
	CMainLoop(std::wstring resourcePath);
	~CMainLoop();

	CMainLoop() = delete;
	CMainLoop(const CMainLoop&) = delete;
	CMainLoop& operator=(const CMainLoop&) = delete;

	bool Initialize(CWindow* window, IRenderer* renderer);
	bool Run(IRenderer* renderer = nullptr);

private:
	bool InitializeClass();
	bool LoadMemory();

	bool IsInsideFrustum(
		const DirectX::BoundingSphere& bSphere, 
		const DirectX::XMMATRIX& invView, 
		const DirectX::XMMATRIX& world);

	void UpdateRenderItems();
	void UpdateInstanceBuffer(const InstanceDataList& visibleInstance);
	void UpdateMainPassCB();
	void FindVisibleSubRenderItems(SubRenderItems& subRenderItems, InstanceDataList& visibleInstance);

	bool CALLBACK MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& lr);
	void CALLBACK SetAppPause(bool pause);
	bool CALLBACK OnResize(int width, int height);

	bool OnResize();
	void AddKeyListener();
	void PressedKey(std::vector<int> keyList);
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
	void OnKeyboardInput();

private:
	std::wstring m_resourcePath{};
	CWindow* m_window{ nullptr };
	IRenderer* m_iRenderer{ nullptr };

	std::unique_ptr<CCamera> m_camera{ nullptr };
	std::unique_ptr<CMaterial> m_material{ nullptr };
	std::unique_ptr<CGameTimer> m_timer{ nullptr };
	std::unique_ptr<CKeyInputManager> m_keyInputManager{ nullptr };
	
	std::unique_ptr<CModel> m_model{ nullptr };
	std::unique_ptr<CInstance> m_instance{ nullptr };
	//랜더링시 필요한 데이터들
	AllRenderItems m_AllRenderItems{};

	DirectX::BoundingFrustum m_camFrustum{};
	bool m_frustumCullingEnabled{ true };

	POINT m_lastMousePos{};
	std::wstring m_windowCaption{};
};