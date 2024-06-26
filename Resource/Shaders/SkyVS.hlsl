#include "GraphicsHeader.hlsli"
#include "VertexInout_Sky.hlsli"

VertexOut main(VertexIn vin, uint instanceID : SV_InstanceID)
{
    VertexOut vout;
    
    InstanceData instData = gInstanceData[instanceID];
    vout.PosL = vin.PosL;
    
    float4 posW = mul(float4(vin.PosL, 1.0f), instData.World);
    posW.xyz += gEyePosW;
    
    vout.PosH = mul(posW, gViewProj).xyww;
	
	return vout;
}
