#include "../Register.hlsli"
#include "Type.hlsli"

void main(VertexOut pin)
{
    MaterialData matData = gMaterialData[pin.MatIndex];
    
    float4 diffuseAlbedo = matData.DiffuseAlbedo;
    uint diffuseMapIndex = matData.DiffuseMapIndex;
    
    diffuseAlbedo *= gDiffuseMap[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    
#ifdef ALPHA_TEST
    clip(diffuseAlbedo.a - 0.1f);
#endif

}