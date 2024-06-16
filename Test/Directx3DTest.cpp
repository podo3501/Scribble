#include "pch.h"
#include "../Core/Directx3D.h"
#include "../Core/Window.h"
#include "../SecondPage/Camera.h"
#include "../SecondPage/Texture.h"
#include "../SecondPage/Renderer.h"
#include "../SecondPage/Shader.h"
#include "../SecondPage/Model.h"
#include "../SecondPage/FrameResource.h"
#include "../Core/d3dUtil.h"
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <memory>
#include <unordered_map>

LRESULT CALLBACK
TestWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

#include <DirectXMath.h>

using namespace DirectX;

class CMaterial
{
public:
	CMaterial();
	~CMaterial() = default;

	CMaterial(const CMaterial&) = delete;
	CMaterial& operator=(const CMaterial&) = delete;

	void Build();
	UINT GetCount();

private:
	std::unordered_map<std::string, std::unique_ptr<Material>> m_materials{};
};

CMaterial::CMaterial()
{}

void CMaterial::Build()
{
	auto MakeMaterial = [&](std::string&& name, int matCBIdx, int diffuseSrvHeapIdx,
		XMFLOAT4 diffuseAlbedo, XMFLOAT3 fresnelR0, float rough) {
			auto curMat = std::make_unique<Material>();
			curMat->Name = name;
			curMat->MatCBIndex = matCBIdx;
			curMat->DiffuseSrvHeapIndex = diffuseSrvHeapIdx;
			curMat->DiffuseAlbedo = diffuseAlbedo;
			curMat->FresnelR0 = fresnelR0;
			curMat->Roughness = rough;
			m_materials[name] = std::move(curMat);
		};

	MakeMaterial("bricks0", 0, 0, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.002f, 0.002f, 0.02f }, 0.1f);
	MakeMaterial("stone0", 1, 1, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.3f);
	MakeMaterial("tile0", 2, 2, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.02f, 0.02f, 0.02f }, 0.3f);
	MakeMaterial("checkboard0", 3, 3, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f);
	MakeMaterial("ice0", 4, 4, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 0.0f);
	MakeMaterial("grass0", 5, 5, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f);
	MakeMaterial("skullMat", 6, 6, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.5f);
}

UINT CMaterial::GetCount()
{
	return static_cast<UINT>(m_materials.size());
}

void Load(MeshGeometry* meshGeo, CRenderer* renderer)
{
	meshGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(
		renderer->GetDevice(), renderer->GetCommandList(), 
		meshGeo->VertexBufferCPU->GetBufferPointer(), 
		meshGeo->VertexBufferByteSize,
		meshGeo->VertexBufferUploader);
	
	meshGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(
		renderer->GetDevice(), renderer->GetCommandList(), 
		meshGeo->IndexBufferCPU->GetBufferPointer(),
		meshGeo->IndexBufferByteSize,
		meshGeo->IndexBufferUploader);

	return;
}

namespace core
{
	TEST(Renderer, Initialize)
	{
		const std::wstring resourcePath = L"../Resource/";
		//초기화
		std::unique_ptr<CDirectx3D> directx3D = std::make_unique<CDirectx3D>(GetModuleHandle(nullptr));
		EXPECT_EQ(directx3D->Initialize(TestWndProc), true);

		std::shared_ptr<CRenderer> renderer = std::make_shared<CRenderer>(directx3D.get());
		EXPECT_EQ(renderer->Initialize(), true);

		//데이터를 시스템 메모리에 올리기
		std::unique_ptr<CMaterial> material = std::make_unique<CMaterial>();
		material->Build();

		std::unique_ptr<CTexture> texture = std::make_unique<CTexture>(resourcePath +L"Textures/");
		std::unique_ptr<CModel> model = std::make_unique<CModel>();

		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> geometries;
		auto geo = std::make_unique<MeshGeometry>();
		model->Read(geo.get());
		std::string geoName = geo->Name;
		geometries[geoName] = std::move(geo);

		//프레임당 쓰이는 데이터 공간을 확보
		const int gNumFrameResources = 3;
		std::vector<std::unique_ptr<FrameResource>> m_frameResources;
		for (auto i{ 0 }; i < gNumFrameResources; ++i)
		{
			auto frameRes = std::make_unique<FrameResource>(renderer->GetDevice(), 1,
				125, material->GetCount());
			m_frameResources.emplace_back(std::move(frameRes));
		}

		//시스템 메모리에서 그래픽 메모리에 데이터 올리기
		directx3D->ResetCommandLists();

		texture->Load(renderer.get());
		Load(geometries[geoName].get(), renderer.get());

		directx3D->ExcuteCommandLists();
		directx3D->FlushCommandQueue();

		std::unique_ptr<CCamera> camera = std::make_unique<CCamera>();
		camera->SetPosition(0.0f, 2.0f, -15.0f);
	}
}

