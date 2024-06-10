#include "UserInterface.h"
#include "../Common/d3dUtil.h"
#include "../Common/Util.h"
#include "../Common/UploadBuffer.h"
#include "FrameResource.h"
#include "../Common/GeometryGenerator.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

const int gNumFrameResources = 3;

InstancingAndCullingApp::InstancingAndCullingApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

InstancingAndCullingApp::~InstancingAndCullingApp()
{
	if (md3dDevice != nullptr)
		FlushCommandQueue();
}

bool InstancingAndCullingApp::Initialize(WNDPROC wndProc)
{
	if (!D3DApp::Initialize(wndProc))
		return false;

	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	mCamera.SetPosition(0.0f, 2.0f, -15.0f);

	LoadTextures();
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	BuildSkullGeometry();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	BuildPSOs();

	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists2[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists2), cmdsLists2);

	FlushCommandQueue();

	return true;
}

void InstancingAndCullingApp::LoadTextures()
{
	std::vector<std::wstring> filenames{ L"bricks.dds", L"stone.dds", L"tile.dds", L"WoodCrate01.dds",
		L"ice.dds", L"grass.dds", L"white1x1.dds" };

	for_each(filenames.begin(), filenames.end(), [&](auto& curFilename) {
		auto tex = std::make_unique<Texture>();
		tex->Filename = L"../Resource/Textures/" + curFilename;
		ThrowIfFailed(CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(),
			tex->Filename.c_str(), tex->Resource, tex->UploadHeap));
		mTextures.emplace_back(std::move(tex));
		});
}

void InstancingAndCullingApp::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	//7인 이유는 텍스춰를 7장을 다 올린다음 동적으로 선택하기 위함이다.  Texture2D gDiffuseMap[7] : register(t0)
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 0, 0);

	CD3DX12_ROOT_PARAMETER slotRootParameter[4];
	slotRootParameter[0].InitAsShaderResourceView(0, 1);
	slotRootParameter[1].InitAsShaderResourceView(1, 1);
	slotRootParameter[2].InitAsConstantBufferView(0);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = d3dUtil::GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(slotRootParameter), slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serialized = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serialized.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void InstancingAndCullingApp::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = static_cast<UINT>(mTextures.size());
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	for_each(mTextures.begin(), mTextures.end(), [&, index{ 0 }](auto& curTex) mutable {
		auto& curTexRes = curTex->Resource;
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = curTexRes->GetDesc().Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = curTexRes->GetDesc().MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDesc{ mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };
		hCpuDesc.Offset(index++, mCbvSrvUavDescriptorSize);
		md3dDevice->CreateShaderResourceView(curTex->Resource.Get(), &srvDesc, hCpuDesc);
		});
}

void InstancingAndCullingApp::BuildShadersAndInputLayout()
{
	mShaders["standardVS"] = d3dUtil::CompileShader(L"../Resource/Shaders/VertexShader.hlsl", nullptr, "main", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"../Resource/Shaders/PixelShader.hlsl", nullptr, "main", "ps_5_1");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void InstancingAndCullingApp::BuildSkullGeometry()
{
	std::ifstream fin("../Resource/Models/skull.txt");
	if (fin.bad())
	{
		MessageBox(0, L"Models/Skull.txt not found", 0, 0);
		return;
	}

	UINT vCount = 0;
	UINT iCount = 0;
	std::string ignore;
	fin >> ignore >> vCount;
	fin >> ignore >> iCount;
	fin >> ignore >> ignore >> ignore >> ignore;

	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

	std::vector<Vertex> vertices(vCount);
	for (auto i : Range(0, vCount))
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;

		XMVECTOR P = XMLoadFloat3(&vertices[i].Pos);

		XMFLOAT3 spherePos;
		XMStoreFloat3(&spherePos, XMVector3Normalize(P));

		float theta = atan2f(spherePos.z, spherePos.x);

		if (theta < 0.0f) theta += XM_2PI;

		float phi = acosf(spherePos.y);

		float u = theta / (2.0f * XM_PI);
		float v = phi / XM_PI;

		vertices[i].TexC = { u, v };

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	BoundingBox boundingBoxBounds;
	XMStoreFloat3(&boundingBoxBounds.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&boundingBoxBounds.Extents, 0.5f * (vMax - vMin));

	BoundingSphere boundingSphere;
	XMStoreFloat3(&boundingSphere.Center, 0.5f * (vMin + vMax));
	boundingSphere.Radius = XMVectorGetX(XMVector3Length(0.5f * (vMax - vMin)));

	fin >> ignore >> ignore >> ignore;

	std::vector<std::int32_t> indices(iCount * 3);
	for (auto i : Range(0, iCount))
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}
	fin.close();

	UINT vbByteSize = static_cast<UINT>(vertices.size()) * sizeof(Vertex);
	UINT ibByteSize = static_cast<UINT>(indices.size()) * sizeof(std::int32_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "skullGeo";

	auto& submesh = geo->DrawArgs["skull"];
	submesh.IndexCount = static_cast<UINT>(indices.size());
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	submesh.BBounds = boundingBoxBounds;
	submesh.BSphere = boundingSphere;

	geo->VertexBufferByteSize = vbByteSize;
	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(
		md3dDevice.Get(), mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;
	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(
		md3dDevice.Get(), mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	mGeometries[geo->Name] = std::move(geo);
}

void InstancingAndCullingApp::BuildMaterials()
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
			mMaterials[name] = std::move(curMat);
		};

	MakeMaterial("bricks0", 0, 0, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.002f, 0.002f, 0.02f }, 0.1f);
	MakeMaterial("stone0", 1, 1, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.3f);
	MakeMaterial("tile0", 2, 2, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.02f, 0.02f, 0.02f }, 0.3f);
	MakeMaterial("checkboard0", 3, 3, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f);
	MakeMaterial("ice0", 4, 4, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 0.0f);
	MakeMaterial("grass0", 5, 5, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f);
	MakeMaterial("skullMat", 6, 6, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.5f);
}


