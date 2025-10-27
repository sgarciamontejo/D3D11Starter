#include "Material.h"

Material::Material(DirectX::XMFLOAT4 colorTint, Microsoft::WRL::ComPtr<ID3D11VertexShader> vs, Microsoft::WRL::ComPtr<ID3D11PixelShader> ps) {
	this->colorTint = colorTint;
	this->vs = vs;
	this->ps = ps;
}

Material::~Material() {

}

DirectX::XMFLOAT4 Material::GetColorTint() {
	return colorTint;
}

Microsoft::WRL::ComPtr<ID3D11VertexShader> Material::GetVertexShader() {
	return vs;
}

Microsoft::WRL::ComPtr<ID3D11PixelShader> Material::GetPixelShader() {
	return ps;
}

void Material::SetColorTint(DirectX::XMFLOAT4 colorTint) {
	this->colorTint = colorTint;
}

void Material::SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> vs) {
	this->vs = vs;
}

void Material::SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> ps) {
	this->ps = ps;
}

void Material::AddTextureSRV(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs[slot] = srv;
}

void Material::AddSampler(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers[slot] = sampler;
}

void Material::BindTexturesAndSamplers()
{
	for (int i = 0; i < textureSRVs.size) {
		Graphics::Context->PSSetShaderResources(tex)
	}
}
