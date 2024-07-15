#include "./Ssao.h"
#include <DirectXMath.h>
#include <ranges>
#include <assert.h>
#include "../Include/FrameResourceData.h"
#include "./Camera.h"

using namespace DirectX;

CSsao::~CSsao() = default;
CSsao::CSsao(UINT width, UINT height)
	: m_renderTargetWidth(width)
	, m_renderTargetHeight(height)
{
	BuildOffsetVectors();
}

UINT CSsao::SsaoMapWidth() const
{
	return m_renderTargetWidth / 2;
}

UINT CSsao::SsaoMapHeight() const
{
	return m_renderTargetHeight / 2;
}

void CSsao::UpdatePassCB(CCamera* camera, SsaoConstants* ssaoCB)
{
	XMMATRIX P = camera->GetProj();
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);
	XMStoreFloat4x4(&ssaoCB->projTex, XMMatrixTranspose(P * T));

	GetOffsetVectors(ssaoCB->offsetVectors);

	auto blurWeights = CalcGaussWeights(2.5f);
	ssaoCB->blurWeights[0] = XMFLOAT4(&blurWeights[0]);
	ssaoCB->blurWeights[1] = XMFLOAT4(&blurWeights[4]);
	ssaoCB->blurWeights[2] = XMFLOAT4(&blurWeights[8]);

	ssaoCB->invRenderTargetSize = XMFLOAT2(1.0f / SsaoMapWidth(), 1.0f / SsaoMapHeight());

	ssaoCB->occlusionRadius = 0.5f;
	ssaoCB->occlusionFadeStart = 0.2f;
	ssaoCB->occlusionFadeEnd = 1.0f;
	ssaoCB->surfaceEpsilon = 0.05f;
}

std::vector<float> CSsao::CalcGaussWeights(float sigma)
{
	float twoSigma2 = 2.0f * sigma * sigma;

	int blurRadius = (int)ceil(2.0f * sigma);

	assert(blurRadius <= MaxBlurRadius);

	std::vector<float> weights;
	weights.resize(2 * blurRadius + 1);

	float weightSum = 0.0f;

	for (auto i : std::views::iota(-blurRadius, blurRadius))
	{
		float x = (float)i;
		weights[i + blurRadius] = expf(-x * x / twoSigma2);
		weightSum += weights[i + blurRadius];
	}

	for (auto i : std::views::iota(0, (int)weights.size()))
	{
		weights[i] /= weightSum;
	}

	return weights;
}

void CSsao::GetOffsetVectors(DirectX::XMFLOAT4 offsets[14])
{
	std::copy(&m_offsets[0], &m_offsets[14], &offsets[0]);
}

float ARandF()
{
	return (float)(rand()) / (float)RAND_MAX;
}

float ARandF(float a, float b)
{
	return a + ARandF() * (b - a);
}

void CSsao::BuildOffsetVectors()
{
	// 8 cube corners
	m_offsets[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	m_offsets[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	m_offsets[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	m_offsets[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	m_offsets[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	m_offsets[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	m_offsets[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	m_offsets[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 centers of cube faces
	m_offsets[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	m_offsets[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	m_offsets[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	m_offsets[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	m_offsets[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	m_offsets[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for (auto i : std::views::iota(0, 14))
	{
		float s = ARandF(0.25f, 1.0f);
		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&m_offsets[i]));

		XMStoreFloat4(&m_offsets[i], v);
	}
}
