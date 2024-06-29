#include "Camera.h"
#include <ranges>
#include "../Core/d3dUtil.h"
#include "../Core/Utility.h"
#include "../Core/headerUtility.h"
#include "../SecondPage/FrameResourceData.h"

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
}

CCamera::~CCamera()
{}

void CCamera::PressedKey(std::vector<int> keyList)
{
	std::map<int, eMove> usableKeyList{ 
		{'W', eMove::Forward}, {'S', eMove::Back}, {'D', eMove::Right}, {'A', eMove::Left} };

	std::vector<int> moveKeyList{};
	std::ranges::copy_if(keyList, std::back_inserter(moveKeyList), 
		[&usableKeyList](auto key) { return usableKeyList.find(key) != usableKeyList.end(); });
	if (moveKeyList.empty()) return;

	std::ranges::transform(keyList, std::back_inserter(m_moveDirection),
		[&usableKeyList](auto key) { return usableKeyList[key]; });
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
	XMStoreFloat4x4(&mProj, P);
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

XMMATRIX CCamera::GetView()const
{
	assert(!mViewDirty);
	return XMLoadFloat4x4(&mView);
}

XMMATRIX CCamera::GetProj()const
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

void CCamera::GetPassCB(PassConstants* pc)
{
	XMMATRIX view = GetView();
	XMMATRIX proj = GetProj();
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	XMStoreFloat4x4(&pc->view, XMMatrixTranspose(view));
	XMStoreFloat4x4(&pc->invView, XMMatrixInverse(nullptr, view));
	XMStoreFloat4x4(&pc->proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&pc->invProj, XMMatrixInverse(nullptr, proj));
	XMStoreFloat4x4(&pc->viewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&pc->invViewProj, XMMatrixInverse(nullptr, viewProj));

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


