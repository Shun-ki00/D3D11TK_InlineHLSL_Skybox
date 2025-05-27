// ============================================
// 
// ファイル名: InputManager.cpp
// 概要: 入力全般の処理を記述（シングルトン）
// 
// 製作者 : 清水駿希
// 
// ============================================
#include "pch.h"
#include "Framework/InputManager.h"

/// <summary>
/// コンストラクタ
/// </summary>
InputManager::InputManager()
	:
	m_mouse{},
	m_mouseStateTracker{},
	m_keyboard{},
	m_keyboardStateTracker{},
	m_gamepad{},
	m_gamePadStateTracker{}
{
	// マウスを使用できる状態にする
	m_mouse = std::make_unique<DirectX::Mouse>();
	m_mouseStateTracker = std::make_unique<DirectX::Mouse::ButtonStateTracker>();

	// キーボードを使用できる状態にする
	m_keyboard = std::make_unique<DirectX::Keyboard>();
	m_keyboardStateTracker = std::make_unique<DirectX::Keyboard::KeyboardStateTracker>();

	// ゲームパッドを使用できる状態にする
	m_gamepad = std::make_unique<DirectX::GamePad>();
	m_gamePadStateTracker = std::make_unique<DirectX::GamePad::ButtonStateTracker>();
}

/// <summary>
/// 更新処理
/// </summary>
void InputManager::Update()
{
	// マウスの情報を更新する
	DirectX::Mouse::State mouseState = m_mouse->GetState();
	m_mouseStateTracker->Update(mouseState);

	// キーボードの情報を更新する
	DirectX::Keyboard::State keyboardState = m_keyboard->GetState();
	m_keyboardStateTracker->Update(keyboardState);

	// ゲームパッドの情報を更新する
	DirectX::GamePad::State gamePadState = m_gamepad->GetState(0);
	m_gamePadStateTracker->Update(gamePadState);
}

/// <summary>
/// キーが現在押されているかを判定する
/// </summary>
/// <param name="key">判定するキー</param>
/// <returns>キーが押されていれば true、それ以外は false</returns>
bool InputManager::OnKey(Keys key)
{
	return m_keyboard->GetState().IsKeyDown(key);
}
/// <summary>
/// キーが押された瞬間を判定する
/// </summary>
/// <param name="key">判定するキー</param>
/// <returns>キーが押された瞬間であれば true、それ以外は false</returns>
bool InputManager::OnKeyDown(Keys key)
{
	return m_keyboardStateTracker->IsKeyPressed(key);
}
/// <summary>
/// 指定されたキーが離された瞬間を判定する
/// </summary>
/// <param name="key">判定するキー</param>
/// <returns>キーが離された瞬間であれば true、それ以外は false</returns>
bool InputManager::OnKeyUp(Keys key)
{
	return m_keyboardStateTracker->IsKeyReleased(key);
}


/// <summary>
/// マウスボタンが現在押されているかを判定する
/// </summary>
/// <param name="mouseButton">判定するマウスボタン</param>
/// <returns>ボタンが押されていれば true、それ以外は false</returns>
bool InputManager::OnMouseButton(MouseButtons mouseButton)
{
	DirectX::Mouse::State state = m_mouse->GetState();

	switch (mouseButton)
	{
	case InputManager::MouseButtons::LEFT:
		return state.leftButton;
	case InputManager::MouseButtons::RIGHT:
		return state.rightButton;
	case InputManager::MouseButtons::MIDDLE:
		return state.middleButton;
	default:
		return false;
	}
}
/// <summary>
/// マウスボタンが押された瞬間を判定する
/// </summary>
/// <param name="mouseButton">判定するマウスボタン</param>
/// <returns>ボタンが押された瞬間であれば true、それ以外は false</returns>
bool InputManager::OnMouseButtonDown(MouseButtons mouseButton)
{
	DirectX::Mouse::ButtonStateTracker* state = m_mouseStateTracker.get();

	switch (mouseButton)
	{
	case InputManager::MouseButtons::LEFT:
		return state->leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED;
	case InputManager::MouseButtons::RIGHT:
		return state->rightButton == DirectX::Mouse::ButtonStateTracker::PRESSED;
	case InputManager::MouseButtons::MIDDLE:
		return state->middleButton == DirectX::Mouse::ButtonStateTracker::PRESSED;
	default:
		return false;
	}
}
/// <summary>
/// マウスボタンが離された瞬間を判定する
/// </summary>
/// <param name="mouseButton">判定するマウスボタン</param>
/// <returns>ボタンが離された瞬間であれば true、それ以外は false</returns>
bool InputManager::OnMouseButtonUp(MouseButtons mouseButton)
{
	DirectX::Mouse::ButtonStateTracker* state = m_mouseStateTracker.get();

	switch (mouseButton)
	{
	case InputManager::MouseButtons::LEFT:
		return state->leftButton == DirectX::Mouse::ButtonStateTracker::RELEASED;
	case InputManager::MouseButtons::RIGHT:
		return state->rightButton == DirectX::Mouse::ButtonStateTracker::RELEASED;
	case InputManager::MouseButtons::MIDDLE:
		return state->middleButton == DirectX::Mouse::ButtonStateTracker::RELEASED;
	default:
		return false;
	}
}


