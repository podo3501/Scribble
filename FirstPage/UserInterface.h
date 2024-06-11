#ifdef _DEBUG
#pragma comment(lib, "../Lib/Common_d.lib")
#else
#pragma comment(lib, "../Lib/Common.lib")
#endif

#include "../Common/d3dApp.h"
#include "../Common/MathHelper.h"
#include "../Common/Camera.h"
#include <map>

class Waves;
struct FrameResource;
struct InstanceData;

struct RenderItem
{
	RenderItem() = default;

	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	int NumFramesDirty = gNumFrameResources;
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	DirectX::BoundingBox BoundingBoxBounds{};
	DirectX::BoundingSphere BoundingSphere{};
	std::vector<InstanceData> Instances;

	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
	UINT InstanceCount = 0;
};

class Model
{
public:
	Model();
	Model(std::shared_ptr<Renderer>& renderer);
	void Update(const GameTimer& gt,
		const Camera& camera,
		FrameResource* curFrameRes,
		DirectX::BoundingFrustum& camFrustum,
		bool frustumCullingEnabled);
	void UpdateMaterialBuffer(FrameResource* curFrameRes);
	UINT GetMaterialsCount();
	std::vector<RenderItem*> GetRenderItems();

private:
	void LoadSkull();
	void BuildMaterials();
	void BuildRenderItems();

private:
	std::shared_ptr<Renderer> m_renderer = nullptr;
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> m_materials;
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	std::vector<RenderItem*> mOpaqueRitems;
};

class MainLoop
{
public:
	static MainLoop* GetMainLoop();
	//MainLoop(std::shared_ptr<Model>& model, std::shared_ptr<Renderer>& renderer);
	MainLoop(HINSTANCE hInstance);
	int Run();
	void OnResize();

	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void CalculateFrameStats();
	void OnKeyboardInput();
	void UpdateMainPassCB();
	void BuildFrameResources();


	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
	 
private:
	std::shared_ptr<Model> m_model = nullptr;
	std::shared_ptr<Renderer> m_renderer = nullptr;
	FrameResource* m_curFrameRes = nullptr;

	std::vector<std::unique_ptr<FrameResource>> m_frameResources;

	DirectX::BoundingFrustum m_camFrustum{};

	HINSTANCE mhAppInst = nullptr; // application instance handle
	HWND      mhMainWnd = nullptr; // main window handle
	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
	bool      mFullscreenState = false;// fullscreen enabled

	// Set true to use 4X MSAA (?.1.8).  The default is false.
	bool      m4xMsaaState = false;    // 4X MSAA enabled
	UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

	GameTimer m_timer;
	Camera m_camera;
	bool m_appPaused = false;
	bool m_frustumCullingEnabled = true;
	UINT m_frameResIdx = 0;
	POINT mLastMousePos{};

	std::wstring m_mainWndCaption = L"d3d App";

	int mClientWidth = 800;
	int mClientHeight = 600;

	static MainLoop* g_mainLoop;
};

enum class GraphicsPSO : int
{
	Opaque,
	Count
};

constexpr std::array<GraphicsPSO, static_cast<size_t>(GraphicsPSO::Count)> GraphicsPSO_ALL
{
	GraphicsPSO::Opaque,
};

class InstancingAndCullingApp : public D3DApp
{
public:
	//InstancingAndCullingApp() = delete;
	InstancingAndCullingApp(HINSTANCE hInstance);
	InstancingAndCullingApp(const InstancingAndCullingApp& rhs) = delete;
	InstancingAndCullingApp& operator=(const InstancingAndCullingApp& rhs) = delete;
	~InstancingAndCullingApp();

	virtual bool Initialize(WNDPROC wndProc) override;

private:
	virtual void OnResize() override;
	virtual void Update(const GameTimer& gt) override;
	virtual void Draw(const GameTimer& gt) override;
	virtual void Draw(
		const GameTimer& gt,
		FrameResource* pCurrFrameRes,
		std::vector<RenderItem*> renderItem) override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

	void OnKeyboardInput(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	void UpdateInstanceData(const GameTimer& gt);
	void UpdateMaterialBuffer(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	void LoadTextures();
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();
	void BuildSkullGeometry();
	void BuildFrameResources();
	void BuildMaterials();
	void MakeOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);
	void MakePSOPipelineState(GraphicsPSO psoType);
	void BuildPSOs();
	void BuildRenderItems();
	void DrawRenderItems(const std::vector<RenderItem*>& ritems);
	void DrawRenderItems(FrameResource* pCurrFrameRes, const std::vector<RenderItem*>& ritems);

	void Render() override {};

private:
	std::vector<std::unique_ptr<Texture>> mTextures;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> mShaders;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	std::vector<RenderItem*> mOpaqueRitems;
	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	std::unordered_map<GraphicsPSO, Microsoft::WRL::ComPtr<ID3D12PipelineState>> mPSOs;

	FrameResource* mCurFrameRes = nullptr;
	UINT mFrameResIdx = 0;

	POINT mLastMousePos{};
	bool mFrustumCullingEnabled = true;
	DirectX::BoundingFrustum mCamFrustum{};

	Camera mCamera;
};
