#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <unordered_map>
#include "Graphics.h"

class Material
{
	const char* name;
	DirectX::XMFLOAT4 colorTint;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
	DirectX::XMFLOAT2 uvScale = DirectX::XMFLOAT2(1.0f, 1.0f);
	DirectX::XMFLOAT2 uvOffset = DirectX::XMFLOAT2(0, 0);

	// Example of arrays holding SRVs and Sampler States for a single material
	// - The array sizes below correspond to the maximum number of SRVs and samplers that can
	// be used simultaneously (during a single draw). This is NOT the maximum number in memory.
	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;

public:
	Material(const char* name, DirectX::XMFLOAT4 colorTint, Microsoft::WRL::ComPtr<ID3D11VertexShader> vs, Microsoft::WRL::ComPtr<ID3D11PixelShader> ps, DirectX::XMFLOAT2 uvScale, DirectX::XMFLOAT2 uvOffset);
	~Material();

	DirectX::XMFLOAT4 GetColorTint();
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT2 GetUVOffset();
	Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader();
	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader();
	const char* GetName();

	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> GetTextureSRVs();
	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11SamplerState>> GetSamplerMap();

	void SetColorTint(DirectX::XMFLOAT4 newTint);
	void SetUVScale(DirectX::XMFLOAT2 uvScale);
	void SetUVOffset(DirectX::XMFLOAT2 uvOffset);
	void SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> vs);
	void SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> ps);

	void AddTextureSRV(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);
	void BindTexturesAndSamplers();
};

