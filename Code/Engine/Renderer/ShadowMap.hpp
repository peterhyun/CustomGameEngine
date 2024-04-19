#pragma once

class ShadowMap
{
	friend class Renderer;

public:
	~ShadowMap();
	struct IntVec2 GetDims() const;

private:
	ShadowMap(unsigned int width, unsigned int height);

protected:
	const unsigned int m_width;
	const unsigned int m_height;
	struct ID3D11Texture2D* m_shadowMapTexture = nullptr;
	struct ID3D11DepthStencilView* m_shadowMapDSV = nullptr;
	struct ID3D11ShaderResourceView* m_shadowMapSRV = nullptr;
};