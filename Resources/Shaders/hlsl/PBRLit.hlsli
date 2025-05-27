#include "Common.hlsli"


// PBR定数バッファ
cbuffer PBRLitConstantBuffer : register(b2)
{
    float4 c_baseColor : packoffset(c0);  // 基本色
    float f_matallic : packoffset(c1.x);    // 金属度
    float f_smoothness : packoffset(c1.y);  // 表面の滑らかさ
    float t_useBaseMap : packoffset(c1.z);   // ベースカラーテクスチャを使用するか
    float t_useNormalMap : packoffset(c1.w); // 法線マップを使用するか
}

// 頂点シェーダ入力用
struct VS_Input
{
    float4 positionOS : SV_Position;
    float3 normalOS : NORMAL;
    float4 tangentOS : TANGENT;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

// ピクセルシェーダ入力用
struct PS_Input
{
    float4 positionCS : SV_Position;
    float3 normalWS : NORMAL;
    float3 tangentWS : TANGENT;
    float3 binormalWS : TEXCOORD1;
    float4  color : COLOR;
    float2 uv : TEXCOORD;
    
    float3 positionWS :TEXCOORD2;
};

// PI
static const float F_PI = 3.1415926f;


// 誘電体の反射率（F0）は4%とする
static const float DIELECTRIC = 0.04f;

// Cook-TorranceのD項をGGXで求める
float D_GGX(float ndoth, float alpha)
{
    float a2 = alpha * alpha;
    float d = (ndoth * a2 - ndoth) * ndoth + 1.0f;
    return a2 / (d * d + 0.0000001) * (1.0 / F_PI);
}
float D_GGX(float perceptualRoughness, float ndoth, float3 normalWS, float3 halfDir)
{
    float3 NcrossH = cross(normalWS, halfDir);
    float a = ndoth * perceptualRoughness;
    float k = perceptualRoughness / (dot(NcrossH, NcrossH) + a * a);
    float d = k * k * (1.0f / F_PI);
    return min(d, 65504.0);
}

// Cook-TorranceのV項をHeight-Correlated Smithモデルで求める
float V_SmithGGXCorrelated(float ndotl, float ndotv, float alpha)
{
    float lambdaV = ndotl * (ndotv * (1 - alpha) + alpha);
    float lambdaL = ndotv * (ndotl * (1 - alpha) + alpha);
    return 0.5f / (lambdaV + lambdaL + 0.0001);
}

// Cook-TorranceのF項をSchlickの近似式で求める
float3 F_Schlick(float3 f0, float cos)
{
    return f0 + (1 - f0) * pow(1 - cos, 5);
}


// Disneyのモデルで拡散反射を求める
float Fd_Burley(float ndotv, float ndotl, float ldoth, float roughness)
{
    float fd90 = 0.5 + 2 * ldoth * ldoth * roughness;
    float lightScatter = (1 + (fd90 - 1) * pow(1 - ndotl, 5));
    float viewScatter = (1 + (fd90 - 1) * pow(1 - ndotv, 5));

    float diffuse = lightScatter * viewScatter;
	// diffuse /= F_PI;
    return diffuse;
}

// BRDF式
float4 BRDF(
	float3 albedo,
	float metallic,
	float perceptualRoughness,
	float3 normalWS,
	float3 viewDir,
	float3 lightDir,
	float3 lightColor,
	float3 indirectSpecular
)
{
    float3 halfDir = normalize(lightDir + viewDir);
    float NdotV = abs(dot(normalWS, viewDir));
    float NdotL = max(0, dot(normalWS, lightDir));
    float NdotH = max(0, dot(normalWS, halfDir));
    float LdotH = max(0, dot(lightDir, halfDir));
    float reflectivity = lerp(DIELECTRIC, 1, metallic);
    float3 f0 = lerp(DIELECTRIC, albedo, metallic);

	// 拡散反射
    float diffuseTerm = Fd_Burley(
		NdotV,
		NdotL,
		LdotH,
		perceptualRoughness
	) * NdotL;
    float3 diffuse = albedo * (1 - reflectivity) * lightColor * diffuseTerm;
	// 関節拡散反射
    diffuse += albedo * (1 - reflectivity) * c_ambientLightColor.rgb * f_ambientLightIntensity;
	
	// 鏡面反射
    float alpha = perceptualRoughness * perceptualRoughness;
    float V = V_SmithGGXCorrelated(NdotL, NdotV, alpha);
    float D = D_GGX(NdotH, alpha);
    float3 F = F_Schlick(f0, LdotH);
    float3 specular = V * D * F * NdotL * lightColor;
    specular *= F_PI;
    specular = max(0, specular);
	// 環境反射光
    float surfaceReduction = 1.0 / (alpha * alpha + 1);
    float f90 = saturate((1 - perceptualRoughness) + reflectivity);
    specular += surfaceReduction * indirectSpecular * lerp(f0, f90, pow(1 - NdotV, 5));
	
	
    float3 color = diffuse + specular;
    return float4(color, 1);
}
