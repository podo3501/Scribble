#pragma once

#include<functional>
#include<memory>
#include<string>
#include<Windows.h>

class CRenderer;
class CDirectx3D;
class CMaterial;
class CTexture;
class CModel;
struct MeshGeometry;
struct FrameResource;

class CMainLoop
{
	using WINDOWPROC = std::function<LRESULT CALLBACK(HWND, UINT, WPARAM, LPARAM)>;

public:
	template<typename T>
	CMainLoop(T&& resourcePath);
	~CMainLoop() = default;

	CMainLoop() = delete;
	CMainLoop(const CMainLoop&) = delete;
	CMainLoop& operator=(const CMainLoop&) = delete;

	HRESULT Initialize(HINSTANCE hInstance);

private:
	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void BuildFrameResource();
	void BuildGraphicMemory();
	void Load(MeshGeometry* meshGeo);

private:
	std::wstring m_resourcePath{};
	std::unique_ptr<CDirectx3D> m_directx3D{ nullptr };
	std::shared_ptr<CRenderer> m_renderer{ nullptr };
	std::unique_ptr<CMaterial> m_material{ nullptr };
	std::unique_ptr<CTexture> m_texture{ nullptr };
	std::unique_ptr<CModel> m_model{ nullptr };

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_geometries{};
	std::vector<std::unique_ptr<FrameResource>> m_frameResources{};
};

template<typename T>
CMainLoop::CMainLoop(T&& resourcePath)
{
	m_resourcePath = std::forward<T>(resourcePath);
}