void InstancingAndCullingApp::BuildRenderItems()
{
	auto renderItem = std::make_unique<RenderItem>();
	auto MakeRenderItem = [&, objIdx{ 0 }](std::string&& geoName, std::string&& smName, std::string&& matName,
		const XMMATRIX& world, const XMMATRIX& texTransform) mutable {
			auto& sm = mGeometries[geoName]->DrawArgs[smName];
			renderItem->Geo = mGeometries[geoName].get();
			renderItem->Mat = mMaterials[matName].get();
			renderItem->ObjCBIndex = objIdx++;
			renderItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			XMStoreFloat4x4(&renderItem->World, world);
			XMStoreFloat4x4(&renderItem->TexTransform, texTransform);
			renderItem->StartIndexLocation = sm.StartIndexLocation;
			renderItem->BaseVertexLocation = sm.BaseVertexLocation;
			renderItem->IndexCount = sm.IndexCount;
			renderItem->BoundingBoxBounds = sm.BBounds;
			renderItem->BoundingSphere = sm.BSphere; };
	MakeRenderItem("skullGeo", "skull", "tile0", XMMatrixIdentity(), XMMatrixIdentity());

	const int n = 5;
	renderItem->Instances.resize(n * n * n);

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;

	float x = -0.5f * width;
	float y = -0.5f * height;
	float z = -0.5f * depth;
	float dx = width / (n - 1);
	float dy = height / (n - 1);
	float dz = depth / (n - 1);
	for (int k = 0; k < n; ++k)
	{
		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				int index = k * n * n + i * n + j;
				renderItem->Instances[index].World = XMFLOAT4X4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x + j * dx, y + i * dy, z + k * dz, 1.0f);

				XMStoreFloat4x4(&renderItem->Instances[index].TexTransform, XMMatrixScaling(2.0f, 2.0f, 1.0f));
				renderItem->Instances[index].MaterialIndex = index % mMaterials.size();
			}
		}
	}

	mAllRitems.emplace_back(std::move(renderItem));

	for (auto& e : mAllRitems)
		mOpaqueRitems.emplace_back(e.get());
}

void InstancingAndCullingApp::BuildFrameResources()
{
	for (auto i : Range(0, gNumFrameResources))
	{
		auto frameRes = std::make_unique<FrameResource>(md3dDevice.Get(), 1,
			125, static_cast<UINT>(mMaterials.size()));
		mFrameResources.emplace_back(std::move(frameRes));
	}
}

D3D12_SHADER_BYTECODE GetShaderBytecode(
	std::unordered_map<std::string, ComPtr<ID3DBlob>>& shaders, std::string&& name)
{
	return { shaders[name]->GetBufferPointer(), shaders[name]->GetBufferSize() };
}

