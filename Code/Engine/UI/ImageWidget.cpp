#include "Engine/UI/ImageWidget.hpp"
#include "Engine/Renderer/Renderer.hpp"

ImageWidget::ImageWidget(Renderer& rendererToUse, Overlay& overlay, const AABB2& screenSpaceAABB2, const Texture& image, const Rgba8& tint, const std::string& name) : Widget(rendererToUse, overlay, screenSpaceAABB2, tint, name), m_image(&image)
{
	m_widgetType = WidgetType::IMAGE;
}

void ImageWidget::Update()
{
}

void ImageWidget::Render() const
{
	if (m_borderVBO) {
		m_renderer.BindTexture(nullptr);
		m_renderer.SetModelConstants(m_modelMatrix, m_borderTint);
		m_renderer.DrawVertexBuffer(m_borderVBO, 6);
	}

	m_renderer.BindTexture(m_image);
	m_renderer.SetModelConstants(m_modelMatrix, m_backgroundTint);
	m_renderer.DrawVertexBuffer(m_nonTextVBO, 6);
}

ImageWidget::~ImageWidget()
{
}