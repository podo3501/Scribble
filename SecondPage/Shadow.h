#pragma once

struct PassConstants;

class CShadow
{
public:
	CShadow();
	~CShadow();

	void Update(float deltaTime);
	PassConstants UpdatePassCB();
	void GetPassCB(PassConstants* outPC);

private:
	void UpdateLight(float deltaTime);
	void UpdateTransform();

private:
	DirectX::BoundingSphere m_sceneBounds{};
	float m_lightRotationAngle{ 0 };
	const DirectX::XMVECTOR m_baseLightDirections[3]
	{
		DirectX::XMVECTOR{ 0.57735f, -0.57735f, 0.57735f },
		DirectX::XMVECTOR{ -0.57735f, -0.57735f, 0.57735f },
		DirectX::XMVECTOR{ 0.0f, -0.707f, -0.707f }
	};

	DirectX::XMVECTOR m_rotatedLightDirections[3]{};

	DirectX::XMVECTOR m_lightPosW{};
	float m_lightNearZ{ 0.0f };
	float m_lightFarZ{ 0.0f };

	DirectX::XMVECTOR m_posW{};
	DirectX::XMMATRIX m_view{};
	DirectX::XMMATRIX m_proj{};
	DirectX::XMMATRIX m_shadowTransform{};
};