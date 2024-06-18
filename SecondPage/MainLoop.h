#pragma once

#include<functional>
#include<memory>
#include<string>
#include<Windows.h>

class CWindow;
class CRenderer;
class CDirectx3D;
class CCamera;
class CMaterial;
class CTexture;
class CModel;
class CKeyInputManager;
struct RenderItem;
struct MeshGeometry;
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

	HRESULT Initialize(HINSTANCE hInstance);

private:
	bool WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& lr);

	void BuildFrameResource();
	void BuildGraphicMemory();
	void Load(MeshGeometry* meshGeo);
	void AddKeyListener();

	void PressedKey(std::vector<int> keyList);

private:
	std::wstring m_resourcePath{};
	std::unique_ptr<CWindow> m_window{ nullptr };
	std::unique_ptr<CDirectx3D> m_directx3D{ nullptr };
	std::shared_ptr<CRenderer> m_renderer{ nullptr };
	std::unique_ptr<CCamera> m_camera{ nullptr };
	std::unique_ptr<CMaterial> m_material{ nullptr };
	std::unique_ptr<CTexture> m_texture{ nullptr };
	std::unique_ptr<CModel> m_model{ nullptr };
	std::unique_ptr<CKeyInputManager> m_keyInputManager{ nullptr };

	std::vector<std::unique_ptr<RenderItem>> m_renderItems{};
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_geometries{};
	std::vector<std::unique_ptr<FrameResource>> m_frameResources{};

	bool m_frustumCullingEnabled{ false };
};

template<typename T>
CMainLoop::CMainLoop(T&& resourcePath)
{
	m_resourcePath = std::forward<T>(resourcePath);
}
