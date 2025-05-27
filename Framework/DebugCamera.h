// ============================================
// 
// ファイル名: DebugCamera.h
// 概要: DebugCamera.cppのヘッダーファイル
// 
// 製作者 : 清水駿希
// 
// ============================================
#pragma once
#include <Mouse.h>
#include <Keyboard.h>

// デバッグ用カメラクラス
class DebugCamera
{
private:
	// デフォルトでカメラが注視点からどのくらい離れているか
	static const float DEFAULT_CAMERA_DISTANCE;
	// 横回転（Ｙ軸回転）
	float m_yAngle, m_yTmp;
	// 縦回転（Ｘ軸回転）
	float m_xAngle, m_xTmp;

	// マウスでドラッグされた座標
	int m_x, m_y;

	// 相対スケール
	float m_sx, m_sy;

	// 生成されたビュー行列
	DirectX::SimpleMath::Matrix m_view;

	// スクロールホイール値
	int m_scrollWheelValue;

	// 視点
	DirectX::SimpleMath::Vector3 m_eye;

	// 注視点
	DirectX::SimpleMath::Vector3 m_target;

	// カメラの頭の方向
	DirectX::SimpleMath::Vector3 m_up;

	//マウス
	DirectX::Mouse::State m_mouse_state;
	// マウスステートトラッカー
	DirectX::Mouse::ButtonStateTracker m_mouseStateTracker;

public:
	DebugCamera();
	~DebugCamera() = default;

	void Initialize(int screenWidth, int screenHeight);
	void Update();

	// ビュー行列（カメラ行列）を取得する
	DirectX::SimpleMath::Matrix GetViewMatrix()
	{
		return m_view;
	}
	// ビュー行列を設定する
	void SetViewMatrix(const DirectX::SimpleMath::Matrix& view) { m_view = view; }

	// 視点を取得する
	DirectX::SimpleMath::Vector3 GetEyePosition()
	{
		return m_eye;
	}

	// 注視点を取得する
	DirectX::SimpleMath::Vector3 GetTargetPosition()
	{
		return m_target;
	}

	// カメラの頭の方向を取得する
	DirectX::SimpleMath::Vector3 GetUpPosition()
	{
		return m_up;
	}


private:
	// 変位を計測する
	void Motion(int x, int y);

	// ビュー行列を計算する
	void CalculateViewMatrix();

	// 相対スケールを計算する
	void CalculateRerativeScale(int windowWidth, int windowHeight);
};

