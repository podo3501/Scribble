#pragma once

interface IRenderer;
class CWindow;
class CCamera;
class CShadow;
class CSsao;
class CModel;
class CKeyInput;
class CGameTimer;
struct RenderItem;
struct InstanceData;
struct PassConstants;
enum class GraphicsPSO : int;

class CMainLoop
{
	using AllRenderItems = std::map<GraphicsPSO, std::unique_ptr<RenderItem>>;
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
	void UpdatePassCB();
	PassConstants UpdateMainPassCB();

	bool CALLBACK MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& lr);
	void CALLBACK SetAppPause(bool pause);
	bool CALLBACK OnResize(int width, int height);

	void AddKeyListener();

private:
	CWindow* m_window{ nullptr };
	IRenderer* m_iRenderer{ nullptr };

	std::unique_ptr<CKeyInput> m_keyInput;
	std::unique_ptr<CCamera> m_camera;
	std::unique_ptr<CGameTimer> m_timer;
	std::unique_ptr<CShadow> m_shadow;
	std::unique_ptr<CSsao> m_ssao;
	
	//�����͸� �ҷ��ͼ� �������� ������ �����͸� ����� �κ�
	std::unique_ptr<CModel> m_model;

	//�������� �ʿ��� �����͵�
	AllRenderItems m_AllRenderItems;
};