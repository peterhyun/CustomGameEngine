#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"

Image::Image()
{
}

Image::~Image()
{
}

Image::Image(char const* imageFilePath) : m_imageFilePath(imageFilePath)
{
	int bytesPerTexel = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int numComponentsRequested = 0; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load(1); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* texelData = stbi_load(imageFilePath, &m_dimensions.x, &m_dimensions.y, &bytesPerTexel, numComponentsRequested);
	int texelDataLength = m_dimensions.x * m_dimensions.y * bytesPerTexel;
	// Check if the load was successful
	GUARANTEE_OR_DIE(texelData, Stringf("Failed to load image \"%s\"", imageFilePath));

	m_rgba8Texels.reserve(m_dimensions.x * m_dimensions.y);
	for (int texelIndex = 0; texelIndex < texelDataLength; texelIndex += bytesPerTexel) {
		if (bytesPerTexel == 1)
			m_rgba8Texels.push_back(Rgba8(texelData[texelIndex], texelData[texelIndex], texelData[texelIndex]));
		else if(bytesPerTexel == 3)
			m_rgba8Texels.push_back(Rgba8(texelData[texelIndex], texelData[texelIndex + 1], texelData[texelIndex + 2]));
		else if(bytesPerTexel == 4)
			m_rgba8Texels.push_back(Rgba8(texelData[texelIndex], texelData[texelIndex + 1], texelData[texelIndex + 2], texelData[texelIndex + 3]));
	}

	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
	stbi_image_free(texelData);
}

Image::Image(IntVec2 size, Rgba8 color) : m_dimensions(size)
{
	int totalSize = size.x * size.y;
	m_rgba8Texels.reserve(totalSize);
	for (int texelIndex = 0; texelIndex < totalSize; texelIndex++) {
		m_rgba8Texels.push_back(color);
	}
}

std::string const& Image::GetImageFilePath() const
{
	return m_imageFilePath;
}

IntVec2 Image::GetDimensions() const
{
	return m_dimensions;
}

const void* Image::GetRawData() const
{
	return m_rgba8Texels.data();
}

Rgba8 Image::GetTexelColor(IntVec2 const& texelCoords) const
{
	return m_rgba8Texels[texelCoords.y * m_dimensions.x + texelCoords.x];
}

Rgba8 Image::GetTexelColor(int texelIndex) const
{
	return m_rgba8Texels[texelIndex];
}

void Image::SetTexelColor(IntVec2 const& texelCoords, Rgba8 const& newColor)
{
	m_rgba8Texels[texelCoords.y * m_dimensions.x + texelCoords.x] = newColor;
}

void Image::SetTexelColor(int texelIndex, Rgba8 const& newColor)
{
	m_rgba8Texels[texelIndex] = newColor;
}
