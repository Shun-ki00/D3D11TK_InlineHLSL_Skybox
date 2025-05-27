#include "pch.h"
#include "Game/Scene.h"
#include "Framework/CommonResources.h"
#include "Framework/DebugCamera.h"
#include "Framework/ConstantBuffer.h"
#include "Framework/Microsoft/ReadData.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include "imgui/ImGuizmo.h"
#include <Model.h>
#include <d3dcompiler.h>

/// <summary>
/// コンストラクタ
/// </summary>
Scene::Scene()
	:
	m_commonResources{},
	m_camera{},
	m_device{},
	m_context{},
	m_skyboxModel{},
	m_vertexShader{},
	m_pixelShader{},
	m_cubeMap{},
	m_constantBuffer{},
	m_angle{},
	m_dayProgress{}
{
	// インスタンスを取得する
	m_commonResources = CommonResources::GetInstance();
	m_device          = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDevice();
	m_context         = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
}

/// <summary>
/// 初期化処理
/// </summary>
void Scene::Initialize()
{
	// カメラの作成
	m_camera = std::make_unique<DebugCamera>();
	m_camera->Initialize(1280, 720);
	

	// 頂点シェーダーの作成
	this->CreateVertexShader();
	// ピクセルシェーダーの作成
	this->CreatePixelShader();


	// スカイボックス用モデルの作成
	m_skyboxModel = DirectX::GeometricPrimitive::CreateSphere(m_context, 2.0f, 6, false);
	
	// キューブマップロード
	DirectX::CreateDDSTextureFromFile(
		m_device, L"Resources/Textures/CubeMap.dds", nullptr, m_cubeMap.ReleaseAndGetAddressOf());

	// 定数バッファ用のバッファオブジェクトを作成する
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(SkyBoxConstBuffer));	// 16の倍数を指定する
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	DX::ThrowIfFailed(
		m_device->CreateBuffer(&bufferDesc, nullptr, m_constantBuffer.ReleaseAndGetAddressOf())
	);

	// 回転角を初期化
	m_angle = 0.0f;
	// 一日の進行度を初期化
	m_dayProgress = 0.0f;

}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="elapsedTime">経過時間</param>
void Scene::Update(const float& elapsedTime)
{

	m_camera->Update();
	m_commonResources->SetViewMatrix(m_camera->GetViewMatrix());

	m_angle += elapsedTime * 30.0f;

	// ワールド行列の作成
	DirectX::SimpleMath::Matrix world =
		DirectX::SimpleMath::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_angle * 0.01f));

	// GPUが使用するリソースのメモリにCPUからアクセスする際に利用する構造体
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	// コンテキスト
	auto context = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();

	// 定数バッファをマップする
	context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// 定数バッファに送るデータを準備する
	SkyBoxConstBuffer* cb = static_cast<SkyBoxConstBuffer*>(mappedResource.pData);
	cb->worldMatrix = world.Transpose();
	cb->viewMatrix = m_camera->GetViewMatrix().Transpose();
	cb->projectionMatrix = m_commonResources->GetProjectionMatrix().Transpose();
	cb->dayProgress = DirectX::SimpleMath::Vector4(m_dayProgress, 0.0f, 0.0f, 0.0f);
	// マップを解除する
	context->Unmap(m_constantBuffer.Get(), 0);

}

/// <summary>
/// 描画処理
/// </summary>
void Scene::Render()
{
	// スカイボックスの描画
	m_skyboxModel->Draw({}, {}, {}, {}, nullptr, false, [&]()
		{
			// シェーダーの設定
			m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
			m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

			// 定数バッファの設定（共通定数バッファのみ）
			m_context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
			m_context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

			ID3D11ShaderResourceView* view = m_cubeMap.Get();
			// テクスチャを設定
			m_context->PSSetShaderResources(0, 1, &view);
			m_context->PSSetShaderResources(1, 1, &view);

			// サンプラーの設定
			auto sampler = m_commonResources->GetCommonStates()->LinearClamp();
			m_context->VSSetSamplers(0, 1, &sampler);
			m_context->PSSetSamplers(0, 1, &sampler);
		});

	// シェーダの解放
	m_context->VSSetShader(nullptr, nullptr, 0);
	m_context->PSSetShader(nullptr, nullptr, 0);
	// リソースの解放
	ID3D11ShaderResourceView* resource = nullptr;
	m_context->PSSetShaderResources(0, 1, &resource);
	m_context->PSSetShaderResources(1, 1, &resource);

}

