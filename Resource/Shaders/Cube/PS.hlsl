#include "../Register.hlsli"
#include "Type.hlsli"

float4 main(VertexOut pin) : SV_Target
{
    return gCubeMap.Sample(gsamLinearWrap, pin.PosL);
}