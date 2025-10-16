#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Graphics.h"
#include "Vertex.h"


class Mesh
{
private:
	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vb;
	Microsoft::WRL::ComPtr<ID3D11Buffer> ib;
	int numIndices = 0; // num of indices - drawing
	int numVertices = 0; // num of vertices - UI
	const char* name; // name displayed in UI

	//future - add variables to store textures and shader data

public:
	Mesh(const char* name, Vertex vertices[], int numVertices, unsigned int indices[], int numIndices);
	Mesh(const char* name, const std::wstring& objFile);
	~Mesh();
	void Draw();
	void CreateBuffers(Vertex* vertices, int numVertices, unsigned int* indices, int numIndices);
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetVertexCount();
	int GetIndexCount();
	const char* GetName();
};

