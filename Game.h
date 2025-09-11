#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include "Mesh.h"
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

	// New Geometry
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::shared_ptr<Mesh> triangle; 
	std::shared_ptr<Mesh> rectangle;
	std::shared_ptr<Mesh> pentagon;

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void CreateGeometry();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// Helpers
	void UpdateImGui(float deltaTime);
	void BuildUI();
};

