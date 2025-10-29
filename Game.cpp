#include "Game.h"
#include "Graphics.h"
#include "Input.h"
#include "Mesh.h"
#include "PathHelpers.h"
#include "Window.h"

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
	//// Create Vertex Shader Constant Buffer
	//// Calculate byte width
	//unsigned int vsSize = sizeof(VertexShaderData);
	//vsSize = (vsSize + 15) / 16 * 16;
	//D3D11_BUFFER_DESC cbDesc = {};
	//cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//cbDesc.ByteWidth = vsSize;
	//cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	//Graphics::Device->CreateBuffer(&cbDesc, 0, vs_constBuffer.GetAddressOf());

	//// Create Pixel Shader Constant Buffer
	//unsigned int psSize = sizeof(PixelShaderData);
	//psSize = (psSize + 15) / 16 * 16;
	//D3D11_BUFFER_DESC ps_cbDesc = {};
	//ps_cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//ps_cbDesc.ByteWidth = psSize;
	//ps_cbDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	//ps_cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	//Graphics::Device->CreateBuffer(&ps_cbDesc, 0, ps_constBuffer.GetAddressOf());

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

	cameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(0.0f, 0.0f, 10.0f))); // cam 1
	cameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(-5.0f, 2.25f, 10.0f))); // cam 2
	cameras[0]->transform.SetRotation(XMFLOAT3(0, 3.14f, 0));
	cameras[1]->transform.SetRotation(XMFLOAT3(0, 3.14f, 0));
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
			D3D11_INPUT_ELEMENT_DESC inputElements[3] = {};

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

			ID3DBlob* vertexShaderBlob;
			D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);
			// Create the input layout, verifying our description against actual shader code
			Graphics::Device->CreateInputLayout(
				inputElements,							// An array of descriptions
				3,										// How many elements in that array?
				vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
				vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
				inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
		}

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		//Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		//Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);

		// Set Const Buffer
		//Graphics::Context->VSSetConstantBuffers(0, 1, vs_constBuffer.GetAddressOf());
		//Graphics::Context->PSSetConstantBuffers(0, 1, ps_constBuffer.GetAddressOf());
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


