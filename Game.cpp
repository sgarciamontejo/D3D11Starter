#include "Game.h"
#include "Graphics.h"
#include "Input.h"
#include "Mesh.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Sky.h"

#include <DirectXMath.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include "WICTextureLoader.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
{
	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	CreateGeometry();

	cameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(0.0f, 0.0f, -10.0f))); // cam 1
	cameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(-5.0f, 2.25f, 10.0f))); // cam 2
	cameras[0]->transform.SetRotation(XMFLOAT3(0, 0.0f, 0));
	cameras[1]->transform.SetRotation(XMFLOAT3(0, 0.0f, 0));
	activeCamera = cameras[0];

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Create ring buffer
		Graphics::ResizeConstantBufferHeap(256 * 1000);

		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		// Create an input layout 
		//  - This describes the layout of data sent to a vertex shader
		//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
		//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
		//  - Luckily, we already have that loaded (the vertex shader blob above)
		{
			D3D11_INPUT_ELEMENT_DESC inputElements[4] = {};

			// Set up the first element - a position, which is 3 float values
			inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
			inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
			inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

			// Set up the second element - a UV map, which is 2 more float values
			inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT;			// 2x 32-bit floats
			inputElements[1].SemanticName = "TEXCOORD";							// Match our vertex shader input!
			inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

			// Set up the third element - a Normal map, which is 3 more float values
			inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;			// 3x 32-bit floats
			inputElements[2].SemanticName = "NORMAL";							// Match our vertex shader input!
			inputElements[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

			//Set up the fourth element - Tangent vector which is 3 more float values
			inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;			// 3x 32-bit floats
			inputElements[3].SemanticName = "TANGENT";						// Match the VS input
			inputElements[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; // after the previous element

			ID3DBlob* vertexShaderBlob;
			D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);
			// Create the input layout, verifying our description against actual shader code
			Graphics::Device->CreateInputLayout(
				inputElements,							// An array of descriptions
				4,										// How many elements in that array?
				vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
				vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
				inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
		}

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());
	}
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	XMFLOAT4 halfRed = XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
	XMFLOAT4 halfGreen = XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f);
	XMFLOAT4 halfBlue = XMFLOAT4(0.5f, 0.5f, 1.0f, 1.0f);

	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 purple = XMFLOAT4(0.5f, 0.0f, 0.5f, 1.0f);

	// Sampler State
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());

	// Create shadow texture
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = 1024; // shadowMapResolution
	shadowDesc.Height = 1024; // shadowMapResolution
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	Graphics::Device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Lights
	Light directionalLight1 = {};
	directionalLight1.Type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight1.Direction = XMFLOAT3(1.0f, -1.0f, -1.0f);
	directionalLight1.Color = XMFLOAT3(1.0f, 0.5f, 0.0f);
	directionalLight1.Intensity = 0.7f;

	Light directionalLight2 = {};
	directionalLight2.Type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight2.Direction = XMFLOAT3(-1.0f, -1.0f, -1.0f);
	directionalLight2.Color = XMFLOAT3(0.3f, 0.9f, 0.3f);
	directionalLight2.Intensity = 0.7f;

	Light pointLight1 = {};
	pointLight1.Type = LIGHT_TYPE_POINT;
	pointLight1.Position = XMFLOAT3(-5.0f, -5.0f, 5.0f);
	pointLight1.Color = XMFLOAT3(0.3f, 0.3f, 1);
	pointLight1.Intensity = 0.6f;
	pointLight1.Range = 20.0f;

	Light spotLight1 = {};
	spotLight1.Type = LIGHT_TYPE_SPOT;
	spotLight1.Position = XMFLOAT3(10.0f, 1.0f, 10.0f);
	spotLight1.Color = XMFLOAT3(0.9f, 0.2f, 0.2f);
	spotLight1.Intensity = 0.7f;
	spotLight1.Direction = XMFLOAT3(0, -1, 0);
	spotLight1.Range = 20.0f;
	spotLight1.SpotOuterAngle = XMConvertToRadians(20.0f);
	spotLight1.SpotInnerAngle = XMConvertToRadians(15.0f);

	lights.push_back(directionalLight1); 
	lights.push_back(directionalLight2);
	lights.push_back(pointLight1);
	lights.push_back(spotLight1);

	// Load Textures
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockResource;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockNormalsResource;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockNormalsResource;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cushionResource;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cushionNormalsResource;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeNormalsResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeMetalResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeRoughnessResource;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneNormalsResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneMetalResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneRoughnessResource;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorNormalsResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorMetalResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorRoughnessResource;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintNormalsResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintMetalResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintRoughnessResource;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughNormalsResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughMetalResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughRoughnessResource;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedNormalsResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedMetalResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedRoughnessResource;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodNormalsResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodMetalResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodRoughnessResource;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> flatNormalsResource;

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/bronze_albedo.png").c_str(), 0, bronzeResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/bronze_normals.png").c_str(), 0, bronzeNormalsResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/bronze_metals.png").c_str(), 0, bronzeMetalResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/bronze_roughness.png").c_str(), 0, bronzeRoughnessResource.GetAddressOf());

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/cobblestone_albedo.png").c_str(), 0, cobblestoneResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/cobblestone_normals.png").c_str(), 0, cobblestoneNormalsResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/cobblestone_metal.png").c_str(), 0, cobblestoneMetalResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/cobblestone_roughness.png").c_str(), 0, cobblestoneRoughnessResource.GetAddressOf());

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/floor_albedo.png").c_str(), 0, floorResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/floor_normals.png").c_str(), 0, floorNormalsResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/floor_metal.png").c_str(), 0, floorMetalResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/floor_roughness.png").c_str(), 0, floorRoughnessResource.GetAddressOf());

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/paint_albedo.png").c_str(), 0, paintResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/paint_normals.png").c_str(), 0, paintNormalsResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/paint_metal.png").c_str(), 0, paintMetalResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/paint_roughness.png").c_str(), 0, paintRoughnessResource.GetAddressOf());

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/rough_albedo.png").c_str(), 0, roughResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/rough_normals.png").c_str(), 0, roughNormalsResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/rough_metal.png").c_str(), 0, roughMetalResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/rough_roughness.png").c_str(), 0, roughRoughnessResource.GetAddressOf());

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/scratched_albedo.png").c_str(), 0, scratchedResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/scratched_normals.png").c_str(), 0, scratchedNormalsResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/scratched_metal.png").c_str(), 0, scratchedMetalResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/scratched_roughness.png").c_str(), 0, scratchedRoughnessResource.GetAddressOf());

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/wood_albedo.png").c_str(), 0, woodResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/wood_normals.png").c_str(), 0, woodNormalsResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/wood_metal.png").c_str(), 0, woodMetalResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/wood_roughness.png").c_str(), 0, woodRoughnessResource.GetAddressOf());

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/PBR/flat_normals.png").c_str(), 0, flatNormalsResource.GetAddressOf());

	// Load Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader> firstVertexShader = Graphics::LoadVertexShader(FixPath(L"VertexShader.cso").c_str());
	Microsoft::WRL::ComPtr<ID3D11PixelShader> firstPixelShader = Graphics::LoadPixelShader(FixPath(L"PixelShader.cso").c_str());
	//Microsoft::WRL::ComPtr<ID3D11PixelShader> comboPixelShader = Graphics::LoadPixelShader(FixPath(L"ComboPS.cso").c_str());
	Microsoft::WRL::ComPtr<ID3D11PixelShader> skyPixelShader = Graphics::LoadPixelShader(FixPath(L"SkyPS.cso").c_str());
	Microsoft::WRL::ComPtr<ID3D11VertexShader> skyVertexShader = Graphics::LoadVertexShader(FixPath(L"SkyVS.cso").c_str());
	/*Microsoft::WRL::ComPtr<ID3D11PixelShader> uvPixelShader = LoadPixelShader(L"DebugUVsPS.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> normalsPixelShader = LoadPixelShader(L"DebugNormalsPS.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> customPixelShader = LoadPixelShader(L"CustomPS.cso");*/

	// Load Meshes
	std::shared_ptr<Mesh> cube = std::make_shared<Mesh>("Cube", FixPath(L"../../Assets/Meshes/cube.obj").c_str());
	std::shared_ptr<Mesh> cylinder = std::make_shared<Mesh>("Cylinder", FixPath(L"../../Assets/Meshes/cylinder.obj").c_str());
	std::shared_ptr<Mesh> helix = std::make_shared<Mesh>("Helix", FixPath(L"../../Assets/Meshes/helix.obj").c_str());
	std::shared_ptr<Mesh> quad = std::make_shared<Mesh>("Quad", FixPath(L"../../Assets/Meshes/quad.obj").c_str());
	std::shared_ptr<Mesh> quad_double_sided = std::make_shared<Mesh>("Quad Double Sided", FixPath(L"../../Assets/Meshes/quad_double_sided.obj").c_str());
	std::shared_ptr<Mesh> sphere = std::make_shared<Mesh>("Sphere", FixPath(L"../../Assets/Meshes/sphere.obj").c_str());
	std::shared_ptr<Mesh> torus = std::make_shared<Mesh>("Torus", FixPath(L"../../Assets/Meshes/torus.obj").c_str());

	// Got this from the demo code, good shortcut to remember
	meshes.insert(meshes.end(), { cube, cylinder, helix, quad, quad_double_sided, sphere, torus });

	// Load Sky resources
	sky = std::make_shared<Sky>(
		FixPath(L"../../Assets/Skies/Planet/right.png").c_str(),
		FixPath(L"../../Assets/Skies/Planet/left.png").c_str(),
		FixPath(L"../../Assets/Skies/Planet/up.png").c_str(),
		FixPath(L"../../Assets/Skies/Planet/down.png").c_str(),
		FixPath(L"../../Assets/Skies/Planet/front.png").c_str(),
		FixPath(L"../../Assets/Skies/Planet/back.png").c_str(),
		cube,
		skyVertexShader,
		skyPixelShader,
		samplerState);

	// Create Materials
	//std::shared_ptr<Material> matRed = std::make_shared<Material>("Red", red, firstVertexShader, firstPixelShader);
	//std::shared_ptr<Material> matWhite = std::make_shared<Material>("White", white, firstVertexShader, firstPixelShader);
	//std::shared_ptr<Material> matPurple = std::make_shared<Material>("Purple", purple, firstVertexShader, firstPixelShader);

	//std::shared_ptr<Material> matUV = std::make_shared<Material>("UV", XMFLOAT4(1, 1, 1, 1), firstVertexShader, uvPixelShader);
	//std::shared_ptr<Material> matNormals = std::make_shared<Material>("Normals", XMFLOAT4(1, 1, 1, 1), firstVertexShader, normalsPixelShader);
	//std::shared_ptr<Material> matCustom = std::make_shared<Material>("Custom", XMFLOAT4(1, 1, 1, 1), firstVertexShader, customPixelShader);

	// Normal map + Metal + Roughness + Environment map materials
	std::shared_ptr<Material> matBronzeEnvMap = std::make_shared<Material>("Bronze with Env Map", XMFLOAT4(1, 1, 1, 1), 0.8f, firstVertexShader, firstPixelShader, XMFLOAT2(2, 2), XMFLOAT2(0, 0));
	matBronzeEnvMap->AddSampler(0, samplerState);
	matBronzeEnvMap->AddTextureSRV(0, bronzeResource);
	matBronzeEnvMap->AddTextureSRV(1, bronzeNormalsResource);
	matBronzeEnvMap->AddTextureSRV(2, bronzeRoughnessResource);
	matBronzeEnvMap->AddTextureSRV(3, bronzeMetalResource);
	matBronzeEnvMap->AddTextureSRV(4, sky->GetTexture());

	std::shared_ptr<Material> matCobblestoneEnvMap = std::make_shared<Material>("Cobblestone with Env Map", XMFLOAT4(1, 1, 1, 1), 0.9f, firstVertexShader, firstPixelShader, XMFLOAT2(2, 2), XMFLOAT2(0, 0));
	matCobblestoneEnvMap->AddSampler(0, samplerState);
	matCobblestoneEnvMap->AddTextureSRV(0, cobblestoneResource);
	matCobblestoneEnvMap->AddTextureSRV(1, cobblestoneNormalsResource);
	matCobblestoneEnvMap->AddTextureSRV(2, cobblestoneRoughnessResource);
	matCobblestoneEnvMap->AddTextureSRV(3, cobblestoneMetalResource);
	matCobblestoneEnvMap->AddTextureSRV(4, sky->GetTexture());

	std::shared_ptr<Material> matFloorEnvMap = std::make_shared<Material>("Floor with Env Map", XMFLOAT4(1, 1, 1, 1), 0.8f, firstVertexShader, firstPixelShader, XMFLOAT2(2, 2), XMFLOAT2(0, 0));
	matFloorEnvMap->AddSampler(0, samplerState);
	matFloorEnvMap->AddTextureSRV(0, floorResource);
	matFloorEnvMap->AddTextureSRV(1, floorNormalsResource);
	matFloorEnvMap->AddTextureSRV(2, floorRoughnessResource);
	matFloorEnvMap->AddTextureSRV(3, floorMetalResource);
	matFloorEnvMap->AddTextureSRV(4, sky->GetTexture());

	std::shared_ptr<Material> matPaintEnvMap = std::make_shared<Material>("Paint with Env Map", XMFLOAT4(1, 1, 1, 1), 0.7f, firstVertexShader, firstPixelShader, XMFLOAT2(2, 2), XMFLOAT2(0, 0));
	matPaintEnvMap->AddSampler(0, samplerState);
	matPaintEnvMap->AddTextureSRV(0, paintResource);
	matPaintEnvMap->AddTextureSRV(1, paintNormalsResource);
	matPaintEnvMap->AddTextureSRV(2, paintRoughnessResource);
	matPaintEnvMap->AddTextureSRV(3, paintMetalResource);
	matPaintEnvMap->AddTextureSRV(4, sky->GetTexture());

	std::shared_ptr<Material> matRoughEnvMap = std::make_shared<Material>("Rough Metal with Env Map", XMFLOAT4(1, 1, 1, 1), 0.7f, firstVertexShader, firstPixelShader, XMFLOAT2(2, 2), XMFLOAT2(0, 0));
	matRoughEnvMap->AddSampler(0, samplerState);
	matRoughEnvMap->AddTextureSRV(0, roughResource);
	matRoughEnvMap->AddTextureSRV(1, roughNormalsResource);
	matRoughEnvMap->AddTextureSRV(2, roughRoughnessResource);
	matRoughEnvMap->AddTextureSRV(3, roughMetalResource);
	matRoughEnvMap->AddTextureSRV(4, sky->GetTexture());

	std::shared_ptr<Material> matScratchedEnvMap = std::make_shared<Material>("Scratched Metal with Env Map", XMFLOAT4(1, 1, 1, 1), 0.8f, firstVertexShader, firstPixelShader, XMFLOAT2(2, 2), XMFLOAT2(0, 0));
	matScratchedEnvMap->AddSampler(0, samplerState);
	matScratchedEnvMap->AddTextureSRV(0, scratchedResource);
	matScratchedEnvMap->AddTextureSRV(1, scratchedNormalsResource);
	matScratchedEnvMap->AddTextureSRV(2, scratchedRoughnessResource);
	matScratchedEnvMap->AddTextureSRV(3, scratchedMetalResource);
	matScratchedEnvMap->AddTextureSRV(4, sky->GetTexture());

	std::shared_ptr<Material> matWoodEnvMap = std::make_shared<Material>("Wood Metal with Env Map", XMFLOAT4(1, 1, 1, 1), 0.8f, firstVertexShader, firstPixelShader, XMFLOAT2(0.5f, 0.5f), XMFLOAT2(0, 0));
	matWoodEnvMap->AddSampler(0, samplerState);
	matWoodEnvMap->AddTextureSRV(0, woodResource);
	matWoodEnvMap->AddTextureSRV(1, woodNormalsResource);
	matWoodEnvMap->AddTextureSRV(2, woodRoughnessResource);
	matWoodEnvMap->AddTextureSRV(3, woodMetalResource);
	matWoodEnvMap->AddTextureSRV(4, sky->GetTexture());

	materials.insert(materials.end(), { matBronzeEnvMap, matCobblestoneEnvMap, matFloorEnvMap, matPaintEnvMap, matRoughEnvMap, matScratchedEnvMap, matWoodEnvMap });


	//std::shared_ptr<GameEntity> cube1 = std::make_shared<GameEntity>(cube, matNormals);
	//std::shared_ptr<GameEntity> cube2 = std::make_shared<GameEntity>(cube, matUV);
	//std::shared_ptr<GameEntity> cube3 = std::make_shared<GameEntity>(cube, matCobblestoneEnvMap);
	//std::shared_ptr<GameEntity> cylinder1 = std::make_shared<GameEntity>(cylinder, matNormals);
	//std::shared_ptr<GameEntity> cylinder2 = std::make_shared<GameEntity>(cylinder, matUV);
	//std::shared_ptr<GameEntity> cylinder3 = std::make_shared<GameEntity>(cylinder, matRockEnvMap);
	//std::shared_ptr<GameEntity> helix1 = std::make_shared<GameEntity>(helix, matNormals);
	//std::shared_ptr<GameEntity> helix2 = std::make_shared<GameEntity>(helix, matUV);
	//std::shared_ptr<GameEntity> helix3 = std::make_shared<GameEntity>(helix, matCobblestoneEnvMap);
	//std::shared_ptr<GameEntity> sphere1 = std::make_shared<GameEntity>(sphere, matNormals);
	//std::shared_ptr<GameEntity> sphere2 = std::make_shared<GameEntity>(sphere, matUV);
	//std::shared_ptr<GameEntity> sphere3 = std::make_shared<GameEntity>(sphere, matCushionEnvMap);
	//std::shared_ptr<GameEntity> torus1 = std::make_shared<GameEntity>(torus, matNormals);
	//std::shared_ptr<GameEntity> torus2 = std::make_shared<GameEntity>(torus, matUV);
	//std::shared_ptr<GameEntity> torus3 = std::make_shared<GameEntity>(torus, matRockEnvMap);
	//std::shared_ptr<GameEntity> quad1 = std::make_shared<GameEntity>(quad, matNormals);
	//std::shared_ptr<GameEntity> quad2 = std::make_shared<GameEntity>(quad, matUV);
	//std::shared_ptr<GameEntity> quad3 = std::make_shared<GameEntity>(quad, matCobblestoneEnvMap);
	//std::shared_ptr<GameEntity> quad_double_sided1 = std::make_shared<GameEntity>(quad_double_sided, matNormals);
	//std::shared_ptr<GameEntity> quad_double_sided2 = std::make_shared<GameEntity>(quad_double_sided, matUV);
	//std::shared_ptr<GameEntity> quad_double_sided3 = std::make_shared<GameEntity>(quad_double_sided, matCushionEnvMap);

	std::shared_ptr<GameEntity> bronzeSphere = std::make_shared<GameEntity>(sphere, matBronzeEnvMap);
	std::shared_ptr<GameEntity> cobblestoneSphere = std::make_shared<GameEntity>(sphere, matCobblestoneEnvMap);
	std::shared_ptr<GameEntity> floorSphere = std::make_shared<GameEntity>(sphere, matFloorEnvMap);
	std::shared_ptr<GameEntity> paintSphere = std::make_shared<GameEntity>(sphere, matPaintEnvMap);
	std::shared_ptr<GameEntity> roughSphere = std::make_shared<GameEntity>(sphere, matRoughEnvMap);
	std::shared_ptr<GameEntity> scratchedSphere = std::make_shared<GameEntity>(sphere, matScratchedEnvMap);
	std::shared_ptr<GameEntity> woodSphere = std::make_shared<GameEntity>(sphere, matWoodEnvMap);
	std::shared_ptr<GameEntity> woodFloor = std::make_shared<GameEntity>(quad_double_sided, matWoodEnvMap);

	//cube1->GetTransform().MoveAbsolute(15.0f, 5.0f, -10.0f);
	//cube2->GetTransform().MoveAbsolute(15.0f, 0.0f, -10.0f);
	//cube3->GetTransform().MoveAbsolute(-15.0f, -5.0f, 10.0f);
	bronzeSphere->GetTransform().MoveAbsolute(-15.0f, -5.0f, 10.0f);
	//cylinder1->GetTransform().MoveAbsolute(10.0f, 5.0f, -10.0f);
	//cylinder2->GetTransform().MoveAbsolute(10.0f, 0.0f, -10.0f);
	//cylinder3->GetTransform().MoveAbsolute(-10.0f, -5.0f, 10.0f);
	cobblestoneSphere->GetTransform().MoveAbsolute(-10.0f, -5.0f, 10.0f);
	//helix1->GetTransform().MoveAbsolute(5.0f, 5.0f, -10.0f);
	//helix2->GetTransform().MoveAbsolute(5.0f, 0.0f, -10.0f);
	//helix3->GetTransform().MoveAbsolute(-5.0f, -5.0f, 10.0f);
	floorSphere->GetTransform().MoveAbsolute(-5.0f, -5.0f, 10.0f);
	//sphere1->GetTransform().MoveAbsolute(0.0f, 5.0f, -10.0f);
	//sphere2->GetTransform().MoveAbsolute(0.0f, 0.0f, -10.0f);
	//sphere3->GetTransform().MoveAbsolute(0.0f, -5.0f, 10.0f);
	paintSphere->GetTransform().MoveAbsolute(0.0f, -5.0f, 10.0f);
	//torus1->GetTransform().MoveAbsolute(-5.0f, 5.0f, -10.0f);
	//torus2->GetTransform().MoveAbsolute(-5.0f, 0.0f, -10.0f);
	//torus3->GetTransform().MoveAbsolute(5.0f, -5.0f, 10.0f);
	roughSphere->GetTransform().MoveAbsolute(5.0f, -5.0f, 10.0f);
	//quad1->GetTransform().MoveAbsolute(-10.0f, 5.0f, -10.0f);
	//quad2->GetTransform().MoveAbsolute(-10.0f, 0.0f, -10.0f);
	//quad3->GetTransform().MoveAbsolute(10.0f, -5.0f, 10.0f);
	scratchedSphere->GetTransform().MoveAbsolute(10.0f, -5.0f, 10.0f);
	//quad_double_sided1->GetTransform().MoveAbsolute(-15.0f, 5.0f, -10.0f);
	//quad_double_sided2->GetTransform().MoveAbsolute(-15.0f, 0.0f, -10.0f);
	//quad_double_sided3->GetTransform().MoveAbsolute(15.0f, -5.0f, 10.0f);
	woodSphere->GetTransform().MoveAbsolute(15.0f, -5.0f, 10.0f);
	woodFloor->GetTransform().MoveAbsolute(0.0f, -8.0f, 10.0f);
	woodFloor->GetTransform().SetScale(20.0f, 20.0f, 20.0f);

	//entities.push_back(cube1);
	//entities.push_back(cube2);
	entities.push_back(bronzeSphere);
	//entities.push_back(cylinder1);
	//entities.push_back(cylinder2);
	entities.push_back(cobblestoneSphere);
	//entities.push_back(helix1);
	//entities.push_back(helix2);
	entities.push_back(floorSphere);
	//entities.push_back(quad1);
	//entities.push_back(quad2);
	entities.push_back(paintSphere);
	//entities.push_back(quad_double_sided1);
	//entities.push_back(quad_double_sided2);
	entities.push_back(roughSphere);
	//entities.push_back(sphere1);
	//entities.push_back(sphere2);
	entities.push_back(scratchedSphere);
	//entities.push_back(torus1);
	//entities.push_back(torus2);
	//entities.push_back(sphere3);
	entities.push_back(woodSphere);
	entities.push_back(woodFloor);
}

// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	if (cameras.size() > 0) {
		for (int i = 0; i < cameras.size(); i++) {
			cameras[i]->GetProjectionMatrix();
		}
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	activeCamera->Update(deltaTime);
	UpdateImGui(deltaTime);

	// Make changes to UI with this helper
	BuildUI();

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	//entities[0]->GetTransform().SetPosition(-0.2f, (float)sin(totalTime)*0.5f-0.5f, 0);
	//entities[2]->GetTransform().Rotate(0, deltaTime * 1.0f, 0);
	//entities[3]->GetTransform().SetScale((float)sin(totalTime)*0.5f+1.0f, (float)sin(totalTime)*0.5f+1.0f, 0.0f);
	//for (std::shared_ptr<GameEntity> entity : entities) {
	//	Transform transform = entity->GetTransform();
	//	XMFLOAT3 currentPYR = transform.GetPitchYawRoll();
	//	entity->GetTransform().Rotate(currentPYR.x, deltaTime * 1.0f, currentPYR.z);
	//}
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	demoColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	{

		// loop through entities and draw them
		for (std::shared_ptr<GameEntity> entity : entities) {
			// Bind textures and samplers
			entity->GetMaterial()->BindTexturesAndSamplers();

			// Bind material shaders
			Graphics::Context->VSSetShader(entity->GetMaterial()->GetVertexShader().Get(), 0, 0);
			Graphics::Context->PSSetShader(entity->GetMaterial()->GetPixelShader().Get(), 0, 0);

			// VS DATA
			VertexShaderData vsData;
			//cbData.colorTint = entity->GetMaterial()->GetColorTint();
			vsData.world = entity->GetTransform().GetWorldMatrix();
			vsData.worldInvTranspose = entity->GetTransform().GetWorldInverseTransposeMatrix();
			vsData.projection = activeCamera->GetProjectionMatrix();
			vsData.view = activeCamera->GetViewMatrix();
			Graphics::FillAndBindNextConstantBuffer(&vsData, sizeof(VertexShaderData), D3D11_VERTEX_SHADER, 0);

			// PS DATA
			PixelShaderData psData;
			memcpy(&psData.lights, &lights[0], sizeof(Light) * (int)lights.size());
			psData.lightCount = (int)lights.size();
			psData.ambientLight = ambientLight;
			psData.colorTint = entity->GetMaterial()->GetColorTint();
			psData.roughness = entity->GetMaterial()->GetRoughness();
			psData.cameraPos = activeCamera->transform.GetPosition();
			psData.uvScale = entity->GetMaterial()->GetUVScale();
			psData.uvOffset = entity->GetMaterial()->GetUVOffset();
			
			Graphics::FillAndBindNextConstantBuffer(&psData, sizeof(PixelShaderData), D3D11_PIXEL_SHADER, 0);

			entity->Draw();
		}

		// draw sky after normal entities
		sky->Draw(activeCamera);

		ImGui::Render(); // Turns this frame’s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}

void Game::UpdateImGui(float deltaTime) {
	// Put this all in a helper method that is called from Game::Update()
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
	// Show the demo window
	if (demoOpen) {
		ImGui::ShowDemoWindow();
	}
}

void Game::BuildUI() {
	ImGuiWindowFlags window_flags = 0;
	if (noResize) {
		window_flags |= ImGuiWindowFlags_NoResize;
	}

	ImGui::Begin("Info", &activeWindow, window_flags);

	if (ImGui::Button("Show Demo Window"))
	{
		// This will only execute on frames in which the button is clicked
		demoOpen = !demoOpen;
	}

	// Replace the %f with the next parameter, and format as a float
	//ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);
	// Replace each %d with the next parameter, and format as decimal integers
	// The "x" will be printed as-is between the numbers, like so: 800x600
	//ImGui::Text("Window Resolution: %dx%d", Window::Width(), Window::Height());

	//ImGui::ColorEdit4("Background Color", &demoColor[0]);
	//ImGui::ColorEdit4("Tint", shaderTint);

	// these are technically 3 elements including the header
	if (ImGui::TreeNode("Meshes"))
	{
		for (int i = 0; i < meshes.size(); i++) {
			if (ImGui::TreeNode(meshes[i]->GetName())) {
				ImGui::Text("\tTriangles: %d", meshes[i]->GetIndexCount() / 3);
				ImGui::Text("\tVertices: %d", meshes[i]->GetVertexCount());
				ImGui::Text("\tIndices: %d", meshes[i]->GetIndexCount());
				ImGui::TreePop();
			}
		}
		// close node tree
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Materials"))
	{
		for (int i = 0; i < materials.size(); i++) {
			if (ImGui::TreeNode(materials[i]->GetName())) {
				XMFLOAT4 tint = materials[i]->GetColorTint();
				XMFLOAT2 scale = materials[i]->GetUVScale();
				XMFLOAT2 offset = materials[i]->GetUVOffset();

				if (ImGui::DragFloat4("\tColor Tint", &tint.x, 0.01f, 0.0f, 1.0f)) materials[i]->SetColorTint(tint);
				if (ImGui::DragFloat2("\tUV Scale", &scale.x, 0.5f, 0.0f, 10.0f)) materials[i]->SetUVScale(scale);
				if (ImGui::DragFloat2("\tUV OFfset", &offset.x, 0.05f, 0.0f, 10.0f)) materials[i]->SetUVOffset(offset);
				
				for(auto& tex : materials[i]->GetTextureSRVs()) {
					ImGui::Text("\n\tTexture Slot %d", tex.first);
					ImGui::Image(tex.second.Get(), ImVec2(256, 256));
				}

				ImGui::TreePop();
			}
		}
		// close node tree
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Entities"))
	{
		for (int i = 0; i < entities.size(); i++) {
			ImGui::PushID(entities[i].get());
			if (ImGui::TreeNode("Entity Node", "Entity %d", i+1)) {
				std::shared_ptr<GameEntity> entity = entities[i];
				std::shared_ptr<Mesh> mesh = entity->GetMesh();

				//Transform
				Transform& transform = entity->GetTransform();
				XMFLOAT3 position = transform.GetPosition();
				XMFLOAT3 rotation = transform.GetPitchYawRoll();
				XMFLOAT3 scale = transform.GetScale();

				if (ImGui::DragFloat3("Position", &position.x, 0.01f)) transform.SetPosition(position);
				if (ImGui::DragFloat3("Rotation", &rotation.x, 0.01f)) transform.SetRotation(rotation);
				if (ImGui::DragFloat3("Scale", &scale.x, 0.01f)) transform.SetScale(scale);
				//ImGui::Text("\tIndex Count: %d", entity->GetMesh()->GetIndexCount());
				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		// close node tree
		ImGui::TreePop();
	}

	//Lights
	if (ImGui::TreeNode("Lights")) {
		if (ImGui::ColorEdit3("Ambience", &ambientLight.x)) {}

		for (int i = 0; i < lights.size(); i++) {
			ImGui::PushID(&lights[i]);

			if (ImGui::TreeNode("Light Node", "Light %d", i + 1)) {
				if (ImGui::ColorEdit3("Color", &lights[i].Color.x)) {}
				if (ImGui::DragFloat("Intensity", &lights[i].Intensity)) {}

				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		ImGui::TreePop();
	}
	
	// Cameras
	if (ImGui::TreeNode("Camera Info")) {
		for (int i = 0; i < cameras.size(); i++) {
			ImGui::PushID(cameras[i].get());
			if (ImGui::TreeNode("Camera Node", "Camera %d", i + 1)) {
				Transform& transform = cameras[i]->transform;
				XMFLOAT3 position = transform.GetPosition();
				XMFLOAT3 rotation = transform.GetPitchYawRoll();
				float fovDegrees = XMConvertToDegrees(cameras[i]->fov);

				if(ImGui::DragFloat3("\tPosition", &position.x, 0.01f)) transform.SetPosition(position);
				if(ImGui::DragFloat3("\tRotation", &rotation.x, 0.01f)) transform.SetRotation(rotation);
				if (ImGui::DragFloat("\tField of View", &fovDegrees, 0.5f, 45.0f, 90.0f)) {
					cameras[i]->fov = XMConvertToRadians(fovDegrees);
					cameras[i]->UpdateProjectionMatrix(Window::AspectRatio());
				}
				ImGui::Text("\tProjection: %s", cameras[i]->isometric ? "Isometric" : "Perspective");

				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Active Camera")) {
		if (ImGui::RadioButton("Camera 1", &radioIndex, 0)) {
			activeCamera = cameras[0];
		}
		if (ImGui::RadioButton("Camera 2", &radioIndex, 1)) {
			activeCamera = cameras[1];
		}
		ImGui::TreePop();
	}

	ImGui::End(); //end window
}