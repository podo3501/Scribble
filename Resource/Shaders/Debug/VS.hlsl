#include "../Register.hlsli"
#include "Type.hlsli"

VertexOut main(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.0f;

    // Already in homogeneous clip space.
    vout.PosH = float4(vin.PosL, 1.0f);
	
    vout.TexC = vin.TexC;
	
    return vout;
}