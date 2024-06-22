﻿#pragma once

#include <vector>
#include <map>
#include <DirectXMath.h>

enum class eMove
{
	Forward = 1,
	Back,
	Right,
	Left,
	Roll,
	Pitch,
	RotateY,
};

class CCamera
{
public:
	CCamera();
	~CCamera();

	void PressedKey(std::vector<int> keyList);

	// Get/Set world camera position.
	DirectX::XMVECTOR GetPosition()const;
	DirectX::XMFLOAT3 GetPosition3f()const;
	void SetPosition(float x, float y, float z);
	void SetPosition(const DirectX::XMFLOAT3& v);
	
	// Get camera basis vectors.
	DirectX::XMVECTOR GetRight()const;
	DirectX::XMFLOAT3 GetRight3f()const;
	DirectX::XMVECTOR GetUp()const;
	DirectX::XMFLOAT3 GetUp3f()const;
	DirectX::XMVECTOR GetLook()const;
	DirectX::XMFLOAT3 GetLook3f()const;

	// Get frustum properties.
	float GetNearZ()const;
	float GetFarZ()const;
	float GetAspect()const;
	float GetFovY()const;
	float GetFovX()const;

	// Get near and far plane dimensions in view space coordinates.
	float GetNearWindowWidth()const;
	float GetNearWindowHeight()const;
	float GetFarWindowWidth()const;
	float GetFarWindowHeight()const;
	
	// Set frustum.
	void SetLens(float fovY, float aspect, float zn, float zf);

	// Define camera space via LookAt parameters.
	void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp);
	void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);

	// Get View/Proj matrices.
	DirectX::XMMATRIX GetView()const;
	DirectX::XMMATRIX GetProj()const;

	DirectX::XMFLOAT4X4 GetView4x4f()const;
	DirectX::XMFLOAT4X4 GetProj4x4f()const;

	// Strafe/Walk the camera a distance d.
	void Walk(float d);
	void Strafe(float d);
	
	// Rotate the camera.
	void Roll(float angle);
	void Pitch(float angle);
	void RotateY(float angle);

	void SetSpeed(float speed);
	void SetSpeed(eMove move, float moveSpeed);
	void Move(eMove move, bool forward, float deltaTime);
	void Move(eMove move, float speed);

	void Update(float deltaTime);
	// After modifying camera position/orientation, call to rebuild the view matrix.
	void UpdateViewMatrix();

private:

	// Camera coordinate system with coordinates relative to world space.
	DirectX::XMFLOAT3 mPosition{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 mRight{ 1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 mUp{ 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 mLook{ 0.0f, 0.0f, 1.0f };

	// Cache frustum properties.
	float mNearZ{ 0.0f };
	float mFarZ{ 0.0f };
	float mAspect{ 0.0f };
	float mFovY{ 0.0f };
	float mNearWindowHeight{ 0.0f };
	float mFarWindowHeight{ 0.0f };

	bool mViewDirty{ true };

	// Cache View/Proj matrices.
	DirectX::XMFLOAT4X4 mView{};
	DirectX::XMFLOAT4X4 mProj{};

	std::map<eMove, float> m_moveSpeed{};
	std::vector<eMove> m_moveDirection{};
};
