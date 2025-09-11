#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Graphics.h"
#include "Vertex.h"


class Mesh
{
private:
	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	int numIndices; // num of indices - drawing
	int numVertices; // num of vertices - UI

	//future - add variables to store textures and shader data

public:
	Mesh(Vertex vertices[], int numVertices, unsigned int indices[], int numIndices);
	~Mesh();
	void Draw();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetVertexCount();
	int GetIndexCount();
};

