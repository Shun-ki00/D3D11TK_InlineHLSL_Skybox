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

	// �X�J�C�{�b�N�X�̒萔�o�b�t�@
	struct SkyBoxConstBuffer
	{
		DirectX::SimpleMath::Matrix worldMatrix;
		DirectX::SimpleMath::Matrix viewMatrix;
		DirectX::SimpleMath::Matrix projectionMatrix;
		DirectX::SimpleMath::Vector4 dayProgress;
	};

public:

	// �R���X�g���N�^
	Scene();
	// �f�X�g���N�^
	~Scene() = default;

public:

	// ����������
	void Initialize();
	// �X�V����
	void Update(const float& elapsedTime);
	// �`�揈��
	void Render();
	// �I������
	void Finalize();

private:

	// ���_�V�F�[�_�[�̍쐬
	void CreateVertexShader();
	// �s�N�Z���V�F�[�_�[�̍쐬
	void CreatePixelShader();

private:


	// ���L���\�[�X
	CommonResources* m_commonResources;

	// �f�o�b�O�J����
	std::unique_ptr<DebugCamera> m_camera;

	// �f�o�C�X
	ID3D11Device1* m_device;
	// �R���e�L�X�g
	ID3D11DeviceContext1* m_context;


	// �X�J�C�{�b�N�X���f��
	std::unique_ptr<DirectX::GeometricPrimitive> m_skyboxModel;
	
	// ���_�V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	// �s�N�Z���V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

	// �L���[�u�}�b�v
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubeMap;
	// �萔�o�b�t�@�p�̃o�b�t�@�I�u�W�F�N�g
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

	// ��]�p
	float m_angle;
	// ����̐i�s�x
	float m_dayProgress;
};