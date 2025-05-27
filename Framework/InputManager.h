// ============================================
// 
// ファイル名: InputManager.h
// 概要: InputManager.cppのヘッダーファイル
// 
// 製作者 : 清水駿希
// 
// ============================================
#pragma once
#include <Mouse.h>
#include <Keyboard.h>
#include <GamePad.h>

enum class KeyType : int
{
	ON_KEY,
	ON_KEY_DOWN,
	ON_KEY_UP,
};

class InputManager
{
public:
	using Keys = DirectX::Keyboard::Keys;

	enum class MouseButtons
	{
		LEFT,		//	左
		RIGHT,		//	右
		MIDDLE,		//	ホイール
	};
	
public:
	// マウスの状態を変更
	void SetMouseMode(DirectX::Mouse::Mode mouse_mode) { m_mouse->SetMode(mouse_mode); }
	// マウスを取得する
	DirectX::Mouse* GetMouse() const { return m_mouse.get(); }
	// マウスのボタンステートトラッカーを取得する
	DirectX::Mouse::ButtonStateTracker* GetMouseTracker() const { return m_mouseStateTracker.get(); }

	// キーボードを取得する
	DirectX::Keyboard* GetKeyboard() const {return m_keyboard.get(); }
	// キーボードステートトラッカーを取得する
	DirectX::Keyboard::KeyboardStateTracker* GetKeyboardStateTracker() const { return m_keyboardStateTracker.get(); }

	// ゲームパッド取得する
	DirectX::GamePad* GetGamePad() const { return m_gamepad.get(); }
	// ゲームパットステートトラッカーを取得する
	DirectX::GamePad::ButtonStateTracker* GetButtonStateTracker() const { return m_gamePadStateTracker.get(); }

private:
	//	コンストラクタ
	InputManager();
	//	デストラクタ
	~InputManager() = default;
public:
	InputManager(const InputManager&) = delete;             // コピー禁止
	InputManager& operator=(const InputManager&) = delete;  // コピー代入禁止
	InputManager(const InputManager&&) = delete;            // ムーブ禁止
	InputManager& operator=(const InputManager&&) = delete; // ムーブ代入禁止

	//	シングルトンインスタンスの取得
	static InputManager* GetInstance()
	{
		static InputManager instance;
		return &instance;
	}
public:
	// 更新処理
	void Update();

	// キー判定
	bool OnKey(Keys key);     // キーが現在押されているかを判定する
	bool OnKeyDown(Keys key); // キーが押された瞬間を判定する
	bool OnKeyUp(Keys key);   // キーが離された瞬間を判定する
	// マウス判定
	bool OnMouseButton(MouseButtons mouseButton);     // マウスボタンが現在押されているかを判定する
	bool OnMouseButtonDown(MouseButtons mouseButton); // マウスボタンが押された瞬間を判定する
	bool OnMouseButtonUp(MouseButtons mouseButton);   // マウスボタンが離された瞬間を判定する


private:
	// マウス
	std::unique_ptr<DirectX::Mouse> m_mouse;
	std::unique_ptr<DirectX::Mouse::ButtonStateTracker> m_mouseStateTracker;
	// キーボード
	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::Keyboard::KeyboardStateTracker> m_keyboardStateTracker;
	// パッド
	std::unique_ptr<DirectX::GamePad> m_gamepad;
	std::unique_ptr<DirectX::GamePad::ButtonStateTracker> m_gamePadStateTracker;
};