#include "Material.h"

Material::Material(const char* name, DirectX::XMFLOAT4 colorTint, Microsoft::WRL::ComPtr<ID3D11VertexShader> vs, Microsoft::WRL::ComPtr<ID3D11PixelShader> ps, DirectX::XMFLOAT2 uvScale, DirectX::XMFLOAT2 uvOffset) {
	this->name = name;
	this->colorTint = colorTint;
	this->vs = vs;
	this->ps = ps;
	this->uvScale = uvScale;
	this->uvOffset = uvOffset;
}

Material::~Material() {

}

DirectX::XMFLOAT4 Material::GetColorTint() {
	return colorTint;
}

DirectX::XMFLOAT2 Material::GetUVScale()
{
	return uvScale;
}

DirectX::XMFLOAT2 Material::GetUVOffset()
{
	return uvOffset;
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

void Material::SetUVScale(DirectX::XMFLOAT2 uvScale)
{
	this->uvScale = uvScale;
}

void Material::SetUVOffset(DirectX::XMFLOAT2 uvOffset)
{
	this->uvOffset = uvOffset;
}

void Material::SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> vs) {
	this->vs = vs;
}

void Material::SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> ps) {
	this->ps = ps;
}

const char* Material::GetName() {
	return name;
}

std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> Material::GetTextureSRVs()
{
	return textureSRVs;
}

std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11SamplerState>> Material::GetSamplerMap()
{
	return samplers;
}

void Material::AddTextureSRV(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({ slot, srv });
}

void Material::AddSampler(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ slot, sampler });
}

void Material::BindTexturesAndSamplers()
{
	for (auto& t : textureSRVs) {
		Graphics::Context->PSSetShaderResources(t.first, 1, t.second.GetAddressOf());
	}

	for (auto& s : samplers) {
		Graphics::Context->PSSetSamplers(s.first, 1, s.second.GetAddressOf());
	}
}
