#pragma once

#include <vector>
#include <map>
#include <DirectXMath.h>
#include <DirectXCollision.h>

struct PassConstants;

enum class eMove : int
{
	None,
	Forward,
	Back,
	Right,
	Left,
	Roll,
	Pitch,
	RotateY,
	Count,
};

class CCamera
{
public:
	CCamera();
	~CCamera();

	void PressedKey(std::vector<int> keyList);

	DirectX::XMFLOAT3 GetPosition() const;
	void SetPosition(float x, float y, float z);
	void SetPosition(const DirectX::XMFLOAT3& v);
	
	DirectX::XMFLOAT3 GetRight() const;
	DirectX::XMFLOAT3 GetUp() const;
	DirectX::XMFLOAT3 GetLook() const;

	float GetNearZ() const;
	float GetFarZ() const;
	float GetAspect() const;
	float GetFovY() const;
	float GetFovX() const;

	float GetNearWindowWidth() const;
	float GetNearWindowHeight() const;
	float GetFarWindowWidth() const;
	float GetFarWindowHeight() const;
	
	void SetLens(float fovY, float aspect, float zn, float zf);

	void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp);
	void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);

	DirectX::XMMATRIX GetView() const;
	DirectX::XMMATRIX GetProj() const;
	DirectX::BoundingFrustum GetFrustum() const;

	void Walk(float d);
	void Strafe(float d);

	void Roll(float angle);
	void Pitch(float angle);
	void RotateY(float angle);

	void SetSpeed(float speed);
	void SetSpeed(eMove move, float moveSpeed);
	void Move(eMove move, float speed);
	void Move(float dx, float dy);

	void GetPassCB(PassConstants* pc);

	void Update(float deltaTime);
	void UpdateViewMatrix();

private:
	DirectX::XMVECTOR m_position{ 0.0f, 0.0f, 0.0f };
	DirectX::XMVECTOR m_right{ 1.0f, 0.0f, 0.0f };
	DirectX::XMVECTOR m_up{ 0.0f, 1.0f, 0.0f };
	DirectX::XMVECTOR m_look{ 0.0f, 0.0f, 1.0f };

	float mNearZ{ 0.0f };
	float mFarZ{ 0.0f };
	float mAspect{ 0.0f };
	float mFovY{ 0.0f };
	float mNearWindowHeight{ 0.0f };
	float mFarWindowHeight{ 0.0f };

	bool mViewDirty{ true };

	DirectX::XMFLOAT4X4 mView{};
	DirectX::XMFLOAT4X4 mProj{};

	std::map<eMove, float> m_moveSpeed{};
	std::vector<eMove> m_moveDirection{};

	DirectX::BoundingFrustum m_camFrustum{};
};
