#pragma once

#include <functional>
#include <memory>
#include <string>
#include <Windows.h>
#include <unordered_map>

interface IRenderer;
class CWindow;
class CCamera;
class CModel;
class CKeyInputManager;
class CGameTimer;
struct RenderItem;
struct InstanceData;

class CMainLoop
{
	using AllRenderItems = std::unordered_map<std::string, std::unique_ptr<RenderItem>>;
	using InstanceDataList = std::vector<std::shared_ptr<InstanceData>>;

public:
	CMainLoop();
	~CMainLoop();

	CMainLoop(const CMainLoop&) = delete;
	CMainLoop& operator=(const CMainLoop&) = delete;

	bool Initialize(const std::wstring& resourcePath, CWindow* window, IRenderer* renderer);
	bool Run(IRenderer* renderer = nullptr);

private:
	bool InitializeClass(const std::wstring& resourcePath);
	void UpdateMainPassCB();

	bool CALLBACK MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& lr);
	void CALLBACK SetAppPause(bool pause);
	bool CALLBACK OnResize(int width, int height);

	void AddKeyListener();

private:
	CWindow* m_window{ nullptr };
	IRenderer* m_iRenderer{ nullptr };

	std::unique_ptr<CCamera> m_camera;
	std::unique_ptr<CGameTimer> m_timer;
	std::unique_ptr<CKeyInputManager> m_keyInputManager;
	
	//�����͸� �ҷ��ͼ� �������� ������ �����͸� ����� �κ�
	std::unique_ptr<CModel> m_model;

	//�������� �ʿ��� �����͵�
	AllRenderItems m_AllRenderItems;
};