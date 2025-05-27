#include "PBRLit.hlsli"

// ノーマルマップ
Texture2D<float4> normalMap : register(t1);
// キューブマップ
TextureCube<float4> cubeMap : register(t2);
// シャドウマップテクスチャ
Texture2D ShadowMapTexture : register(t3);


float4 main(PS_Input input) : SV_TARGET
{
    // ベースカラーを使用する場合サンプリングを行う
    float4 baseColor = lerp(c_baseColor,Texture.Sample(Sampler,input.uv),t_useBaseMap);
    
     // ノーマルマップをサンプリング
    float3 normalMapSample = normalMap.Sample(Sampler, input.uv).rgb;
    
    normalMapSample = normalMapSample * 2.0 - 1.0;
    // TBN行列の作成
    float3x3 TBN = float3x3(input.tangentWS, input.binormalWS, input.normalWS);
    
    // ノーマルマップを使用する場合接線空間の法線をワールド空間に変換
    float3 normalWS = lerp(input.normalWS, normalize(mul(normalMapSample, TBN)), t_useNormalMap);
   
    // 視線ベクトルを計算
    float3 viewDir = normalize(EyePosition - input.positionWS);
    
    // 鏡面反射光の色をサンプリング
    float3 refVec = reflect(viewDir, normalWS);
    refVec.y *= -1;
    float3 indirectSpecular = cubeMap.SampleLevel(Sampler, refVec, f_smoothness * 12).rgb;
    
    // ディレクショナルライトの情報（DirectXTK の標準ライト）
    float3 lightDir = LightDirection[0];      // ライトの方向
    float3 lightColor = LightDiffuseColor[0]; // ライトの色

     // PBRのBRDFを計算
    float4 color = BRDF(
        baseColor.rgb,   // ベースカラー
        f_matallic,      // メタリック
        f_smoothness,    // 粗さ
        normalWS,        // 法線ベクトル
        viewDir,         // 視線ベクトル
        lightDir,        // ライトの方向
        lightColor,      // ライトの色
        indirectSpecular // 環境光による間接鏡面反射
    );
    
    // アルファ値を設定
    color.a = baseColor.a;

    // 最終的なピクセルカラーを返す
    return color;
}