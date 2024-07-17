#include "Camera.h"
#include <ranges>
#include <algorithm>
#include "../Include/FrameResourceData.h"
#include "../Include/RenderItem.h"
#include "./MathHelper.h"
#include "./Utility.h"

using namespace DirectX;

inline XMFLOAT3 ConvertXMFLOAT3(const XMVECTOR& xmVec)
{
	XMFLOAT3 xmFloat{};
	XMStoreFloat3(&xmFloat, xmVec);
	return xmFloat;
}

CCamera::CCamera()
	: mView{ MathHelper::Identity4x4() }
	, mProj{ MathHelper::Identity4x4() }
{
	SetLens(0.25f*MathHelper::Pi, 1.0f, 1.0f, 1000.0f);
	SetPosition(0.0f, 2.0f, -15.0f);
	SetSpeed(10.0f);
}

CCamera::~CCamera()
{}

void CCamera::PressedKey(std::vector<int> keyList)
{
	std::map<int, eMove> usableKeyList{ 
		{'W', eMove::Forward}, {'S', eMove::Back}, {'D', eMove::Right}, {'A', eMove::Left} };
	
	auto middle = std::partition(keyList.begin(), keyList.end(),
		[&usableKeyList](auto key) { return usableKeyList.find(key) != usableKeyList.end(); });

	std::transform(keyList.begin(), middle, std::back_inserter(m_moveDirection),
		[&usableKeyList](auto key) { return usableKeyList[key]; });

	std::for_each(middle, keyList.end(), [this](auto key) {
		switch (key)
		{
			case '1':		m_frustumCullingEnabled = true;			break;
			case '2':		m_frustumCullingEnabled = false;		break;
		}});
}

void CCamera::SetPosition(float x, float y, float z)
{
	m_position = XMVectorSet(x, y, z, 1.0f);
	mViewDirty = true;
}

void CCamera::SetPosition(const XMFLOAT3& v)
{
	m_position = XMLoadFloat3(&v);
	mViewDirty = true;
}

XMFLOAT3 CCamera::GetPosition() const	{	return ConvertXMFLOAT3(m_position);	}
XMFLOAT3 CCamera::GetRight() const			{	return ConvertXMFLOAT3(m_right);			}
XMFLOAT3 CCamera::GetUp() const				{	return ConvertXMFLOAT3(m_up);				}
XMFLOAT3 CCamera::GetLook()const			{	return ConvertXMFLOAT3(m_look);			}

float CCamera::GetNearZ() const	{	return mNearZ;	}
float CCamera::GetFarZ() const		{	return mFarZ;		}
float CCamera::GetAspect() const	{	return mAspect;	}
float CCamera::GetFovY() const		{	return mFovY;		}
float CCamera::GetFovX() const
{
	float halfWidth = 0.5f*GetNearWindowWidth();
	return 2.0f*atanf(halfWidth / mNearZ);
}

float CCamera::GetNearWindowWidth() const	{	return mAspect * mNearWindowHeight;	}
float CCamera::GetNearWindowHeight() const	{	return mNearWindowHeight;					}
float CCamera::GetFarWindowWidth() const		{	return mAspect * mFarWindowHeight;		}
float CCamera::GetFarWindowHeight() const		{	return mFarWindowHeight;						}

void CCamera::SetLens(float fovY, float aspect, float zn, float zf)
{
	mFovY = fovY;
	mAspect = aspect;
	mNearZ = zn;
	mFarZ = zf;

	mNearWindowHeight = 2.0f * mNearZ * tanf( 0.5f*mFovY );
	mFarWindowHeight  = 2.0f * mFarZ * tanf( 0.5f*mFovY );

	XMMATRIX P = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
	BoundingFrustum::CreateFromMatrix(m_camFrustum, P);
	XMStoreFloat4x4(&mProj, P);
}

void CCamera::OnResize(int width, int height)
{
	auto aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	SetLens(0.25f * MathHelper::Pi, aspectRatio, 1.0f, 1000.f);
}