void InstancingAndCullingApp::MakeOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{
	inoutDesc->VS = GetShaderBytecode(mShaders, "standardVS");
	inoutDesc->PS = GetShaderBytecode(mShaders, "opaquePS");
	inoutDesc->NodeMask = 0;
	inoutDesc->SampleMask = UINT_MAX;
	inoutDesc->NumRenderTargets = 1;
	inoutDesc->InputLayout = { mInputLayout.data(), static_cast<UINT>(mInputLayout.size()) };
	inoutDesc->pRootSignature = mRootSignature.Get();
	inoutDesc->BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	inoutDesc->DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	inoutDesc->RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	inoutDesc->RTVFormats[0] = mBackBufferFormat;
	inoutDesc->DSVFormat = mDepthStencilFormat;
	inoutDesc->PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	inoutDesc->SampleDesc.Count = m4xMsaaState ? 4 : 1;
	inoutDesc->SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
}

void InstancingAndCullingApp::MakePSOPipelineState(GraphicsPSO psoType)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	MakeOpaqueDesc(&psoDesc);

	switch (psoType)
	{
	case GraphicsPSO::Opaque:						break;
	default: assert(!"wrong type");
	}

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSOs[GraphicsPSO::Opaque])));
}

void InstancingAndCullingApp::BuildPSOs()
{
	for (auto gPso : GraphicsPSO_ALL)
		MakePSOPipelineState(gPso);
}

void InstancingAndCullingApp::OnResize()
{
	D3DApp::OnResize();

	mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.f);

	BoundingFrustum::CreateFromMatrix(mCamFrustum, mCamera.GetProj());
}

void InstancingAndCullingApp::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();

	float speed = 10.0f;
	float angle = 0.2f;
	float walkSpeed = 0.0f;
	float strafeSpeed = 0.0f;

	std::vector<int> keyList{ 'W', 'S', 'D', 'A', '1', '2' };
	for_each(keyList.begin(), keyList.end(), [&](int vKey) {
		bool bPressed = GetAsyncKeyState(vKey) & 0x8000;
		if (bPressed)
		{
			switch (vKey)
			{
			case 'W':		walkSpeed += speed;		break;
			case 'S':		walkSpeed += -speed;		break;
			case 'D':		strafeSpeed += speed;		break;
			case 'A':		strafeSpeed += -speed;		break;
			case '1':		mFrustumCullingEnabled = true;			break;
			case '2':		mFrustumCullingEnabled = false;			break;
			}
		}});

	mCamera.Move(Camera::eWalk, walkSpeed * dt);
	mCamera.Move(Camera::eStrafe, strafeSpeed * dt);

	mCamera.UpdateViewMatrix();
}

void InstancingAndCullingApp::AnimateMaterials(const GameTimer& gt)
{
}

void StoreMatrix4x4(XMFLOAT4X4& dest, XMFLOAT4X4& src) { XMStoreFloat4x4(&dest, XMMatrixTranspose(XMLoadFloat4x4(&src))); }
void StoreMatrix4x4(XMFLOAT4X4& dest, XMMATRIX src) { XMStoreFloat4x4(&dest, XMMatrixTranspose(src)); }
XMMATRIX Multiply(XMFLOAT4X4& m1, XMFLOAT4X4 m2) { return XMMatrixMultiply(XMLoadFloat4x4(&m1), XMLoadFloat4x4(&m2)); }
XMMATRIX Inverse(XMMATRIX& m) { return XMMatrixInverse(nullptr, m); }
XMMATRIX Inverse(XMFLOAT4X4& src) { return Inverse(RvToLv(XMLoadFloat4x4(&src))); }
//
void InstancingAndCullingApp::UpdateInstanceData(const GameTimer& gt)
{
	XMMATRIX view = mCamera.GetView();
	XMMATRIX invView = XMMatrixInverse(&RvToLv(XMMatrixDeterminant(view)), view);

	auto currInstanceBuffer = mCurFrameRes->InstanceBuffer.get();
	for (auto& e : mAllRitems)
	{
		const auto& instanceData = e->Instances;

		int visibleInstanceCount = 0;

		for (UINT i = 0; i < (UINT)instanceData.size(); ++i)
		{
			XMMATRIX world = XMLoadFloat4x4(&instanceData[i].World);
			XMMATRIX texTransform = XMLoadFloat4x4(&instanceData[i].TexTransform);

			XMMATRIX invWorld = XMMatrixInverse(&RvToLv(XMMatrixDeterminant(world)), world);

			// View space to the object's local space.
			XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

			// Transform the camera frustum from view space to the object's local space.
			BoundingFrustum localSpaceFrustum;
			mCamFrustum.Transform(localSpaceFrustum, viewToLocal);

			// Perform the box/frustum intersection test in local space.
			//if ((localSpaceFrustum.Contains(e->BoundingBoxBounds) != DirectX::DISJOINT) || (mFrustumCullingEnabled == false))
			if ((localSpaceFrustum.Contains(e->BoundingSphere) != DirectX::DISJOINT) || (mFrustumCullingEnabled == false))
			{
				InstanceData data;
				XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
				data.MaterialIndex = instanceData[i].MaterialIndex;

				// Write the instance data to structured buffer for the visible objects.
				currInstanceBuffer->CopyData(visibleInstanceCount++, data);
			}
		}

		e->InstanceCount = visibleInstanceCount;

		std::wostringstream outs;
		outs.precision(6);
		outs << L"Instancing and Culling Demo" <<
			L"    " << e->InstanceCount <<
			L" objects visible out of " << e->Instances.size();
		mMainWndCaption = outs.str();
	}
}

