#pragma once

#include <Windows.h>
#include <d3d11_1.h>
#include <string>
#include <wrl/client.h>
#include <d3d11shadertracing.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace Graphics
{
	// --- GLOBAL VARS ---

	// Primary D3D11 API objects
	inline Microsoft::WRL::ComPtr<ID3D11Device> Device;
	inline Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context;
	inline Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;

	// Rendering buffers
	inline Microsoft::WRL::ComPtr<ID3D11RenderTargetView> BackBufferRTV;
	inline Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthBufferDSV;

	inline Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer;

	// Debug Layer
	inline Microsoft::WRL::ComPtr<ID3D11InfoQueue> InfoQueue;

	// --- FUNCTIONS ---

	// Getters
	bool VsyncState();
	std::wstring APIName();

	// General functions
	HRESULT Initialize(unsigned int windowWidth, unsigned int windowHeight, HWND windowHandle, bool vsyncIfPossible);
	void ShutDown();
	void ResizeBuffers(unsigned int width, unsigned int height);

	// Shader loading helpers
	Microsoft::WRL::ComPtr<ID3D11PixelShader> LoadPixelShader(const wchar_t* compiledShaderPath);
	Microsoft::WRL::ComPtr<ID3D11VertexShader> LoadVertexShader(const wchar_t* compiledShaderPath);

	void ResizeConstantBufferHeap(unsigned int sizeInBytes);
	void FillAndBindNextConstantBuffer(void* buffData, unsigned int size, D3D11_SHADER_TYPE shaderType, unsigned int slot);

	// Debug Layer
	void PrintDebugMessages();
}