#pragma once

#include "Mesh.h"
#include "Camera.h"
#include <wrl/client.h>
#include <memory>

class Sky
{
private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState; // sampler options
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeMapSRV; // sky texture
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencil; // adjust depth buffer comparison type
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer; // rasterizer options (culling)
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader; // sky specific pixel shader
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader; // sky specific vertex shader
	std::shared_ptr<Mesh> skyMesh;

public:
	Sky(const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back,
		std::shared_ptr<Mesh> skyMesh,
		Microsoft::WRL::ComPtr<ID3D11VertexShader> skyVS,
		Microsoft::WRL::ComPtr<ID3D11PixelShader> skyPS,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState
	);
	~Sky();
	void Draw(std::shared_ptr<Camera> camera);

	// --------------------------------------------------------
	// Author: Chris Cascioli
	// Purpose: Creates a cube map on the GPU from 6 individual textures
	//
	// - Note: This code assumes you’re putting the function in Sky.cpp, 
	//   you’ve included WICTextureLoader.h and you have an ID3D11Device 
	//   ComPtr called “device”.  Make any adjustments necessary for
	//   your own implementation.
	// --------------------------------------------------------

	// --- HEADER ---

	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTexture();
};

