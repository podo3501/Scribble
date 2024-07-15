#pragma once
#include <vector>
#include <DirectXMath.h>
#include <wrl.h>

class CCamera;
struct SsaoConstants;

class CSsao
{
public:
	CSsao(UINT width, UINT height);
	~CSsao();

	void UpdatePassCB(CCamera* camera, SsaoConstants* ssaoCB);
	void GetOffsetVectors(DirectX::XMFLOAT4 offsets[14]);

private:
	std::vector<float> CalcGaussWeights(float sigma);
	void BuildOffsetVectors();
	UINT SsaoMapWidth() const;
	UINT SsaoMapHeight() const;

private:
	static const int MaxBlurRadius = 5;

	DirectX::XMFLOAT4 m_offsets[14]{};

	UINT m_renderTargetWidth{ 0 };
	UINT m_renderTargetHeight{ 0 };
};