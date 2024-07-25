#include "../Register.hlsli"
#include "Type.hlsli"

VertexOut main(VertexIn vin, uint instanceID : SV_InstanceID)
{
    VertexOut vout = (VertexOut) 0.0f;
    
    InstanceData instData = gInstanceData[instanceID];
    float4x4 world = instData.World;
    float4x4 texTransform = instData.TexTransform;
    uint matIndex = instData.MaterialIndex;
    
    vout.MatIndex = matIndex;
    
    MaterialData matData = gMaterialData[matIndex];
    
    vout.NormalW = mul(vin.NormalL, (float3x3) world);
    vout.TangentW = mul(vin.TangentU, (float3x3) world);
    
    float4 posW = mul(float4(vin.PosL, 1.0f), world);
    vout.PosH = mul(posW, gViewProj);
    
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), texTransform);
    vout.TexC = mul(texC, matData.MatTransform).xy;
    
    return vout;
}