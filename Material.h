#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>
class Material
{
	DirectX::XMFLOAT4 colorTint;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;

public:
	Material(DirectX::XMFLOAT4 colorTint, Microsoft::WRL::ComPtr<ID3D11VertexShader> vs, Microsoft::WRL::ComPtr<ID3D11PixelShader> ps);
	~Material();
	DirectX::XMFLOAT4 GetColorTint();
	Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader();
	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader();
	void SetColorTint(DirectX::XMFLOAT4 newTint);
	void SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> vs);
	void SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> ps);
};