void InstancingAndCullingApp::UpdateMaterialBuffer(const GameTimer& gt)
{
	auto curMaterialBuf = mCurFrameRes->MaterialBuffer.get();
	for (auto& e : mMaterials)
	{
		Material* m = e.second.get();
		if (m->NumFramesDirty <= 0)
			continue;

		XMMATRIX matTransform = XMLoadFloat4x4(&m->MatTransform);

		MaterialData matData;
		matData.DiffuseAlbedo = m->DiffuseAlbedo;
		matData.FresnelR0 = m->FresnelR0;
		matData.Roughness = m->Roughness;
		XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
		matData.DiffuseMapIndex = m->DiffuseSrvHeapIndex;

		curMaterialBuf->CopyData(m->MatCBIndex, matData);

		m->NumFramesDirty--;
	}
}

void InstancingAndCullingApp::UpdateMainPassCB(const GameTimer& gt)
{
	auto& passCB = mCurFrameRes->PassCB;
	XMMATRIX view = mCamera.GetView();
	XMMATRIX proj = mCamera.GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	PassConstants pc;
	StoreMatrix4x4(pc.View, view);
	StoreMatrix4x4(pc.InvView, Inverse(view));
	StoreMatrix4x4(pc.Proj, proj);
	StoreMatrix4x4(pc.InvProj, Inverse(proj));
	StoreMatrix4x4(pc.ViewProj, viewProj);
	StoreMatrix4x4(pc.InvViewProj, Inverse(viewProj));
	pc.EyePosW = mCamera.GetPosition3f();
	pc.RenderTargetSize = { (float)mClientWidth, (float)mClientHeight };
	pc.InvRenderTargetSize = { 1.0f / (float)mClientWidth, 1.0f / (float)mClientHeight };
	pc.NearZ = 1.0f;
	pc.FarZ = 1000.0f;
	pc.TotalTime = gt.TotalTime();
	pc.DeltaTime = gt.DeltaTime();
	pc.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	pc.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	pc.Lights[0].Strength = { 0.8f, 0.8f, 0.8f };
	pc.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	pc.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
	pc.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	pc.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

	passCB->CopyData(0, pc);
}

void InstancingAndCullingApp::Update(const GameTimer& gt)
{
	OnKeyboardInput(gt);

	mFrameResIdx = (mFrameResIdx + 1) % gNumFrameResources;
	mCurFrameRes = mFrameResources[mFrameResIdx].get();
	if (mCurFrameRes->Fence != 0 && mFence->GetCompletedValue() < mCurFrameRes->Fence)
	{
		HANDLE hEvent = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurFrameRes->Fence, hEvent));
		WaitForSingleObject(hEvent, INFINITE);
		CloseHandle(hEvent);
	}

	AnimateMaterials(gt);
	UpdateInstanceData(gt);
	UpdateMaterialBuffer(gt);
	UpdateMainPassCB(gt);
}

