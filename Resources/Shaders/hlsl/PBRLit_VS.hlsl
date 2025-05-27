#include "PBRLit.hlsli"

PS_Input main(VS_Input input)
{
    // 出力構造体0初期化
    PS_Input output = (PS_Input)0;
    
    // ワールド空間へ変換
    output.positionWS = mul(float4(input.positionOS.xyz, 1.0), World).xyz;
    // 投影空間へ変換
    output.positionCS = mul(float4(input.positionOS.xyz, 1.0), WorldViewProj);
    
    
    // 法線ベクトルをワールド空間へ（拡大縮小の影響を打ち消すため逆転置行列をかける）
    output.normalWS = normalize(mul(input.normalOS, WorldInverseTranspose));
    // 接線ベクトルをワールド空間へ
    output.tangentWS = normalize(mul(float4(input.tangentOS.xyz, 0), World).xyz);
    // バイノーマル計算
    output.binormalWS = normalize(cross(output.normalWS, output.tangentWS));

        
    // テクスチャのUV座標
    output.uv = input.uv;
    //output.uv.y *= -1.0f;
    
    // 頂点カラー
    output.color = input.color;
        
    return output;
}