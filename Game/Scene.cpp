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
/// �R���X�g���N�^
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
	// �C���X�^���X���擾����
	m_commonResources = CommonResources::GetInstance();
	m_device          = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDevice();
	m_context         = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
}

/// <summary>
/// ����������
/// </summary>
void Scene::Initialize()
{
	// �J�����̍쐬
	m_camera = std::make_unique<DebugCamera>();
	m_camera->Initialize(1280, 720);
	

	// ���_�V�F�[�_�[�̍쐬
	this->CreateVertexShader();
	// �s�N�Z���V�F�[�_�[�̍쐬
	this->CreatePixelShader();


	// �X�J�C�{�b�N�X�p���f���̍쐬
	m_skyboxModel = DirectX::GeometricPrimitive::CreateSphere(m_context, 2.0f, 6, false);
	
	// �L���[�u�}�b�v���[�h
	DirectX::CreateDDSTextureFromFile(
		m_device, L"Resources/Textures/CubeMap.dds", nullptr, m_cubeMap.ReleaseAndGetAddressOf());

	// �萔�o�b�t�@�p�̃o�b�t�@�I�u�W�F�N�g���쐬����
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(SkyBoxConstBuffer));	// 16�̔{�����w�肷��
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	DX::ThrowIfFailed(
		m_device->CreateBuffer(&bufferDesc, nullptr, m_constantBuffer.ReleaseAndGetAddressOf())
	);

	// ��]�p��������
	m_angle = 0.0f;
	// ����̐i�s�x��������
	m_dayProgress = 0.0f;

}

/// <summary>
/// �X�V����
/// </summary>
/// <param name="elapsedTime">�o�ߎ���</param>
void Scene::Update(const float& elapsedTime)
{

	m_camera->Update();
	m_commonResources->SetViewMatrix(m_camera->GetViewMatrix());

	m_angle += elapsedTime * 30.0f;

	// ���[���h�s��̍쐬
	DirectX::SimpleMath::Matrix world =
		DirectX::SimpleMath::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_angle * 0.01f));

	// GPU���g�p���郊�\�[�X�̃�������CPU����A�N�Z�X����ۂɗ��p����\����
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	// �R���e�L�X�g
	auto context = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();

	// �萔�o�b�t�@���}�b�v����
	context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// �萔�o�b�t�@�ɑ���f�[�^����������
	SkyBoxConstBuffer* cb = static_cast<SkyBoxConstBuffer*>(mappedResource.pData);
	cb->worldMatrix = world.Transpose();
	cb->viewMatrix = m_camera->GetViewMatrix().Transpose();
	cb->projectionMatrix = m_commonResources->GetProjectionMatrix().Transpose();
	cb->dayProgress = DirectX::SimpleMath::Vector4(m_dayProgress, 0.0f, 0.0f, 0.0f);
	// �}�b�v����������
	context->Unmap(m_constantBuffer.Get(), 0);

}

/// <summary>
/// �`�揈��
/// </summary>
void Scene::Render()
{
	// �X�J�C�{�b�N�X�̕`��
	m_skyboxModel->Draw({}, {}, {}, {}, nullptr, false, [&]()
		{
			// �V�F�[�_�[�̐ݒ�
			m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
			m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

			// �萔�o�b�t�@�̐ݒ�i���ʒ萔�o�b�t�@�̂݁j
			m_context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
			m_context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

			ID3D11ShaderResourceView* view = m_cubeMap.Get();
			// �e�N�X�`����ݒ�
			m_context->PSSetShaderResources(0, 1, &view);
			m_context->PSSetShaderResources(1, 1, &view);

			// �T���v���[�̐ݒ�
			auto sampler = m_commonResources->GetCommonStates()->LinearClamp();
			m_context->VSSetSamplers(0, 1, &sampler);
			m_context->PSSetSamplers(0, 1, &sampler);
		});

	// �V�F�[�_�̉��
	m_context->VSSetShader(nullptr, nullptr, 0);
	m_context->PSSetShader(nullptr, nullptr, 0);
	// ���\�[�X�̉��
	ID3D11ShaderResourceView* resource = nullptr;
	m_context->PSSetShaderResources(0, 1, &resource);
	m_context->PSSetShaderResources(1, 1, &resource);

}

/// <summary>
/// �I������
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

			// ���_�V�F�[�_���͗p
			struct VS_Input
			{
				float3 positionOS : SV_Position;
			};

			// �s�N�Z���V�F�[�_���͗p
			struct PS_Input
			{
				float4 positionCS : SV_POSITION;
				float3 texcoord : TEXCOORD;
			};

		PS_Input main(VS_Input input)
		{
			// �o�͍\���̂�������
			PS_Input output = (PS_Input) 0;
    
			// �I�u�W�F�N�g��� (OS) ���烏�[���h��� (WS) �֕ϊ�
			float4 position = mul(float4(input.positionOS, 1.0f), matWorld);
    
				// �r���[�s��̕��s�ړ��������[���ɂ���
			matrix SetMatView = matView;
			SetMatView._41 = 0;
			SetMatView._42 = 0;
			SetMatView._43 = 0;
    
			// ���[���h��Ԃ���r���[��Ԃ֕ϊ�
			position = mul(position, SetMatView);
			// �r���[��Ԃ���N���b�v��� (CS) �֕ϊ�
			position = mul(position, matProj);
    
			// �[�x�l���ő剻���ď�ɉ��ɕ`��
			position.z = position.w;
    
			// �o�͕ϐ��ɍ��W�ƃe�N�X�`�����W���i�[
			output.positionCS = position;
			output.texcoord = input.positionOS.xyz;
    
			// �s�N�Z���V�F�[�_�[�ɓn��
			return output;
		}

		)";

	// �R���p�C������
	ID3DBlob* vertexShaderBlob;
	D3DCompile(
		SkyboxShaderHLSL,
		strlen(SkyboxShaderHLSL),
		nullptr, nullptr, nullptr,
		"main", "vs_5_0", 0, 0, &vertexShaderBlob, nullptr);


	// ���_�V�F�[�_�����[�h����
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

			// ���_�V�F�[�_���͗p
			struct VS_Input
			{
				float3 positionOS : SV_Position;
			};

			// �s�N�Z���V�F�[�_���͗p
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

	// �R���p�C������
	ID3DBlob* PixelShaderBlob;
	D3DCompile(
		SkyboxShaderHLSL,
		strlen(SkyboxShaderHLSL),
		nullptr, nullptr, nullptr,
		"main", "ps_5_0", 0, 0, &PixelShaderBlob, nullptr);


	// �V�F�s�N�Z���[�_�����[�h����
	DX::ThrowIfFailed(
		m_device->CreatePixelShader(
			PixelShaderBlob->GetBufferPointer(), PixelShaderBlob->GetBufferSize(),
			nullptr, m_pixelShader.ReleaseAndGetAddressOf())
	);

	PixelShaderBlob->Release();

}




