struct VertexIn
{
    float3 PosL : POSITION;
    float2 TexC : TEXCOORD;
    float3 BoneWeights : WEIGHTS;
    uint4 BoneIndices  : BONEINDICES;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD;
    
    nointerpolation uint MatIndex : MATINDEX;
};
