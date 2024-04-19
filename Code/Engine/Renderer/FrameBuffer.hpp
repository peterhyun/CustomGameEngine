#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <string>

class Renderer;
class FrameBuffer
{
	friend class Renderer; // Only the Renderer can create new FrameBuffer objects!

public:
	virtual ~FrameBuffer();

private:
	FrameBuffer() {}; // can't instantiate directly; must ask Renderer to do it for you
	FrameBuffer(FrameBuffer const& copy) = delete; // No copying allowed!  This represents GPU memory.

public:
	IntVec2				GetDimensions() const { return m_dimensions; }
	std::string const& GetImageFilePath() const { return m_name; }

protected:
	std::string			m_name;
	IntVec2				m_dimensions;
	struct ID3D11Texture2D* m_textureForRTV = nullptr;
	struct ID3D11Texture2D* m_textureForSRV = nullptr;	//Now I'm gonna copy RTV texture to SRV texture whenever I bind it
	struct ID3D11RenderTargetView* m_renderTargetViewForFrameBuffer = nullptr;
	struct ID3D11ShaderResourceView* m_shaderResourceViewForFrameBuffer = nullptr;
};