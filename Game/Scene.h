#pragma once
#include <future>
#include "Framework/DebugCamera.h"
#include "Framework/ConstantBuffer.h"
#include <thread>

class CommonResources;
class DebugCamera;

class Scene
{
private:

	// スカイボックスの定数バッファ
	struct SkyBoxConstBuffer
	{
		DirectX::SimpleMath::Matrix worldMatrix;
		DirectX::SimpleMath::Matrix viewMatrix;
		DirectX::SimpleMath::Matrix projectionMatrix;
		DirectX::SimpleMath::Vector4 dayProgress;
	};

public:

	// コンストラクタ
	Scene();
	// デストラクタ
	~Scene() = default;

public:

	// 初期化処理
	void Initialize();
	// 更新処理
	void Update(const float& elapsedTime);
	// 描画処理
	void Render();
	// 終了処理
	void Finalize();

private:

	// 頂点シェーダーの作成
	void CreateVertexShader();
	// ピクセルシェーダーの作成
	void CreatePixelShader();

private:


	// 共有リソース
	CommonResources* m_commonResources;

	// デバッグカメラ
	std::unique_ptr<DebugCamera> m_camera;

	// デバイス
	ID3D11Device1* m_device;
	// コンテキスト
	ID3D11DeviceContext1* m_context;


	// スカイボックスモデル
	std::unique_ptr<DirectX::GeometricPrimitive> m_skyboxModel;
	
	// 頂点シェーダー
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	// ピクセルシェーダー
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

	// キューブマップ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubeMap;
	// 定数バッファ用のバッファオブジェクト
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

	// 回転角
	float m_angle;
	// 一日の進行度
	float m_dayProgress;
};