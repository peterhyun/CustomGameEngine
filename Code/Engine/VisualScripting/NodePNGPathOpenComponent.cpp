#include "Engine/VisualScripting/NodePNGPathOpenComponent.hpp"
#include "Engine/VisualScripting/BehaviorTreeNode.hpp"
#include "Engine/VisualScripting/BehaviorTreeEditor.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/StopWatch.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"

NodePNGPathOpenComponent::NodePNGPathOpenComponent(BehaviorTreeNode& ownerNode, const Vec2& nodeUVMins, const Vec2& nodeUVMaxs, float textCellHeight, float textCellAspect, const std::string& pathCutoffSubstring) : NodeHelperComponent(ownerNode, nodeUVMins, nodeUVMaxs), m_textCellHeight(textCellHeight), m_textCellAspect(textCellAspect), m_pathCutoffSubstring(pathCutoffSubstring)
{
}

NodePNGPathOpenComponent::~NodePNGPathOpenComponent()
{
}

void NodePNGPathOpenComponent::UpdateInEditor()
{
	UpdateFromKeyboard();
	UpdateOutlineVertsForMe();
	UpdateTextVertsForMe();
}

void NodePNGPathOpenComponent::SetText(const std::string& text)
{
	m_inputText = text;
}

void NodePNGPathOpenComponent::SetDefaultText(const std::string& defaultText)
{
	m_defaultText = defaultText;
}

bool NodePNGPathOpenComponent::IsPosInside(const Vec2& cursorPosInEditor)
{
	AABB2 screenAABB2 = m_ownerNode.GetAABB2().GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);
	return screenAABB2.IsPointInside(cursorPosInEditor);
}

std::string NodePNGPathOpenComponent::GetText() const
{
	return m_inputText;
}

std::string NodePNGPathOpenComponent::GetPathCutoffSubstring() const
{
	return m_pathCutoffSubstring;
}

void NodePNGPathOpenComponent::SetPathCutoffSubstring(const std::string& pathCutoffSubstring)
{
	m_pathCutoffSubstring = pathCutoffSubstring;
}

void NodePNGPathOpenComponent::UpdateFromKeyboard()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB)) {
		AABB2 screenAABB2 = m_ownerNode.GetAABB2().GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);
		const BehaviorTreeEditor* editor = m_ownerNode.GetConstPtrToEditor();
		GUARANTEE_OR_DIE(editor != nullptr, "m_ownerNode.m_editor == nullptr");
		Vec2 pointOnScreen = editor->GetCursorPosInEditor();
		if (IsPointInsideAABB2D(pointOnScreen, screenAABB2)) {
			std::string fileName;
			Window::GetWindowContext()->GetPNGFileName(fileName, KEYCODE_LMB);
			if (fileName.length() > 0 && DoesFileExistOnDisk(fileName)) {
				//DebuggerPrintf("%s", fileName.c_str());
				size_t pos = fileName.find(m_pathCutoffSubstring);
				size_t offset = std::string(m_pathCutoffSubstring).length();
				if (pos != std::string::npos) { // If the substring is found
					m_inputText = fileName.erase(0, pos + offset); // Erase all characters before the substring (including the substring itself)
					pos = fileName.find(".png");
					if (pos == std::string::npos) {
						pos = fileName.find(".PNG");
					}
					if (pos != std::string::npos) {
						m_inputText = fileName.erase(pos);
					}
				}
				else {
					ERROR_AND_DIE(Stringf("Path doesn't have the substring %s / FullPath: %s", m_pathCutoffSubstring.c_str(), fileName.c_str()));
				}
			}
		}
	}
}

void NodePNGPathOpenComponent::UpdateOutlineVertsForMe()
{
	m_outlineVerts.clear();

	AABB2 ownerNodeAABB = m_ownerNode.GetAABB2();
	AABB2 componentOutline = ownerNodeAABB.GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);
	AddVertsForAABB2(m_outlineVerts, componentOutline, Rgba8::WHITE);

	Renderer& renderer = m_ownerNode.GetConstPtrToEditor()->GetRefToRenderer();
	if (m_outlineVBO == nullptr) {
		m_outlineVBO = renderer.CreateVertexBuffer(m_outlineVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "OutlineVBO for NodePNGPathOpenComponent");
	}
	renderer.CopyCPUToGPU(m_outlineVerts.data(), m_outlineVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_outlineVBO);
}

void NodePNGPathOpenComponent::UpdateTextVertsForMe()
{
	m_textVerts.clear();
	Renderer& renderer = m_ownerNode.GetConstPtrToEditor()->GetRefToRenderer();
	BitmapFont* bitmapFont = renderer.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	GUARANTEE_OR_DIE(bitmapFont != nullptr, "Check BitmapFont file");
	if (m_inputText.size() > 0) {
		AABB2 screenAABB2 = m_ownerNode.GetAABB2().GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);
		bitmapFont->AddVertsForTextInBox2D(m_textVerts, screenAABB2, m_textCellHeight, m_inputText, Rgba8::BLACK, m_textCellAspect, Vec2(0.0f, 0.5f), TextBoxMode::SHRINK_TO_FIT);
	}
	else {
		AABB2 screenAABB2 = m_ownerNode.GetAABB2().GetBoxWithin(m_nodeUVMins, m_nodeUVMaxs);
		bitmapFont->AddVertsForTextInBox2D(m_textVerts, screenAABB2, m_textCellHeight, m_defaultText, Rgba8::PASTEL_BLUE, m_textCellAspect, Vec2(0.0f, 0.5f), TextBoxMode::SHRINK_TO_FIT);
	}

	if (m_textVBO == nullptr) {
		m_textVBO = renderer.CreateVertexBuffer(m_textVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "TextVBO for NodePNGPathOpenComponent");
	}
	renderer.CopyCPUToGPU(m_textVerts.data(), m_textVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_textVBO);
}