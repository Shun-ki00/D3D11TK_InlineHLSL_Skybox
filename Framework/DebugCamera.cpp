// ============================================
// 
// ファイル名: DebugCamera.cpp
// 概要: デバッグカメラ
// 
// 製作者 : 清水駿希
// 
// ============================================
#include "pch.h"
#include "Framework/DebugCamera.h"


using namespace DirectX;
using namespace DirectX::SimpleMath;

const float DebugCamera::DEFAULT_CAMERA_DISTANCE = 2.0f;

/// <summary>
/// コンストラクタ
/// </summary>
DebugCamera::DebugCamera()
	:
	m_yAngle{ 0.0f },
	m_yTmp{ 0.0f },
	m_xAngle{ 0.0f },
	m_xTmp{ 0.0f },
	m_x{ 0 },
	m_y{ 0 },
	m_sx{ 0.0f },
	m_sy{ 0.0f },
	m_view{},
	m_scrollWheelValue{ 0 },
	m_eye{},
	m_target{}
{

}

/// <summary>
/// 初期化処理
/// </summary>
/// <param name="screenWidth">スクリーンサイズ　横</param>
/// <param name="screenHeight">スクリーンサイズ　縦</param>
void DebugCamera::Initialize(int screenWidth,int screenHeight)
{

	CalculateRerativeScale(screenWidth, screenHeight);
	CalculateViewMatrix();

	// マウスのホイール値をリセット
	Mouse::Get().ResetScrollWheelValue();
	//!! DirectX::Mouseはシングルトンなので…
	
	
}

/// <summary>
/// 更新処理
/// </summary>
void DebugCamera::Update()
{
	

	// マウスの状態を取得する
	m_mouse_state = DirectX::Mouse::Get().GetState();
	// マウスステートトラッカーを更新する
	m_mouseStateTracker.Update(m_mouse_state);

	// 相対モードなら何もしない
	if (m_mouse_state.positionMode == Mouse::MODE_RELATIVE)
	{
		return;
	}

	// マウスの左ボタンが押された
	if (m_mouseStateTracker.leftButton == Mouse::ButtonStateTracker::ButtonState::PRESSED)
	{
		// マウスの座標を取得
		m_x = m_mouse_state.x;
		m_y = m_mouse_state.y;
	}
	// マウスの左ボタンが解放された
	else if (m_mouseStateTracker.leftButton == Mouse::ButtonStateTracker::ButtonState::RELEASED)
	{
		// 現在の回転を保存
		m_xAngle = m_xTmp;
		m_yAngle = m_yTmp;
	}

	// マウスのボタンが押されていたらカメラを移動させる
	if (m_mouse_state.leftButton)
	{
		Motion(m_mouse_state.x, m_mouse_state.y);
	}

	// マウスのホイール値を取得
	m_scrollWheelValue = m_mouse_state.scrollWheelValue;
	if (m_scrollWheelValue > 0)
	{
		m_scrollWheelValue = 0;
		Mouse::Get().ResetScrollWheelValue();
		//!! DirectX::Mouseはシングルトンなので…
	}

	// ビュー行列を算出する
	CalculateViewMatrix();
	// ビュー行列の設定
	//m_graphics->SetViewMatrix(m_view);
}

/// <summary>
/// 変位を計測する
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
void DebugCamera::Motion(int x, int y)
{
	// マウスポインタの位置のドラッグ開始位置からの変位 (相対値)
	float dx = (x - m_x) * m_sx;
	float dy = (y - m_y) * m_sy;

	if (dx != 0.0f || dy != 0.0f)
	{
		// Ｙ軸の回転
		float yAngle = dx * XM_PI;
		// Ｘ軸の回転
		float xAngle = dy * XM_PI;

		m_xTmp = m_xAngle + xAngle;
		m_yTmp = m_yAngle + yAngle;
	}
}

/// <summary>
/// ビュー行列を計算する
/// </summary>
void DebugCamera::CalculateViewMatrix()
{
	// ビュー行列を算出する
	Matrix rotY = Matrix::CreateRotationY(m_yTmp);
	Matrix rotX = Matrix::CreateRotationX(m_xTmp);

	Matrix rotYX = rotY * rotX;

	Vector3 eye(0.0f, 1.0f, 1.0f);
	Vector3 target(0.0f, 0.0f, 0.0f);
	Vector3 up(0.0f, 1.0f, 0.0f);

	eye = Vector3::Transform(eye, rotYX.Invert());
	eye *= (DEFAULT_CAMERA_DISTANCE - m_scrollWheelValue / 100);
	up = Vector3::Transform(up, rotYX.Invert());

	m_eye = eye;
	m_target = target;
	m_up = up;

	m_view = Matrix::CreateLookAt(eye, target, up);
}

/// <summary>
/// 相対スケールを計算する
/// </summary>
/// <param name="screenWidth">スクリーンサイズ　横</param>
/// <param name="screenHeight">スクリーンサイズ　縦</param>
void DebugCamera::CalculateRerativeScale(int screenWidth, int screenHeight)
{
	// 画面サイズに対する相対的なスケールに調整
	m_sx = 1.0f / static_cast<float>(screenWidth);
	m_sy = 1.0f / static_cast<float>(screenHeight);
}
