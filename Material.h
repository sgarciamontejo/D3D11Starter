#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include "Graphics.h"
class Material
{
	DirectX::XMFLOAT4 colorTint;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;

	// Example of arrays holding SRVs and Sampler States for a single material
	// - The array sizes below correspond to the maximum number of SRVs and samplers that can
	// be used simultaneously (during a single draw). This is NOT the maximum number in memory.
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRVs[128];
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplers[16];

public:
	Material(DirectX::XMFLOAT4 colorTint, Microsoft::WRL::ComPtr<ID3D11VertexShader> vs, Microsoft::WRL::ComPtr<ID3D11PixelShader> ps);
	~Material();
	DirectX::XMFLOAT4 GetColorTint();
	Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader();
	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader();
	void SetColorTint(DirectX::XMFLOAT4 newTint);
	void SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> vs);
	void SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> ps);
	void AddTextureSRV(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);
	void BindTexturesAndSamplers();
};

