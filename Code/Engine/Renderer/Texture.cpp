#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Renderer.hpp"

//Including directx 11 header files and such
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

Texture::~Texture()
{
	DX_SAFE_RELEASE(m_shaderResourceView);
	DX_SAFE_RELEASE(m_texture);
}
