#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include "Mesh.h"
#include "Vertex.h"
#include "BufferStructs.h"
#include "GameEntity.h"
#include "Camera.h"
#include "Lights.h"
#include <vector>
#include "Sky.h"

class Game
{
public:
	// Basic OOP setup
	Game();
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:
	// GUI Control Variables
	bool activeWindow = true;
	bool noResize = false;
	bool demoOpen = false;
	float demoColor[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
	float shaderOffset[3];
	float shaderTint[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	int radioIndex = 0;
	int blurDistance;

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	// ambient lighting
	XMFLOAT3 ambientLight = XMFLOAT3(0, 0, 0);

	// directional lighting
	std::vector<Light> lights;

	// New Geometry
	std::vector<std::shared_ptr<Mesh>> meshes;

	// Sky
	std::shared_ptr<Sky> sky;

	// Game Entities
	std::vector<std::shared_ptr<GameEntity>> entities;

	// Camera
	std::vector<std::shared_ptr<Camera>> cameras;
	std::shared_ptr<Camera> activeCamera;

	// Shadows
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> shadowVS;
	XMFLOAT4X4 lightViewMatrix;
	XMFLOAT4X4 lightProjectionMatrix;

	// Post Process Resources
	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> ppVS;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> fullscreenVS;

	// Resources that are tied to a particular post process
	Microsoft::WRL::ComPtr<ID3D11PixelShader> ppPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppRTV; // Rendering
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppSRV; // Sampling




	// Initialization helper methods - feel free to customize, combine, remove, etc.
	//void LoadShaders();
	void CreateGeometry();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	std::vector<std::shared_ptr<Material>> materials;

	// Helpers
	void CreateShadowMapResources();
	void RenderShadowMap();
	void CreatePostProcessResource();
	void ResizedPostProcessResources();
	void UpdateImGui(float deltaTime);
	void BuildUI();
};