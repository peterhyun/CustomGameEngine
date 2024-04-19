#include "Engine/UI/VariablesBox.hpp"
#include "Engine/UI/Overlay.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/UI/TextTypeComponent.hpp"
#include "Engine/UI/DropDownComponent.hpp"

VariablesBox::VariablesBox(Renderer& rendererToUse, Overlay& overlay, const AABB2& screenSpaceAABB2, unsigned int maxVariablesNum, const Rgba8& textTint, const Rgba8& backgroundTint, const std::string& name) : Widget(rendererToUse, overlay, screenSpaceAABB2, backgroundTint, name), m_textTint(textTint), m_maxVariablesNum(maxVariablesNum)
{
	GUARANTEE_OR_DIE(maxVariablesNum > 0, "maxVariablesNum == 0 for VariableBox");
	m_widgetType = WidgetType::VARIABLESBOX;
	m_bitmapFont = rendererToUse.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	m_bitmapFont->AddVertsForTextInBox2D(m_textVertices, screenSpaceAABB2.GetBoxWithin(Vec2(0.0f, 0.95f), Vec2(1.0f, 1.0f)), 25.0f, "Variables", m_textTint, 0.8f, Vec2(0.5f, 0.5f));
	m_textVBO = m_renderer.CreateVertexBuffer(m_textVertices.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "VariablesBox Text VBO");
	m_renderer.CopyCPUToGPU(m_textVertices.data(), m_textVertices.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_textVBO);

	AABB2 variableDisplayAABB2 = screenSpaceAABB2.GetBoxWithin(Vec2(0.0f, 0.05f), Vec2(1.0f, 0.95f));
	float inv_maxVariablesNum = 1.0f / (float)maxVariablesNum;
	for (unsigned int i = 0; i < maxVariablesNum; i++) {
		m_variableAABBs.push_back(AABB2(variableDisplayAABB2.GetBoxWithin(Vec2(0.0f, inv_maxVariablesNum * (float)i), Vec2(1.0f, inv_maxVariablesNum * (float)(i + 1)))));
	}

	std::vector<std::string> options = {"Int", "Bool"};
	AABB2 buttonScreenSpaceAABB = m_screenSpaceAABB2.GetBoxWithin(Vec2(0.85f, 0.0f), Vec2(1.0f, 0.05f));
	Vec2 buttonScreenSpaceDims = buttonScreenSpaceAABB.GetDimensions();
	m_components.push_back(new DropDownComponent(rendererToUse, options, *this, buttonScreenSpaceAABB, false, true, Vec2(buttonScreenSpaceDims.x * 1.5f, buttonScreenSpaceDims.y)));

	AABB2 textTypeBarAABB = m_screenSpaceAABB2.GetBoxWithin(Vec2(0.0f, 0.0f), Vec2(0.85f, 0.05f));
	TextTypeComponent* textTypeComp = new TextTypeComponent(rendererToUse, *this, textTypeBarAABB, 18.0f);
	m_components.push_back(textTypeComp);
	textTypeComp->RegisterEnterKeyCallbackFunction([this](const std::string& inputStr) {return OnEnterPressed(inputStr); });

	RemakeNonTextVBO();
}

void VariablesBox::Update()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB)) {	//Highlight it
		m_selectedVariableName.clear();
		m_selectedVariableIdx = -1;
		Vec2 normalizedCursorPos = g_theInput->GetNormalizedCursorPos();
		Vec2 clickedPosInOverlay = m_overlay.GetBounds().GetPointAtUV(normalizedCursorPos);
		if (m_screenSpaceAABB2.IsPointInside(clickedPosInOverlay)) {
			int aabbIdx = 0;
			for (auto variableKeyVal : m_variableKeyValues.m_keyValuePairs) {
				if (m_variableAABBs[aabbIdx].IsPointInside(clickedPosInOverlay) && aabbIdx < (int)m_variableKeyValues.Num()) {
					m_selectedVariableName = variableKeyVal.first;
					m_selectedVariableIdx = aabbIdx;
					m_isBeingDragged = true;
					break;
				}
				aabbIdx++;
			}
		}
		RemakeNonTextVBO();
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_LMB)) {
		if (m_isBeingDragged) {
			if (m_dragAndDropVariableCallbackFunction) {
				Vec2 normalizedCursorPos = g_theInput->GetNormalizedCursorPos();
				m_dragAndDropVariableCallbackFunction(normalizedCursorPos, m_selectedVariableName);
			}
			m_isBeingDragged = false;
		}
	}

	for (Component* component : m_components) {
		component->Update();
	}
}

