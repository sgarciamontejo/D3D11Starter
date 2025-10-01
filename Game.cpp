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

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
{
	// Calculate byte width
	unsigned int size = sizeof(VertexShaderData);
	size = (size + 15) / 16 * 16;
	// Constant Buffer
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = size;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	Graphics::Device->CreateBuffer(&cbDesc, 0, constBuffer.GetAddressOf());

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
	LoadShaders();
	CreateGeometry();

	cam = std::make_shared<Camera>(Window::AspectRatio());

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);

		// Set Const Buffer
		Graphics::Context->VSSetConstantBuffers(0, 1, constBuffer.GetAddressOf());
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
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
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

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	Vertex vertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};
	int numVertices = sizeof(vertices) / sizeof(vertices[0]);

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indices[] = { 0, 1, 2 };
	int numIndices = sizeof(indices) / sizeof(indices[0]);

	//mesh 1
	triangle = std::make_shared<Mesh>(vertices, numVertices, indices, numIndices);
	meshes.push_back(triangle);

	//mesh 2
	Vertex vertices_2[] =
	{
		{ XMFLOAT3(-0.75f, +0.75f, +0.0f), blue },
		{ XMFLOAT3(-0.50f, +0.75f, +0.0f), red },
		{ XMFLOAT3(-0.50f, +0.50f, 0.0f), red },
		{ XMFLOAT3(-0.75f, +0.50f, +0.0f), blue },
	};
	int numVertices_2 = sizeof(vertices_2) / sizeof(vertices_2[0]);
	unsigned int indices_2[] = { 0, 1, 2, 0, 2, 3 };
	int numIndices_2 = sizeof(indices_2) / sizeof(indices_2[0]);
	rectangle = std::make_shared<Mesh>(vertices_2, numVertices_2, indices_2, numIndices_2);
	meshes.push_back(rectangle);

	//mesh 3
	Vertex vertices_3[] =
	{
		{ XMFLOAT3(+0.67f, +0.75f, +0.0f), red },
		{ XMFLOAT3(+0.75f, +0.86f, +0.0f), red },
		{ XMFLOAT3(+0.83f, +0.75f, +0.0f), blue },
		{ XMFLOAT3(+0.80f, +0.60f, +0.0f), blue },
		{ XMFLOAT3(+0.70f, +0.60f, +0.0f), red},
	};
	int numVertices_3 = sizeof(vertices_3) / sizeof(vertices_3[0]);
	unsigned int indices_3[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4 };
	int numIndices_3 = sizeof(indices_3) / sizeof(indices_3[0]);
	pentagon = std::make_shared<Mesh>(vertices_3, numVertices_3, indices_3, numIndices_3);
	meshes.push_back(pentagon);

	std::shared_ptr<GameEntity> e1 = std::make_shared<GameEntity>(pentagon);
	std::shared_ptr<GameEntity> e2 = std::make_shared<GameEntity>(pentagon);
	std::shared_ptr<GameEntity> e3 = std::make_shared<GameEntity>(triangle);
	std::shared_ptr<GameEntity> e4 = std::make_shared<GameEntity>(rectangle);
	std::shared_ptr<GameEntity> e5 = std::make_shared<GameEntity>(rectangle);

	e1->GetTransform().MoveAbsolute(-0.2f, -0.5f, 0.0f);
	e2->GetTransform().MoveAbsolute(0.0f, -0.0f, 0.0f);
	e3->GetTransform().Rotate(0.0f, 0.0f, 3.14f);
	e4->GetTransform().MoveAbsolute(-0.2f, 0.0f, 0.0f);
	e5->GetTransform().MoveAbsolute(0.0f, -0.5f, 0.0f);
	

	entities.push_back(e1);
	entities.push_back(e2);
	entities.push_back(e3);
	entities.push_back(e4);
	entities.push_back(e5);
}

// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	if (cam) {
		cam->GetProjectionMatrix();
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	cam->Update(deltaTime);
	UpdateImGui(deltaTime);

	// Make changes to UI with this helper
	BuildUI();

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	entities[0]->GetTransform().SetPosition(-0.2f, (float)sin(totalTime)*0.5f-0.5f, 0);
	entities[2]->GetTransform().Rotate(0, 0, deltaTime * 1.0f);
	entities[3]->GetTransform().SetScale((float)sin(totalTime)*0.5+1.0, (float)sin(totalTime)*0.5+1.0, 0.0f);
	

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
			VertexShaderData cbData;
			cbData.colorTint = XMFLOAT4(shaderTint);
			cbData.world = entity->GetTransform().GetWorldMatrix();
			cbData.projection = cam->GetProjectionMatrix();
			cbData.view = cam->GetViewMatrix();

			D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
			Graphics::Context->Map(constBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
			memcpy(mappedBuffer.pData, &cbData, sizeof(cbData));
			Graphics::Context->Unmap(constBuffer.Get(), 0);

			entity->Draw();
		}

		ImGui::Render(); // Turns this frame�s UI into renderable triangles
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

	// Replace the %f with the next parameter, and format as a float
	ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);
	// Replace each %d with the next parameter, and format as decimal integers
	// The "x" will be printed as-is between the numbers, like so: 800x600
	ImGui::Text("Window Resolution: %dx%d", Window::Width(), Window::Height());

	ImGui::ColorEdit4("Background Color", &demoColor[0]);
	

	if (ImGui::Button("Show Demo Window"))
	{
		// This will only execute on frames in which the button is clicked
		demoOpen = !demoOpen;
	}

	// these are technically 3 elements including the header
	if (ImGui::TreeNode("Meshes"))
	{
		if (ImGui::TreeNode("Triangle")) {
			ImGui::Text("\tTriangles: %d", triangle->GetIndexCount() / 3);
			ImGui::Text("\tVertices: %d", triangle->GetVertexCount());
			ImGui::Text("\tIndices: %d", triangle->GetIndexCount());
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Rectangle")) {
			ImGui::Text("\tTriangles: %d", rectangle->GetIndexCount() / 3);
			ImGui::Text("\tVertices: %d", rectangle->GetVertexCount());
			ImGui::Text("\tIndices: %d", rectangle->GetIndexCount());
			ImGui::TreePop();
		}
		
		if (ImGui::TreeNode("Pentagon")) {
			ImGui::Text("\tTriangles: %d", pentagon->GetIndexCount() / 3);
			ImGui::Text("\tVertices: %d", pentagon->GetVertexCount());
			ImGui::Text("\tIndices: %d", pentagon->GetIndexCount());
			ImGui::TreePop();
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
	ImGui::ColorEdit4("Tint", shaderTint);

	ImGui::End(); //end window
}