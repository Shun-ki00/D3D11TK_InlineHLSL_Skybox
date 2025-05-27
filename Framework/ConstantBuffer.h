#pragma once
#include <wrl/client.h>
#include <cstring>
#include <type_traits>



template<typename T>
class ConstantBuffer {
    static_assert(std::is_trivially_copyable_v<T>, "ConstantBuffer<T>: T must be trivially copyable");

public:
    ConstantBuffer() = default;

    // 初期化
    bool Initialize(ID3D11Device* device) {
        // 定数バッファ用のバッファオブジェクトを作成する
        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = static_cast<UINT>(sizeof(T));	
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      
        return SUCCEEDED(device->CreateBuffer(&bufferDesc, nullptr, buffer.ReleaseAndGetAddressOf()));
    }

    // 値が変更された場合のみ更新
    void UpdateIfNeeded(ID3D11DeviceContext* context, const T& data) {
        if (!buffer) return;  // 安全チェック

        if (std::memcmp(&cachedData, &data, sizeof(T)) != 0)
        {
            D3D11_MAPPED_SUBRESOURCE mapped{};
            HRESULT hr = context->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            if (SUCCEEDED(hr) && mapped.pData) 
            {
                std::memcpy(mapped.pData, &data, sizeof(T));
                context->Unmap(buffer.Get(), 0);
                cachedData = data;
            }
            else 
            {
                OutputDebugStringA("Map failed or pData is null\n");
            }
        }
    }

    // 強制更新
    void Update(ID3D11DeviceContext* context, const T& data) 
    {
        // GPUが使用するリソースのメモリにCPUからアクセスする際に利用する構造体
        D3D11_MAPPED_SUBRESOURCE mappedResource{};
        // 定数バッファをマップする
        context->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

        std::memcpy(mappedResource.pData, &data, sizeof(T));
       
        // マップ解除
        context->Unmap(buffer.Get(), 0);

        cachedData = data;
    }

    ID3D11Buffer* GetBuffer() const noexcept {
        return buffer.Get();
    }

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
    T cachedData{};  // 前回送信した内容
};