/// <summary>
/// 終了処理
/// </summary>
void Scene::Finalize() {}



void Scene::CreateVertexShader()
{
	static const char* SkyboxShaderHLSL = R"(
			cbuffer SkyBoxConstBuffer : register(b0)
			{
				matrix matWorld;
				matrix matView;
				matrix matProj;
				float4 dayProgress;
			};

			// 頂点シェーダ入力用
			struct VS_Input
			{
				float3 positionOS : SV_Position;
			};

			// ピクセルシェーダ入力用
			struct PS_Input
			{
				float4 positionCS : SV_POSITION;
				float3 texcoord : TEXCOORD;
			};

		PS_Input main(VS_Input input)
		{
			// 出力構造体を初期化
			PS_Input output = (PS_Input) 0;
    
			// オブジェクト空間 (OS) からワールド空間 (WS) へ変換
			float4 position = mul(float4(input.positionOS, 1.0f), matWorld);
    
				// ビュー行列の平行移動成分をゼロにする
			matrix SetMatView = matView;
			SetMatView._41 = 0;
			SetMatView._42 = 0;
			SetMatView._43 = 0;
    
			// ワールド空間からビュー空間へ変換
			position = mul(position, SetMatView);
			// ビュー空間からクリップ空間 (CS) へ変換
			position = mul(position, matProj);
    
			// 深度値を最大化して常に奥に描画
			position.z = position.w;
    
			// 出力変数に座標とテクスチャ座標を格納
			output.positionCS = position;
			output.texcoord = input.positionOS.xyz;
    
			// ピクセルシェーダーに渡す
			return output;
		}

		)";

	// コンパイルする
	ID3DBlob* vertexShaderBlob;
	D3DCompile(
		SkyboxShaderHLSL,
		strlen(SkyboxShaderHLSL),
		nullptr, nullptr, nullptr,
		"main", "vs_5_0", 0, 0, &vertexShaderBlob, nullptr);


	// 頂点シェーダをロードする
	DX::ThrowIfFailed(
		m_device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(),
			nullptr, m_vertexShader.ReleaseAndGetAddressOf())
	);

	vertexShaderBlob->Release();
}


void Scene::CreatePixelShader()
{
	static const char* SkyboxShaderHLSL = R"(
			cbuffer SkyBoxConstBuffer : register(b0)
			{
				matrix matWorld;
				matrix matView;
				matrix matProj;
				float4 dayProgress;
			};

			// 頂点シェーダ入力用
			struct VS_Input
			{
				float3 positionOS : SV_Position;
			};

			// ピクセルシェーダ入力用
			struct PS_Input
			{
				float4 positionCS : SV_POSITION;
				float3 texcoord : TEXCOORD;
			};

			TextureCube cubeMap : register(t0);
			TextureCube eveningCubeMap : register(t1);

			SamplerState Sample : register(s0);

			float4 main(PS_Input input) : SV_TARGET
			{
				float4 color = cubeMap.Sample(Sample, normalize(input.texcoord));
				float4 color2 = eveningCubeMap.Sample(Sample, normalize(input.texcoord));
    
				float4 finalColor = lerp(color, color2, dayProgress.x);
    
				return finalColor;
			}
		)";

	// コンパイルする
	ID3DBlob* PixelShaderBlob;
	D3DCompile(
		SkyboxShaderHLSL,
		strlen(SkyboxShaderHLSL),
		nullptr, nullptr, nullptr,
		"main", "ps_5_0", 0, 0, &PixelShaderBlob, nullptr);


	// シェピクセルーダをロードする
	DX::ThrowIfFailed(
		m_device->CreatePixelShader(
			PixelShaderBlob->GetBufferPointer(), PixelShaderBlob->GetBufferSize(),
			nullptr, m_pixelShader.ReleaseAndGetAddressOf())
	);

	PixelShaderBlob->Release();

}