void VariablesBox::Render() const
{
	m_renderer.BindTexture(nullptr);
	if (m_borderVBO) {
		m_renderer.SetModelConstants(m_modelMatrix, m_borderTint);
		m_renderer.DrawVertexBuffer(m_borderVBO, 6);
	}

	m_renderer.SetModelConstants();
	m_renderer.DrawVertexBuffer(m_nonTextVBO, (int)m_nonTextVerts.size());

	m_renderer.BindTexture(&m_bitmapFont->GetTexture());
	m_renderer.SetModelConstants();
	m_renderer.DrawVertexBuffer(m_textVBO, (int)m_textVertices.size());

	for (Component* component : m_components) {
		component->Render();
	}
}

VariablesBox::~VariablesBox()
{
	for (int i = 0; i < m_components.size(); i++) {
		delete m_components[i];
	}
	delete m_textVBO;
}

bool VariablesBox::OnEnterPressed(const std::string& newVarName)
{
	if ((unsigned int)m_variableKeyValues.Num() >= m_maxVariablesNum) {
		return false;
	}

	DropDownComponent* dropDown = dynamic_cast<DropDownComponent*>(m_components[0]);
	GUARANTEE_OR_DIE(dropDown != nullptr, "Are you sure DropDownComponent is the first component of VariableBox?");
	std::string selectedOption = dropDown->GetSelectedOptionString();
	if (selectedOption != "INVALID") {
		if (selectedOption == "Int") {
			AddVariable(newVarName, 0);
			dropDown->ResetSelectedOptionIdx();
			return true;
		}
		else if (selectedOption == "Bool") {
			AddVariable(newVarName, false);
			dropDown->ResetSelectedOptionIdx();
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

void VariablesBox::RemakeTextVBO()
{
	m_textVertices.clear();

	m_bitmapFont->AddVertsForTextInBox2D(m_textVertices, m_screenSpaceAABB2.GetBoxWithin(Vec2(0.0f, 0.8f), Vec2(1.0f, 1.0f)), 25.0f, "Variables", m_textTint, 0.8f, Vec2(0.5f, 0.9f));
	unsigned int varIdx = 0;
	for (auto variableKeyVal : m_variableKeyValues.m_keyValuePairs) {
		std::string variableName = variableKeyVal.first;
		if (dynamic_cast<NamedPropertyOfType<int>*>(variableKeyVal.second)) {
			m_bitmapFont->AddVertsForTextInBox2D(m_textVertices, m_variableAABBs[varIdx], 30.0f, variableName + " (" + "int" + ")", Rgba8::WHITE, 0.8f, Vec2(0.0f, 0.5f));

		}
		else if (dynamic_cast<NamedPropertyOfType<bool>*>(variableKeyVal.second)) {
			m_bitmapFont->AddVertsForTextInBox2D(m_textVertices, m_variableAABBs[varIdx], 30.0f, variableName + " (" + "bool" + ")", Rgba8::WHITE, 0.8f, Vec2(0.0f, 0.5f));
		}
		else {
			ERROR_AND_DIE("Only int or bool supported");
		}
		varIdx++;
	}

	delete m_textVBO;
	m_textVBO = m_renderer.CreateVertexBuffer(sizeof(Vertex_PCU) * m_textVertices.size(), sizeof(Vertex_PCU), "TextVBO for VariablesBox");
	m_renderer.CopyCPUToGPU(m_textVertices.data(), sizeof(Vertex_PCU) * m_textVertices.size(), sizeof(Vertex_PCU), m_textVBO);
}

void VariablesBox::RemakeNonTextVBO()
{
	m_nonTextVerts.clear();
	AddVertsForAABB2(m_nonTextVerts, m_screenSpaceAABB2, Rgba8::BLACK);

	if (m_selectedVariableName.length() > 0) {
		AddVertsForAABB2(m_nonTextVerts, m_variableAABBs[m_selectedVariableIdx], Rgba8::PURPLE);
	}

	delete m_nonTextVBO;
	m_nonTextVBO = m_renderer.CreateVertexBuffer(m_nonTextVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "VariablesBox NonTextVBO");
	m_renderer.CopyCPUToGPU(m_nonTextVerts.data(), m_nonTextVerts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_nonTextVBO);
}

NamedProperties VariablesBox::GetNamedProperties() const
{
	return m_variableKeyValues;
}

void VariablesBox::Clear()
{
	m_textVertices.clear();
	delete m_textVBO;
	m_textVBO = nullptr;
	m_selectedVariableIdx = -1;
	m_variableKeyValues.Clear();
	RemakeTextVBO();
}

void VariablesBox::RegisterDragAndDropVariableCallbackFunction(std::function<void(const Vec2& normalizedCursorPos, const std::string& variableName)> dragAndDropVariableCallbackFunction)
{
	m_dragAndDropVariableCallbackFunction = dragAndDropVariableCallbackFunction;
}