void InstancingAndCullingApp::DrawRenderItems(const std::vector<RenderItem*>& ritems)
{
	for (auto& ri : ritems)
	{
		mCommandList->IASetVertexBuffers(0, 1, &RvToLv(ri->Geo->VertexBufferView()));
		mCommandList->IASetIndexBuffer(&RvToLv(ri->Geo->IndexBufferView()));
		mCommandList->IASetPrimitiveTopology(ri->PrimitiveType);

		auto instanceBuffer = mCurFrameRes->InstanceBuffer->Resource();
		mCommandList->SetGraphicsRootShaderResourceView(0, instanceBuffer->GetGPUVirtualAddress());

		mCommandList->DrawIndexedInstanced(ri->IndexCount, ri->InstanceCount,
			ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}

void InstancingAndCullingApp::DrawRenderItems(FrameResource* pCurrFrameRes, const std::vector<RenderItem*>& ritems)
{
	for (auto& ri : ritems)
	{
		mCommandList->IASetVertexBuffers(0, 1, &RvToLv(ri->Geo->VertexBufferView()));
		mCommandList->IASetIndexBuffer(&RvToLv(ri->Geo->IndexBufferView()));
		mCommandList->IASetPrimitiveTopology(ri->PrimitiveType);

		auto instanceBuffer = pCurrFrameRes->InstanceBuffer->Resource();
		mCommandList->SetGraphicsRootShaderResourceView(0, instanceBuffer->GetGPUVirtualAddress());

		mCommandList->DrawIndexedInstanced(ri->IndexCount, ri->InstanceCount,
			ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}

void InstancingAndCullingApp::Draw(
	const GameTimer& gt,
	FrameResource* pCurrFrameRes,
	std::vector<RenderItem*> renderItem)
{
	auto cmdListAlloc = pCurrFrameRes->CmdListAlloc;
	ThrowIfFailed(cmdListAlloc->Reset());
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs[GraphicsPSO::Opaque].Get()));

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	mCommandList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)));

	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	mCommandList->OMSetRenderTargets(1, &RvToLv(CurrentBackBufferView()), true, &RvToLv(DepthStencilView()));

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	auto matBuf = pCurrFrameRes->MaterialBuffer->Resource();
	mCommandList->SetGraphicsRootShaderResourceView(1, matBuf->GetGPUVirtualAddress());

	auto passCB = pCurrFrameRes->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	mCommandList->SetGraphicsRootDescriptorTable(3, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	DrawRenderItems(pCurrFrameRes, renderItem);

	mCommandList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));
	ThrowIfFailed(mCommandList->Close());

	ID3D12CommandList* ppCommandLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	pCurrFrameRes->Fence = ++mCurrentFence;
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}


