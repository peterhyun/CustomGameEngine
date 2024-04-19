#include "Engine/Renderer/ShadowMap.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/IntVec2.hpp"

//Including directx 11 header files and such
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

ShadowMap::~ShadowMap()
{
	DX_SAFE_RELEASE(m_shadowMapTexture);
	DX_SAFE_RELEASE(m_shadowMapDSV);
	DX_SAFE_RELEASE(m_shadowMapSRV);
}

ShadowMap::ShadowMap(unsigned int width, unsigned int height) : m_width(width), m_height(height)
{
}

IntVec2 ShadowMap::GetDims() const
{
	return IntVec2(m_width, m_height);
}