void CCamera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	m_look = XMVector3Normalize(XMVectorSubtract(target, pos));
	m_right = XMVector3Normalize(XMVector3Cross(worldUp, m_look));
	m_up = XMVector3Cross(m_look, m_right);
	m_position = pos;

	mViewDirty = true;
}

void CCamera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);

	mViewDirty = true;
}

XMMATRIX CCamera::GetView() const
{
	assert(!mViewDirty);
	return XMLoadFloat4x4(&mView);
}

XMMATRIX CCamera::GetProj() const
{
	return XMLoadFloat4x4(&mProj);
}

void CCamera::Strafe(float d)
{
	XMVECTOR s = XMVectorReplicate(d);
	m_position = XMVectorMultiplyAdd(s, m_right, m_position);

	mViewDirty = true;
}

void CCamera::Walk(float d)
{
	XMVECTOR s = XMVectorReplicate(d);
	m_position = XMVectorMultiplyAdd(s, m_look, m_position);

	mViewDirty = true;
}

void CCamera::Pitch(float angle)
{
	XMMATRIX R = XMMatrixRotationAxis(m_right, angle);

	m_up = XMVector3TransformNormal(m_up, R);
	m_look = XMVector3TransformNormal(m_look, R);

	mViewDirty = true;
}

void CCamera::Roll(float angle)
{
	XMMATRIX R = XMMatrixRotationAxis(m_look, angle);

	m_right = XMVector3TransformNormal(m_right, R);
	m_up = XMVector3TransformNormal(m_up, R);

	mViewDirty = true;
}

void CCamera::RotateY(float angle)
{
	XMMATRIX R = XMMatrixRotationY(angle);

	m_right = XMVector3TransformNormal(m_right, R);
	m_up = XMVector3TransformNormal(m_up, R);
	m_look = XMVector3TransformNormal(m_look, R);

	mViewDirty = true;
}

//방향에 따라 이동하는 것은 나중에 객체에서도 쓰일 예정이지만 일단 여기에 놔 두고 나중에 떼내자
void CCamera::SetSpeed(float speed)
{
	for (auto move{ EtoV(eMove::None) }; move < EtoV(eMove::Count); ++move)
	{
		SetSpeed(static_cast<eMove>(move), speed);
	}
}

void CCamera::SetSpeed(eMove move, float moveSpeed)
{
	m_moveSpeed.insert(std::make_pair(move, moveSpeed));
}

void CCamera::Move(eMove move, float speed)
{
	if (speed == 0.0f) return;

	switch (move)
	{
	case eMove::Forward:		Walk(speed);		break;
	case eMove::Back:			Walk(-speed);		break;
	case eMove::Right:			Strafe(speed);		break;
	case eMove::Left:			Strafe(-speed);	break;
	case eMove::Roll:			Roll(speed);			break;
	case eMove::Pitch:			Pitch(speed);		break;
	case eMove::RotateY:		RotateY(speed);	break;
	}
}

void CCamera::Move(float dx, float dy)
{
	Move(eMove::RotateY, dx);
	Move(eMove::Pitch, dy);
}
void StoreMatrix4x4dd(XMFLOAT4X4& dest, XMMATRIX src) { XMStoreFloat4x4(&dest, XMMatrixTranspose(src)); }
XMMATRIX Inversedd(XMMATRIX& m) { return XMMatrixInverse(nullptr, m); }
void CCamera::GetPassCB(PassConstants* pc)
{
	XMMATRIX view = GetView();
	XMMATRIX proj = GetProj();
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);
	XMMATRIX viewProjTex = XMMatrixMultiply(viewProj, T);

	XMStoreFloat4x4(&pc->view, XMMatrixTranspose(view));
	XMStoreFloat4x4(&pc->invView, XMMatrixInverse(nullptr, view));
	XMStoreFloat4x4(&pc->proj, XMMatrixTranspose(proj));
	//StoreMatrix4x4dd(pc->invProj, Inversedd(proj));
	XMStoreFloat4x4(&(pc->invProj), XMMatrixTranspose(XMMatrixInverse(nullptr, proj)));
	XMStoreFloat4x4(&pc->viewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&pc->invViewProj, XMMatrixInverse(nullptr, viewProj));
	XMStoreFloat4x4(&pc->viewProjTex, XMMatrixTranspose(viewProjTex));

	XMStoreFloat3(&pc->eyePosW, m_position);
}