//Microsoft::WRL::ComPtr<ID3D11VertexShader> LoadVertexShader(std::wstring filePath) {
//	ID3DBlob* vertexShaderBlob;
//	Microsoft::WRL::ComPtr<ID3D11VertexShader> shader;
//
//	D3DReadFileToBlob(FixPath(filePath).c_str(), &vertexShaderBlob);
//	Graphics::Device->CreateVertexShader(
//		vertexShaderBlob->GetBufferPointer(),	// Pointer to start of binary data
//		vertexShaderBlob->GetBufferSize(),		// How big is the data
//		0,										// No classes in the shader
//		shader.GetAddressOf()					// ID3D11VertexShader**
//	);
//
//	return shader;
//}
//
//Microsoft::WRL::ComPtr<ID3D11PixelShader> LoadPixelShader(std::wstring filePath) {
//	ID3DBlob* pixelShaderBlob;
//	Microsoft::WRL::ComPtr<ID3D11PixelShader> shader;
//
//	D3DReadFileToBlob(FixPath(filePath).c_str(), &pixelShaderBlob);
//	Graphics::Device->CreatePixelShader(
//		pixelShaderBlob->GetBufferPointer(),	// Pointer to start of binary data
//		pixelShaderBlob->GetBufferSize(),		// How big is the data
//		0,										// No classes in the shader
//		shader.GetAddressOf()					// ID3D11PixelShader**
//	);
//	
//	return shader;
//}

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

	// Load Textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockWallResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodTableResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> crackedWallResource;

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Rock Wall/rock_wall_15_diff_4k.jpg").c_str(), 0, rockWallResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Cracks.jpg").c_str(), 0, crackedWallResource.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Wooden Table/wood_table_diff_4k.jpg").c_str(), 0, woodTableResource.GetAddressOf());

	// Load Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader> firstVertexShader = Graphics::LoadVertexShader(FixPath(L"VertexShader.cso").c_str());
	Microsoft::WRL::ComPtr<ID3D11PixelShader> firstPixelShader = Graphics::LoadPixelShader(FixPath(L"PixelShader.cso").c_str());
	Microsoft::WRL::ComPtr<ID3D11PixelShader> comboPixelShader = Graphics::LoadPixelShader(FixPath(L"ComboPS.cso").c_str());
	/*Microsoft::WRL::ComPtr<ID3D11PixelShader> uvPixelShader = LoadPixelShader(L"DebugUVsPS.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> normalsPixelShader = LoadPixelShader(L"DebugNormalsPS.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> customPixelShader = LoadPixelShader(L"CustomPS.cso");*/

	// Create Materials
	//std::shared_ptr<Material> matRed = std::make_shared<Material>("Red", red, firstVertexShader, firstPixelShader);
	//std::shared_ptr<Material> matWhite = std::make_shared<Material>("White", white, firstVertexShader, firstPixelShader);
	//std::shared_ptr<Material> matPurple = std::make_shared<Material>("Purple", purple, firstVertexShader, firstPixelShader);

	//std::shared_ptr<Material> matUV = std::make_shared<Material>("UV", XMFLOAT4(1, 1, 1, 1), firstVertexShader, uvPixelShader);
	//std::shared_ptr<Material> matNormals = std::make_shared<Material>("Normals", XMFLOAT4(1, 1, 1, 1), firstVertexShader, normalsPixelShader);
	//std::shared_ptr<Material> matCustom = std::make_shared<Material>("Custom", XMFLOAT4(1, 1, 1, 1), firstVertexShader, customPixelShader);

	std::shared_ptr<Material> matWood = std::make_shared<Material>("Wood", XMFLOAT4(1, 1, 1, 1), firstVertexShader, firstPixelShader, XMFLOAT2(1, 1), XMFLOAT2(0, 0));
	matWood->AddSampler(0, samplerState);
	matWood->AddTextureSRV(0, woodTableResource);

	std::shared_ptr<Material> matRock = std::make_shared<Material>("Rock", XMFLOAT4(1, 1, 1, 1), firstVertexShader, firstPixelShader, XMFLOAT2(1,1), XMFLOAT2(0, 0));
	matRock->AddSampler(0, samplerState);
	matRock->AddTextureSRV(0, rockWallResource);

	std::shared_ptr<Material> matCrackedRock = std::make_shared<Material>("Cracked Rock", XMFLOAT4(1, 1, 1, 1), firstVertexShader, comboPixelShader, XMFLOAT2(2,2), XMFLOAT2(0, 0));
	matCrackedRock->AddSampler(0, samplerState);
	matCrackedRock->AddTextureSRV(0, rockWallResource);
	matCrackedRock->AddTextureSRV(1, crackedWallResource);
		
	materials.insert(materials.end(), { matRock, matWood, matCrackedRock });

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


	//std::shared_ptr<GameEntity> cube1 = std::make_shared<GameEntity>(cube, matNormals);
	//std::shared_ptr<GameEntity> cube2 = std::make_shared<GameEntity>(cube, matUV);
	std::shared_ptr<GameEntity> cube3 = std::make_shared<GameEntity>(cube, matWood);
	//std::shared_ptr<GameEntity> cylinder1 = std::make_shared<GameEntity>(cylinder, matNormals);
	//std::shared_ptr<GameEntity> cylinder2 = std::make_shared<GameEntity>(cylinder, matUV);
	std::shared_ptr<GameEntity> cylinder3 = std::make_shared<GameEntity>(cylinder, matCrackedRock);
	//std::shared_ptr<GameEntity> helix1 = std::make_shared<GameEntity>(helix, matNormals);
	//std::shared_ptr<GameEntity> helix2 = std::make_shared<GameEntity>(helix, matUV);
	std::shared_ptr<GameEntity> helix3 = std::make_shared<GameEntity>(helix, matWood);
	//std::shared_ptr<GameEntity> sphere1 = std::make_shared<GameEntity>(sphere, matNormals);
	//std::shared_ptr<GameEntity> sphere2 = std::make_shared<GameEntity>(sphere, matUV);
	std::shared_ptr<GameEntity> sphere3 = std::make_shared<GameEntity>(sphere, matRock);
	//std::shared_ptr<GameEntity> torus1 = std::make_shared<GameEntity>(torus, matNormals);
	//std::shared_ptr<GameEntity> torus2 = std::make_shared<GameEntity>(torus, matUV);
	std::shared_ptr<GameEntity> torus3 = std::make_shared<GameEntity>(torus, matWood);
	//std::shared_ptr<GameEntity> quad1 = std::make_shared<GameEntity>(quad, matNormals);
	//std::shared_ptr<GameEntity> quad2 = std::make_shared<GameEntity>(quad, matUV);
	std::shared_ptr<GameEntity> quad3 = std::make_shared<GameEntity>(quad, matRock);
	//std::shared_ptr<GameEntity> quad_double_sided1 = std::make_shared<GameEntity>(quad_double_sided, matNormals);
	//std::shared_ptr<GameEntity> quad_double_sided2 = std::make_shared<GameEntity>(quad_double_sided, matUV);
	std::shared_ptr<GameEntity> quad_double_sided3 = std::make_shared<GameEntity>(quad_double_sided, matWood);

	//cube1->GetTransform().MoveAbsolute(15.0f, 5.0f, -10.0f);
	//cube2->GetTransform().MoveAbsolute(15.0f, 0.0f, -10.0f);
	cube3->GetTransform().MoveAbsolute(15.0f, -5.0f, -10.0f);
	//cylinder1->GetTransform().MoveAbsolute(10.0f, 5.0f, -10.0f);
	//cylinder2->GetTransform().MoveAbsolute(10.0f, 0.0f, -10.0f);
	cylinder3->GetTransform().MoveAbsolute(10.0f, -5.0f, -10.0f);
	//helix1->GetTransform().MoveAbsolute(5.0f, 5.0f, -10.0f);
	//helix2->GetTransform().MoveAbsolute(5.0f, 0.0f, -10.0f);
	helix3->GetTransform().MoveAbsolute(5.0f, -5.0f, -10.0f);
	//sphere1->GetTransform().MoveAbsolute(0.0f, 5.0f, -10.0f);
	//sphere2->GetTransform().MoveAbsolute(0.0f, 0.0f, -10.0f);
	sphere3->GetTransform().MoveAbsolute(0.0f, -5.0f, -10.0f);
	//torus1->GetTransform().MoveAbsolute(-5.0f, 5.0f, -10.0f);
	//torus2->GetTransform().MoveAbsolute(-5.0f, 0.0f, -10.0f);
	torus3->GetTransform().MoveAbsolute(-5.0f, -5.0f, -10.0f);
	//quad1->GetTransform().MoveAbsolute(-10.0f, 5.0f, -10.0f);
	//quad2->GetTransform().MoveAbsolute(-10.0f, 0.0f, -10.0f);
	quad3->GetTransform().MoveAbsolute(-10.0f, -5.0f, -10.0f);
	//quad_double_sided1->GetTransform().MoveAbsolute(-15.0f, 5.0f, -10.0f);
	//quad_double_sided2->GetTransform().MoveAbsolute(-15.0f, 0.0f, -10.0f);
	quad_double_sided3->GetTransform().MoveAbsolute(-15.0f, -5.0f, -10.0f);

	//entities.push_back(cube1);
	//entities.push_back(cube2);
	entities.push_back(cube3);
	//entities.push_back(cylinder1);
	//entities.push_back(cylinder2);
	entities.push_back(cylinder3);
	//entities.push_back(helix1);
	//entities.push_back(helix2);
	entities.push_back(helix3);
	//entities.push_back(quad1);
	//entities.push_back(quad2);
	entities.push_back(quad3);
	//entities.push_back(quad_double_sided1);
	//entities.push_back(quad_double_sided2);
	entities.push_back(quad_double_sided3);
	//entities.push_back(sphere1);
	//entities.push_back(sphere2);
	entities.push_back(sphere3);
	//entities.push_back(torus1);
	//entities.push_back(torus2);
	entities.push_back(torus3);
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
			vsData.projection = activeCamera->GetProjectionMatrix();
			vsData.view = activeCamera->GetViewMatrix();
			Graphics::FillAndBindNextConstantBuffer(&vsData, sizeof(VertexShaderData), D3D11_VERTEX_SHADER, 0);

			// PS DATA
			PixelShaderData psData;
			psData.colorTint = entity->GetMaterial()->GetColorTint();
			psData.uvScale = entity->GetMaterial()->GetUVScale();
			psData.uvOffset = entity->GetMaterial()->GetUVOffset();
			Graphics::FillAndBindNextConstantBuffer(&psData, sizeof(PixelShaderData), D3D11_PIXEL_SHADER, 0);

			//D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
			// map the Vertex Shader cb
			//Graphics::Context->Map(vs_constBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
			//memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
			//Graphics::Context->Unmap(vs_constBuffer.Get(), 0);

			//// map the Pixel Shader cb
			//Graphics::Context->Map(ps_constBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
			//memcpy(mappedBuffer.pData, &psData, sizeof(psData));
			//Graphics::Context->Unmap(ps_constBuffer.Get(), 0);
			

			entity->Draw();
		}

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
	ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);
	// Replace each %d with the next parameter, and format as decimal integers
	// The "x" will be printed as-is between the numbers, like so: 800x600
	ImGui::Text("Window Resolution: %dx%d", Window::Width(), Window::Height());

	ImGui::ColorEdit4("Background Color", &demoColor[0]);
	ImGui::ColorEdit4("Tint", shaderTint);

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