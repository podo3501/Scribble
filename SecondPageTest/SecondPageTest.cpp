#include "pch.h"
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <memory>
#include <unordered_map>
#include <functional>
#include "../Include/interface.h"
#include "../Include/RenderItem.h"
#include "../Include/FrameResourceData.h"
#include "../Include/types.h"
#include "../SecondPage/Window.h"
#include "../SecondPage/GameTimer.h"
#include "../SecondPage/Camera.h"
#include "../SecondPage/Model.h"
#include "../SecondPage/Material.h"
#include "../SecondPage/MainLoop.h"
#include "../SecondPage/KeyInputManager.h"
#include "../SecondPage/SetupData.h"
#include "../SecondPage/GeometryGenerator.h"
#include "../SecondPage/Utility.h"

namespace MainLoop
{
	class MainLoopClassTest : public ::testing::Test
	{
	public:
		MainLoopClassTest() {};

	protected:
		void SetUp() override
		{
			m_window = std::make_unique<CWindow>(GetModuleHandle(nullptr));
			EXPECT_EQ(m_window->Initialize(false), true);
			m_renderer = CreateRenderer(m_resourcePath, m_window->GetHandle(), m_window->GetWidth(), m_window->GetHeight());
			EXPECT_EQ(m_renderer != nullptr, true);
			m_setupData = std::make_unique<CSetupData>();
			EXPECT_EQ(m_setupData->CreateMockData(), true);
		}

		void TearDown() override
		{
			m_setupData.reset();
			m_renderer.reset();
			m_window.reset();
		}
		
	protected:
		std::wstring m_resourcePath{ L"../Resource/" };
		std::unique_ptr<CWindow> m_window{ nullptr };
		std::unique_ptr<IRenderer> m_renderer{ nullptr };
		std::unique_ptr<CSetupData> m_setupData{ nullptr };
	};

	TEST_F(MainLoopClassTest, CameraUpdate)
	{
		auto deltaTime = 0.1f;

		CKeyInputManager keyInput(m_window->GetHandle());
		CCamera camera;
		camera.SetPosition(0.0f, 0.0f, 0.0f);
		camera.SetSpeed(eMove::Forward, 10.0f);
		keyInput.AddKeyListener([&camera](std::vector<int> keyList) {
			camera.PressedKey(keyList); });
		//mock으로 처리 가능할지 생각해보자
		//keyInput.PressedKeyList([]() {
		//	return std::vector<int>{'W'};
		//	});
		//camera.Update(deltaTime);
		//DirectX::XMFLOAT3 pos = camera.GetPosition();
		//EXPECT_EQ(pos.z, 1.0f);
	}

	TEST_F(MainLoopClassTest, WindowMessage)
	{
		MSG msg = { 0 };
		msg.hwnd = m_window->GetHandle();
		msg.message = WM_SIZE;
		msg.lParam = MAKELONG(1024, 768);

		DispatchMessage(&msg);

		EXPECT_EQ(m_window->GetWidth(), 1024);
		EXPECT_EQ(m_window->GetHeight(), 768);
	}

	TEST_F(MainLoopClassTest, Instance)
	{
		AllRenderItems renderItems{};
		std::unique_ptr<CModel> model = std::make_unique<CModel>(m_resourcePath);
		EXPECT_EQ(m_setupData->LoadModel(model.get(), &renderItems), true);
		EXPECT_EQ(model->LoadModelIntoVRAM(m_renderer.get(), &renderItems), true);
		EXPECT_EQ(renderItems.empty(), false);
	}

	TEST_F(MainLoopClassTest, Texture)
	{
		std::unique_ptr<CMaterial> material = std::make_unique<CMaterial>();
		EXPECT_EQ(m_setupData->LoadTextureIntoVRAM(m_renderer.get(), material.get()), true);
	}

