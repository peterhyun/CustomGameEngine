#include "Engine/UI/TextBox.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"

TextBox::TextBox(Renderer& rendererToUse, Overlay& overlay, const AABB2& screenSpaceAABB2, const std::string& text, float textSize, const Vec2& textAlignment, const Rgba8& textTint, const Rgba8& backgroundTint, const std::string& name) : Widget(rendererToUse, overlay, screenSpaceAABB2, backgroundTint, name), m_textTint(textTint), m_textSize(textSize), m_textAlignment(textAlignment)
{
	m_widgetType = WidgetType::TEXTBOX;
	m_text = text;
	m_bitmapFont = rendererToUse.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	m_bitmapFont->AddVertsForTextInBox2D(m_textVertices, screenSpaceAABB2, textSize, text, Rgba8::WHITE, 1.0f, textAlignment);
	m_textVBO = m_renderer.CreateVertexBuffer(m_textVertices.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "TextBox Text VBO");
	m_renderer.CopyCPUToGPU(m_textVertices.data(), m_textVertices.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_textVBO);
}

void TextBox::Update()
{
}

void TextBox::Render() const
{
	m_renderer.BindTexture(nullptr);
	if (m_borderVBO) {
		m_renderer.SetModelConstants(m_modelMatrix, m_borderTint);
		m_renderer.DrawVertexBuffer(m_borderVBO, 6);
	}

	m_renderer.SetModelConstants(m_modelMatrix, m_backgroundTint);
	m_renderer.DrawVertexBuffer(m_nonTextVBO, 6);

	m_renderer.BindTexture(&m_bitmapFont->GetTexture());
	m_renderer.SetModelConstants(m_modelMatrix, m_textTint);
	m_renderer.DrawVertexBuffer(m_textVBO, (int)m_textVertices.size());
}

void TextBox::SetText(const std::string& newText)
{
	delete m_textVBO;
	m_textVertices.clear();

	m_text = newText;
	m_bitmapFont->AddVertsForTextInBox2D(m_textVertices, m_screenSpaceAABB2, m_textSize, newText, Rgba8::WHITE, 1.0f, m_textAlignment);
	m_textVBO = m_renderer.CreateVertexBuffer(m_textVertices.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "TextBox Text VBO");
	m_renderer.CopyCPUToGPU(m_textVertices.data(), m_textVertices.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_textVBO);
}

TextBox::~TextBox()
{
	delete m_textVBO;
}
