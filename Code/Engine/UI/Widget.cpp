#include "Engine/UI/Widget.hpp"
#include "Engine/UI/Overlay.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

Widget::Widget(Renderer& rendererToUse, Overlay& overlay, const AABB2& screenSpaceAABB2, const Rgba8& backgroundTint, const std::string& name) : m_renderer(rendererToUse), m_overlay(overlay), m_screenSpaceAABB2(screenSpaceAABB2), m_backgroundTint(backgroundTint), m_name(name)
{
	GUARANTEE_OR_DIE(DoesAABB2FitInOtherAABB2(screenSpaceAABB2, overlay.GetBounds()), Stringf("Widget: %s does not fit in overlay's AABB!", name.c_str()));

	AddVertsForAABB2(m_nonTextVerts, m_screenSpaceAABB2);
	m_nonTextVBO = m_renderer.CreateVertexBuffer(m_nonTextVerts.size(), sizeof(Vertex_PCU), "Widget Quad VBO");
	m_renderer.CopyCPUToGPU(m_nonTextVerts.data(), 6 * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_nonTextVBO);
}

void Widget::SetModelMatrix(const Mat44& modelMatrix)
{
	m_modelMatrix = modelMatrix;
}

void Widget::AddBorders(const Vec2& boxWithinMin, const Vec2& boxWithinMax, const Rgba8& borderColor)
{
	m_borderTint = borderColor;

	AABB2 newBounds = m_screenSpaceAABB2.GetBoxWithin(boxWithinMin, boxWithinMax);
	delete m_nonTextVBO;
	delete m_borderVBO;
	m_nonTextVerts.clear();
	m_borderVerts.clear();

	AddVertsForAABB2(m_borderVerts, m_screenSpaceAABB2);
	m_borderVBO = m_renderer.CreateVertexBuffer(m_borderVerts.size(), sizeof(Vertex_PCU), "Widget Quad Border VBO");
	m_renderer.CopyCPUToGPU(m_borderVerts.data(), 6 * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_borderVBO);

	AddVertsForAABB2(m_nonTextVerts, newBounds);
	m_nonTextVBO = m_renderer.CreateVertexBuffer(m_nonTextVerts.size(), sizeof(Vertex_PCU), "Widget Quad VBO");
	m_renderer.CopyCPUToGPU(m_nonTextVerts.data(), 6 * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_nonTextVBO);
}

const Overlay& Widget::GetConstRefToOverlay() const
{
	return m_overlay;
}

AABB2 Widget::GetBounds() const
{
	return m_screenSpaceAABB2;
}

Widget::~Widget()
{
	delete m_borderVBO;
	delete m_nonTextVBO;
}
