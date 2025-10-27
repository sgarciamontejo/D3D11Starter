#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include "Mesh.h"
#include "Vertex.h"
#include "BufferStructs.h"
#include "GameEntity.h"
#include "Camera.h"
#include <vector>

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

	// New Geometry
	std::vector<std::shared_ptr<Mesh>> meshes;

	// Game Entities
	std::vector<std::shared_ptr<GameEntity>> entities;

	// Camera
	std::vector<std::shared_ptr<Camera>> cameras;
	std::shared_ptr<Camera> activeCamera;

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	//void LoadShaders();
	void CreateGeometry();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Shaders and shader-related constructs
	//Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	//Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// Const Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> vs_constBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> ps_constBuffer;

	// Helpers
	void UpdateImGui(float deltaTime);
	void BuildUI();
};