	std::unique_ptr<Material> MakeMaterial(std::string&& name, eTextureType type, std::wstring&& filename,
		DirectX::XMFLOAT4 diffuseAlbedo, DirectX::XMFLOAT3 fresnelR0, float rough)
	{
		std::unique_ptr<Material> material = std::make_unique<Material>();
		material->name = std::move(name);
		material->type = type;
		material->filename = std::move(filename);
		material->normalSrvHeapIndex = 0;
		material->diffuseAlbedo = diffuseAlbedo;
		material->fresnelR0 = fresnelR0;
		material->roughness = rough;
		material->transform = DirectX::XMMatrixIdentity();

		return std::move(material);
	}

	ModelProperty CreateMock(const std::string& meshName)
	{
		MaterialList materialList;
		materialList.emplace_back(MakeMaterial("bricks0", eTextureType::Common, L"bricks.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.002f, 0.002f, 0.02f }, 0.1f));
		materialList.emplace_back(MakeMaterial("sky", eTextureType::Cube, L"grasscube1024.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 1.0f));
		materialList.emplace_back(MakeMaterial("bricks0", eTextureType::Common, L"bricks.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.002f, 0.002f, 0.02f }, 0.1f));
		materialList.emplace_back(MakeMaterial("bricks1", eTextureType::Common, L"bricks.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.002f, 0.002f, 0.02f }, 0.1f));
		materialList.emplace_back(MakeMaterial("bricks2", eTextureType::Common, L"bricks2.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.002f, 0.002f, 0.02f }, 0.1f));

		ModelProperty  modelProp{};
		modelProp.createType = ModelProperty::CreateType::Generator;
		modelProp.meshData = nullptr;
		modelProp.cullingFrustum = false;
		modelProp.filename = L"";
		modelProp.instanceDataList = {};
		modelProp.materialList = materialList;
		
		return modelProp;
	}

	class GMockTestRenderer : public IRenderer
	{
		virtual bool LoadTexture(eTextureType type, std::set<std::wstring>& filenames) override
		{
			switch (type)
			{
			case eTextureType::Cube: EXPECT_EQ(filenames.size(), 1); break;
			case eTextureType::Common: EXPECT_EQ(filenames.size(), 2); break;
			}

			return true;
		}
	};

	TEST_F(MainLoopClassTest, MockData)
	{
		std::unique_ptr<CMaterial> material = std::make_unique<CMaterial>();
		std::unique_ptr<CSetupData> setupData = std::make_unique<CSetupData>();
		setupData->N_InsertModelProperty("nature", "cube1", CreateMock("cube"), material.get());
		setupData->N_InsertModelProperty("nature", "cube2", CreateMock("cube"), material.get());

		std::unique_ptr<IRenderer> mockRenderer = std::make_unique<GMockTestRenderer>();
		EXPECT_EQ(material->LoadTextureIntoVRAM(mockRenderer.get()), true);
		EXPECT_EQ(material->GetDiffuseIndex(L"bricks.dds"), 1);
		EXPECT_EQ(material->GetDiffuseIndex(L"brickddddd"), -1);
	}
} //SecondPage

class GTestRenderer : public IRenderer
{
	virtual bool Draw(std::unordered_map<std::string, std::unique_ptr<RenderItem>>& renderItem) override
	{
		EXPECT_EQ(renderItem.empty(), false);
		PostQuitMessage(0);
		return true;
	};
};

namespace A_SecondPage
{
	TEST(MainLoop, RunTest)
	{
		std::wstring resPath = L"../Resource/";

		std::unique_ptr<CWindow> window = std::make_unique<CWindow>(GetModuleHandle(nullptr));
		window->Initialize(true);
		auto renderer = CreateRenderer(resPath, window->GetHandle(), window->GetWidth(), window->GetHeight());

		std::unique_ptr<CMainLoop> mainLoop = std::make_unique<CMainLoop>();
		EXPECT_EQ(mainLoop->Initialize(resPath, window.get(), renderer.get()), true);

		GTestRenderer testRenderer{};
		EXPECT_EQ(mainLoop->Run(&testRenderer), true);
	}
}