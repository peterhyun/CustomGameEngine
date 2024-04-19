#pragma once
#include "Engine/UI/Widget.hpp"
#include "Engine/Core/Rgba8.hpp"

class Renderer;
class VBO;
class Texture;
struct AABB2;
class Overlay;

class ImageWidget : public Widget {
	friend class Overlay;
public:
	ImageWidget(Renderer& rendererToUse, Overlay& overlay, const AABB2& screenSpaceAABB2, const Texture& image, const Rgba8& tint, const std::string& name);

protected:
	virtual ~ImageWidget();
	virtual void Update() override;
	virtual void Render() const override;

private:
	const Texture* m_image = nullptr;
};