void InstancingAndCullingApp::Draw(const GameTimer& gt)
{
	auto cmdListAlloc = mCurFrameRes->CmdListAlloc;
	ThrowIfFailed(cmdListAlloc->Reset());
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs[GraphicsPSO::Opaque].Get()));

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	mCommandList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)));

	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	mCommandList->OMSetRenderTargets(1, &RvToLv(CurrentBackBufferView()), true, &RvToLv(DepthStencilView()));

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	auto matBuf = mCurFrameRes->MaterialBuffer->Resource();
	mCommandList->SetGraphicsRootShaderResourceView(1, matBuf->GetGPUVirtualAddress());

	auto passCB = mCurFrameRes->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	mCommandList->SetGraphicsRootDescriptorTable(3, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	DrawRenderItems(mOpaqueRitems);

	mCommandList->ResourceBarrier(1, &RvToLv(CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));
	ThrowIfFailed(mCommandList->Close());

	ID3D12CommandList* ppCommandLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	mCurFrameRes->Fence = ++mCurrentFence;
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void InstancingAndCullingApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}
void InstancingAndCullingApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}
void InstancingAndCullingApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		mCamera.Move(Camera::ePitch, dy);
		mCamera.Move(Camera::eRotateY, dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

///////////////////////////////////////////

Model::Model(std::shared_ptr<Renderer>& renderer)
	: m_renderer(renderer)
	, mGeometries()
	, m_materials()
	, mAllRitems()
	, mOpaqueRitems()
{
	LoadSkull();
	BuildMaterials();
	BuildRenderItems();
}

UINT Model::GetMaterialsCount()
{
	return static_cast<UINT>(m_materials.size());
}

std::vector<RenderItem*> Model::GetRenderItems()
{
	return mOpaqueRitems;
}

void Model::LoadSkull()
{
	std::ifstream fin("../Resource/Models/skull.txt");
	if (fin.bad())
	{
		MessageBox(0, L"Models/Skull.txt not found", 0, 0);
		return;
	}

	UINT vCount = 0;
	UINT iCount = 0;
	std::string ignore;
	fin >> ignore >> vCount;
	fin >> ignore >> iCount;
	fin >> ignore >> ignore >> ignore >> ignore;

	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

	std::vector<Vertex> vertices(vCount);
	for (auto i : Range(0, vCount))
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;

		XMVECTOR P = XMLoadFloat3(&vertices[i].Pos);

		XMFLOAT3 spherePos;
		XMStoreFloat3(&spherePos, XMVector3Normalize(P));

		float theta = atan2f(spherePos.z, spherePos.x);

		if (theta < 0.0f) theta += XM_2PI;

		float phi = acosf(spherePos.y);

		float u = theta / (2.0f * XM_PI);
		float v = phi / XM_PI;

		vertices[i].TexC = { u, v };

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	BoundingBox boundingBoxBounds;
	XMStoreFloat3(&boundingBoxBounds.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&boundingBoxBounds.Extents, 0.5f * (vMax - vMin));

	BoundingSphere boundingSphere;
	XMStoreFloat3(&boundingSphere.Center, 0.5f * (vMin + vMax));
	boundingSphere.Radius = XMVectorGetX(XMVector3Length(0.5f * (vMax - vMin)));

	fin >> ignore >> ignore >> ignore;

	std::vector<std::int32_t> indices(iCount * 3);
	for (auto i : Range(0, iCount))
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}
	fin.close();

	UINT vbByteSize = static_cast<UINT>(vertices.size()) * sizeof(Vertex);
	UINT ibByteSize = static_cast<UINT>(indices.size()) * sizeof(std::int32_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "skullGeo";

	auto& submesh = geo->DrawArgs["skull"];
	submesh.IndexCount = static_cast<UINT>(indices.size());
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	submesh.BBounds = boundingBoxBounds;
	submesh.BSphere = boundingSphere;
	
	geo->VertexBufferByteSize = vbByteSize;
	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(
		m_renderer->GetDevice(), m_renderer->GetCommandList(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;
	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(
		m_renderer->GetDevice(), m_renderer->GetCommandList(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	mGeometries[geo->Name] = std::move(geo);
}

void Model::BuildMaterials()
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

void Model::BuildRenderItems()
{
	auto renderItem = std::make_unique<RenderItem>();
	auto MakeRenderItem = [&, objIdx{ 0 }](std::string&& geoName, std::string&& smName, std::string&& matName,
		const XMMATRIX& world, const XMMATRIX& texTransform) mutable {
			auto& sm = mGeometries[geoName]->DrawArgs[smName];
			renderItem->Geo = mGeometries[geoName].get();
			renderItem->Mat = m_materials[matName].get();
			renderItem->ObjCBIndex = objIdx++;
			renderItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			XMStoreFloat4x4(&renderItem->World, world);
			XMStoreFloat4x4(&renderItem->TexTransform, texTransform);
			renderItem->StartIndexLocation = sm.StartIndexLocation;
			renderItem->BaseVertexLocation = sm.BaseVertexLocation;
			renderItem->IndexCount = sm.IndexCount;
			renderItem->BoundingBoxBounds = sm.BBounds;
			renderItem->BoundingSphere = sm.BSphere; };
	MakeRenderItem("skullGeo", "skull", "tile0", XMMatrixIdentity(), XMMatrixIdentity());

	const int n = 5;
	renderItem->Instances.resize(n * n * n);

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;

	float x = -0.5f * width;
	float y = -0.5f * height;
	float z = -0.5f * depth;
	float dx = width / (n - 1);
	float dy = height / (n - 1);
	float dz = depth / (n - 1);
	for (int k = 0; k < n; ++k)
	{
		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				int index = k * n * n + i * n + j;
				renderItem->Instances[index].World = XMFLOAT4X4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x + j * dx, y + i * dy, z + k * dz, 1.0f);

				XMStoreFloat4x4(&renderItem->Instances[index].TexTransform, XMMatrixScaling(2.0f, 2.0f, 1.0f));
				renderItem->Instances[index].MaterialIndex = index % m_materials.size();
			}
		}
	}

	mAllRitems.emplace_back(std::move(renderItem));

	for (auto& e : mAllRitems)
		mOpaqueRitems.emplace_back(e.get());
}

void Model::UpdateMaterialBuffer(FrameResource* curFrameRes)
{
	auto curMaterialBuf = curFrameRes->MaterialBuffer.get();
	for (auto& e : m_materials)
	{
		Material* m = e.second.get();
		if (m->NumFramesDirty <= 0)
			continue;

		XMMATRIX matTransform = XMLoadFloat4x4(&m->MatTransform);

		MaterialData matData;
		matData.DiffuseAlbedo = m->DiffuseAlbedo;
		matData.FresnelR0 = m->FresnelR0;
		matData.Roughness = m->Roughness;
		XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
		matData.DiffuseMapIndex = m->DiffuseSrvHeapIndex;

		curMaterialBuf->CopyData(m->MatCBIndex, matData);

		m->NumFramesDirty--;
	}
}

void Model::Update(const GameTimer& gt, 
	const Camera& camera, 
	FrameResource* curFrameRes,
	DirectX::BoundingFrustum& camFrustum,
	bool frustumCullingEnabled)
{
	XMMATRIX view = camera.GetView();
	XMMATRIX invView = XMMatrixInverse(&RvToLv(XMMatrixDeterminant(view)), view);

	auto currInstanceBuffer = curFrameRes->InstanceBuffer.get();
	for (auto& e : mAllRitems)
	{
		const auto& instanceData = e->Instances;

		int visibleInstanceCount = 0;

		for (UINT i = 0; i < (UINT)instanceData.size(); ++i)
		{
			XMMATRIX world = XMLoadFloat4x4(&instanceData[i].World);
			XMMATRIX texTransform = XMLoadFloat4x4(&instanceData[i].TexTransform);

			XMMATRIX invWorld = XMMatrixInverse(&RvToLv(XMMatrixDeterminant(world)), world);

			// View space to the object's local space.
			XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

			// Transform the camera frustum from view space to the object's local space.
			BoundingFrustum localSpaceFrustum;
			camFrustum.Transform(localSpaceFrustum, viewToLocal);

			// Perform the box/frustum intersection test in local space.
			//if ((localSpaceFrustum.Contains(e->BoundingBoxBounds) != DirectX::DISJOINT) || (mFrustumCullingEnabled == false))
			if ((localSpaceFrustum.Contains(e->BoundingSphere) != DirectX::DISJOINT) || (frustumCullingEnabled == false))
			{
				InstanceData data;
				XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
				data.MaterialIndex = instanceData[i].MaterialIndex;

				// Write the instance data to structured buffer for the visible objects.
				currInstanceBuffer->CopyData(visibleInstanceCount++, data);
			}
		}

		e->InstanceCount = visibleInstanceCount;

		//std::wostringstream outs;
		//outs.precision(6);
		//outs << L"Instancing and Culling Demo" <<
		//	L"    " << e->InstanceCount <<
		//	L" objects visible out of " << e->Instances.size();
		//mMainWndCaption = outs.str();
	}
}

////////////////////////////////////////////

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return MainLoop::GetMainLoop()->MsgProc(hwnd, msg, wParam, lParam);
}

MainLoop* MainLoop::g_mainLoop = nullptr;
MainLoop* MainLoop::GetMainLoop()
{
	return g_mainLoop;
}


LRESULT MainLoop::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			mAppPaused = true;
			m_timer.Stop();
		}
		else
		{
			mAppPaused = false;
			m_timer.Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		mClientWidth = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if (md3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (mMinimized)
				{
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if (mMaximized)
				{
					mAppPaused = false;
					mMaximized = false;
					OnResize();
				}
				else if (mResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		mAppPaused = true;
		mResizing = true;
		m_timer.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing = false;
		mTimer.Start();
		OnResize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		else if ((int)wParam == VK_F2)
			m_renderer->Set4xMsaaState(!m4xMsaaState);

		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

MainLoop::MainLoop(HINSTANCE hInstance)
{
	g_mainLoop = this;
	m_renderer = std::make_shared<InstancingAndCullingApp>(hInstance);
	auto result = m_renderer->Initialize(nullptr);
	m_model = std::make_shared<Model>(m_renderer);
}

void MainLoop::OnResize()
{
	auto aspectRatio = 
		static_cast<float>(m_renderer->GetClientWidth()) / 
		static_cast<float>(m_renderer->GetClientHeight());
	m_camera.SetLens(0.25f * MathHelper::Pi, aspectRatio, 1.0f, 1000.f);

	BoundingFrustum::CreateFromMatrix(m_camFrustum, m_camera.GetProj());
}

void MainLoop::CalculateFrameStats()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ((m_timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);

		std::wstring windowText = m_mainWndCaption +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr;

		SetWindowText(m_renderer->GetMainWnd(), windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void MainLoop::BuildFrameResources()
{
	for (auto i : Range(0, gNumFrameResources))
	{
		auto frameRes = std::make_unique<FrameResource>(m_renderer->GetDevice(), 1,
			125, m_model->GetMaterialsCount());
		m_frameResources.emplace_back(std::move(frameRes));
	}
}

void MainLoop::OnKeyboardInput()
{
	const float dt = m_timer.DeltaTime();

	float speed = 10.0f;
	float angle = 0.2f;
	float walkSpeed = 0.0f;
	float strafeSpeed = 0.0f;

	std::vector<int> keyList{ 'W', 'S', 'D', 'A', '1', '2' };
	for_each(keyList.begin(), keyList.end(), [&](int vKey) {
		bool bPressed = GetAsyncKeyState(vKey) & 0x8000;
		if (bPressed)
		{
			switch (vKey)
			{
			case 'W':		walkSpeed += speed;		break;
			case 'S':		walkSpeed += -speed;		break;
			case 'D':		strafeSpeed += speed;		break;
			case 'A':		strafeSpeed += -speed;		break;
			case '1':		m_frustumCullingEnabled = true;			break;
			case '2':		m_frustumCullingEnabled = false;		break;
			}
		}});

	m_camera.Move(Camera::eWalk, walkSpeed * dt);
	m_camera.Move(Camera::eStrafe, strafeSpeed * dt);

	m_camera.UpdateViewMatrix();
}

void MainLoop::UpdateMainPassCB()
{
	auto& passCB = m_curFrameRes->PassCB;
	XMMATRIX view = m_camera.GetView();
	XMMATRIX proj = m_camera.GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	PassConstants pc;
	StoreMatrix4x4(pc.View, view);
	StoreMatrix4x4(pc.InvView, Inverse(view));
	StoreMatrix4x4(pc.Proj, proj);
	StoreMatrix4x4(pc.InvProj, Inverse(proj));
	StoreMatrix4x4(pc.ViewProj, viewProj);
	StoreMatrix4x4(pc.InvViewProj, Inverse(viewProj));
	pc.EyePosW = m_camera.GetPosition3f();

	auto clientWidth = static_cast<float>(m_renderer->GetClientWidth());
	auto clientHeight = static_cast<float>(m_renderer->GetClientHeight());

	pc.RenderTargetSize = { clientWidth, clientHeight };
	pc.InvRenderTargetSize = { 1.0f / clientWidth, 1.0f / clientHeight };
	pc.NearZ = 1.0f;
	pc.FarZ = 1000.0f;
	pc.TotalTime = m_timer.TotalTime();
	pc.DeltaTime = m_timer.DeltaTime();
	pc.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	pc.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	pc.Lights[0].Strength = { 0.8f, 0.8f, 0.8f };
	pc.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	pc.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
	pc.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	pc.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

	passCB->CopyData(0, pc);
}

int MainLoop::Run()
{
	m_camera.SetPosition(0.0f, 2.0f, -15.0f);
	BuildFrameResources();

	MSG msg = { 0 };
	m_timer.Reset();
	
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_timer.Tick();

			if (!m_appPaused)
			{
				CalculateFrameStats();

				OnKeyboardInput();

				ID3D12Fence* pFence = m_renderer->GetFence();
				m_frameResIdx = (m_frameResIdx + 1) % gNumFrameResources;
				m_curFrameRes = m_frameResources[m_frameResIdx].get();
				if (m_curFrameRes->Fence != 0 && pFence->GetCompletedValue() < m_curFrameRes->Fence)
				{
					HANDLE hEvent = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
					ThrowIfFailed(pFence->SetEventOnCompletion(m_curFrameRes->Fence, hEvent));
					WaitForSingleObject(hEvent, INFINITE);
					CloseHandle(hEvent);
				}

				m_model->Update(m_timer, m_camera, m_curFrameRes, m_camFrustum, m_frustumCullingEnabled);
				m_model->UpdateMaterialBuffer(m_curFrameRes);

				UpdateMainPassCB();

				m_renderer->Draw(m_timer, m_curFrameRes, m_model->GetRenderItems());
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}