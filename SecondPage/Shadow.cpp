#include "Shadow.h"
#include <ranges>
#include "../Include/FrameResourceData.h"
#include "../Include/RendererDefine.h"

using namespace DirectX;

CShadow::CShadow()
{
	m_sceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_sceneBounds.Radius = sqrtf(10.0f * 10.0f + 15.0f * 15.0f);
};

CShadow::~CShadow() {};

void CShadow::Update(float deltaTime)
{
	UpdateLight(deltaTime);
	UpdateTransform();
}

void CShadow::UpdateLight(float deltaTime)
{
	m_lightRotationAngle += 0.1f * deltaTime;
	XMMATRIX rotation = XMMatrixRotationY(m_lightRotationAngle);
	for (auto i : std::views::iota(0, 3))
		m_rotatedLightDirections[i] = XMVector3TransformNormal(m_baseLightDirections[i], rotation);
}

void CShadow::UpdateTransform()
{
	XMVECTOR lightPos = -2.0f * m_sceneBounds.Radius * m_rotatedLightDirections[0];
	XMVECTOR targetPos = XMLoadFloat3(&m_sceneBounds.Center);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	XMFLOAT3 sphereCenterLS{};
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

	float l = sphereCenterLS.x - m_sceneBounds.Radius;
	float b = sphereCenterLS.y - m_sceneBounds.Radius;
	float n = sphereCenterLS.z - m_sceneBounds.Radius;
	float r = sphereCenterLS.x + m_sceneBounds.Radius;
	float t = sphereCenterLS.y + m_sceneBounds.Radius;
	float f = sphereCenterLS.z + m_sceneBounds.Radius;

	m_lightNearZ = n;
	m_lightFarZ = f;
	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	XMMATRIX ndcToTexture(	//-1, 1 => 0, 1·Î º¯È¯
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX shadow = lightView * lightProj * ndcToTexture;

	m_posW = lightPos;
	m_view = lightView;
	m_proj = lightProj;
	m_shadowTransform = shadow;
}

PassConstants CShadow::UpdatePassCB()
{
	float width = static_cast<float>(gShadowMapWidth);
	float height = static_cast<float>(gShadowMapHeight);

	XMMATRIX viewProj = XMMatrixMultiply(m_view, m_proj);

	PassConstants pc{};
	XMStoreFloat4x4(&pc.view, XMMatrixTranspose(m_view));
	XMStoreFloat4x4(&pc.invView, XMMatrixTranspose(XMMatrixInverse(nullptr, m_view)));
	XMStoreFloat4x4(&pc.proj, XMMatrixTranspose(m_proj));
	XMStoreFloat4x4(&pc.invProj, XMMatrixTranspose(XMMatrixInverse(nullptr, m_proj)));
	XMStoreFloat4x4(&pc.viewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&pc.invViewProj, XMMatrixTranspose(XMMatrixInverse(nullptr, viewProj)));
	XMStoreFloat3(&pc.eyePosW, m_posW);
	pc.renderTargetSize = XMFLOAT2(width, height);
	pc.invRenderTargetSize = XMFLOAT2(1.0f / width, 1.0f / height);
	pc.nearZ = m_lightNearZ;
	pc.farZ = m_lightFarZ;

	return pc;
}

void CShadow::GetPassCB(PassConstants* outPC)
{
	XMStoreFloat4x4(&outPC->shadowTransform, XMMatrixTranspose(m_shadowTransform));
	(*outPC).ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	for(auto i : std::views::iota(0, 3))
		XMStoreFloat3(&outPC->lights[i].direction, m_rotatedLightDirections[i]);
	(*outPC).lights[0].strength = { 0.9f, 0.8f, 0.7f };
	(*outPC).lights[1].strength = { 0.4f, 0.4f, 0.4f };
	(*outPC).lights[2].strength = { 0.2f, 0.2f, 0.2f };
	(*outPC).nearZ = 1.0f;
	(*outPC).farZ = 1000.0f;
}