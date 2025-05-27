/**********************************************
 *
 *  CommonResources.h
 *  各リソースのラップクラス（シングルトン）
 *
 *  製作者：清水駿希
 * 
 *
 **********************************************/
#pragma once
#include "pch.h"
#include "DeviceResources.h"
#include "CommonStates.h"
#include "StepTimer.h"
#include "InputManager.h"

class CommonResources
{
public:
	// 設定　取得
	// デバイスリソース
	void SetDeviceResources(DX::DeviceResources* deviceResources) { m_deviceResources = deviceResources; }
	DX::DeviceResources* GetDeviceResources() { return m_deviceResources; }
	//	タイマー
	void SetStepTimer(DX::StepTimer* stepTimer) { m_stepTimer = stepTimer; }
	DX::StepTimer* GetStepTimer() { return m_stepTimer; }
	//	コモンステート
	void SetCommonStates(DirectX::CommonStates* commonStates) { m_commonStates = commonStates; }
	DirectX::CommonStates* GetCommonStates() { return m_commonStates; }

	// スクリーンサイズ
	void GetScreenSize(int& width, int& height) const { width = m_screenW; height = m_screenH; }
	void SetScreenSize(const int& width, const int& height) { m_screenW = width; m_screenH = height; }
	// ビュー行列
	void SetViewMatrix(const DirectX::SimpleMath::Matrix& view) { m_view = view; }
	const DirectX::SimpleMath::Matrix& GetViewMatrix() { return m_view; };
	// 射影行列
	void SetProjectionMatrix(const DirectX::SimpleMath::Matrix& projection) { m_projection = projection; }
	const DirectX::SimpleMath::Matrix& GetProjectionMatrix() { return m_projection; };

private:
	//	コンストラクタ
	CommonResources() :
		m_deviceResources(nullptr),
		m_stepTimer(nullptr),
		m_commonStates(nullptr),
		m_screenW(0),
		m_screenH(0),
		m_view{},
		m_projection{}
	{};
	//	デストラクタ
	~CommonResources() = default;

public:
	CommonResources(const CommonResources&) = delete;             // コピー禁止
	CommonResources& operator=(const CommonResources&) = delete;  // コピー代入禁止
	CommonResources(const CommonResources&&) = delete;            // ムーブ禁止
	CommonResources& operator=(const CommonResources&&) = delete; // ムーブ代入禁止

	//	シングルトンインスタンスの取得
	static CommonResources* GetInstance()
	{
		static CommonResources instance;
		return &instance;
	}

private:

	// デバイスリソース
	DX::DeviceResources* m_deviceResources;
	// タイマー
	DX::StepTimer* m_stepTimer;
	// コモンステート
	DirectX::CommonStates* m_commonStates;
	
	// スクリーンサイズ
	int m_screenW, m_screenH;

	// ビュー行列
	DirectX::SimpleMath::Matrix m_view;
	// 射影行列
	DirectX::SimpleMath::Matrix m_projection;
};