void CCamera::Update(float deltaTime)
{
	for (auto moveDir : m_moveDirection)
	{
		Move(moveDir, deltaTime * m_moveSpeed[moveDir]);
	}
	m_moveDirection.clear();

	UpdateViewMatrix();
}

void CCamera::UpdateViewMatrix()
{
	if (mViewDirty == false)
		return;

	m_look = XMVector3Normalize(m_look);
	m_up = XMVector3Normalize(XMVector3Cross(m_look, m_right));
	m_right = XMVector3Cross(m_up, m_look);

	float x = -XMVectorGetX(XMVector3Dot(m_position, m_right));
	float y = -XMVectorGetX(XMVector3Dot(m_position, m_up));
	float z = -XMVectorGetX(XMVector3Dot(m_position, m_look));

	XMFLOAT3 right = GetRight();
	XMFLOAT3 up = GetUp();
	XMFLOAT3 look = GetLook();

	mView(0, 0) = right.x;
	mView(1, 0) = right.y;
	mView(2, 0) = right.z;
	mView(3, 0) = x;

	mView(0, 1) = up.x;
	mView(1, 1) = up.y;
	mView(2, 1) = up.z;
	mView(3, 1) = y;

	mView(0, 2) = look.x;
	mView(1, 2) = look.y;
	mView(2, 2) = look.z;
	mView(3, 2) = z;

	mView(0, 3) = 0.0f;
	mView(1, 3) = 0.0f;
	mView(2, 3) = 0.0f;
	mView(3, 3) = 1.0f;

	mViewDirty = false;
}

bool CCamera::IsInsideFrustum(const DirectX::BoundingSphere& bSphere, const XMMATRIX& invView, const XMMATRIX& world)
{
	BoundingFrustum frustum{};
	XMMATRIX invWorld = XMMatrixInverse(&RvToLv(XMMatrixDeterminant(world)), world);
	XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

	m_camFrustum.Transform(frustum, viewToLocal);

	const bool isInside = (frustum.Contains(bSphere) != DirectX::DISJOINT);
	return (isInside || !m_frustumCullingEnabled);
}

void CCamera::FindVisibleSubRenderItems(SubRenderItems& subRenderItems, InstanceDataList& visibleInstance)
{
	XMMATRIX view = GetView();
	XMMATRIX invView = XMMatrixInverse(&RvToLv(XMMatrixDeterminant(view)), view);

	int startSubIndex{ 0 };
	for (auto& iterSubItem : subRenderItems)
	{
		InstanceDataList curVisible{};
		auto& subRenderItem = iterSubItem.second;
		auto& instanceList = subRenderItem.instanceDataList;
		if (subRenderItem.cullingFrustum)
		{
			std::ranges::copy_if(instanceList, std::back_inserter(curVisible),
				[this, &invView, &subRenderItem](auto& instance) {
					return IsInsideFrustum(subRenderItem.subItem.boundingSphere, invView, instance->world);
				});
		}
		else
			std::ranges::copy(instanceList, std::back_inserter(curVisible));
		subRenderItem.startSubIndexInstance = startSubIndex;
		subRenderItem.instanceCount = static_cast<UINT>(curVisible.size());
		startSubIndex += subRenderItem.instanceCount;

		visibleInstance.insert(visibleInstance.end(), curVisible.begin(), curVisible.end()); 
	}
}


