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