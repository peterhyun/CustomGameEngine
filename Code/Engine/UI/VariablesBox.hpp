#pragma once
#include "Engine/UI/Widget.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>
#include <string>
//#include <variant>
#include <functional>

class Renderer;
class VBO;
class VertexBuffer;
class BitmapFont;
class Component;

class VariablesBox : public Widget {
	friend class Overlay;
public:
	VariablesBox(Renderer& rendererToUse, Overlay& overlay, const AABB2& screenSpaceAABB2, unsigned int maxVariablesNum, const Rgba8& fontTint, const Rgba8& backgroundTint, const std::string& name);
	void RegisterDragAndDropVariableCallbackFunction(std::function<void(const Vec2& normalizedCursorPos, const std::string& variableName)> dragAndDropVariableCallbackFunction);
	NamedProperties GetNamedProperties() const;
	void Clear();

protected:
	virtual void Update() override;
	virtual void Render() const override;

	/*
	virtual void AddVariable(const std::string& newVarName, bool defaultValue);
	virtual void AddVariable(const std::string& newVarName, int defaultValue);
	*/
	template<typename T>
	void AddVariable(const std::string& newVarName, T defaultValue);

	virtual ~VariablesBox();

	virtual bool OnEnterPressed(const std::string& newVarName);

	void RemakeTextVBO();
	void RemakeNonTextVBO();

protected:
	/*	//Don't need it if I'm using std::variant
	enum class VariableTypeEnum {
		INVALID = -1,
		BOOL,
		INT,
		FLOAT,
		NUM
	};
	*/
	/*
	struct VariableNameTypePair {
		std::string m_variableName;
		std::variant<std::monostate, bool, int> m_defaultValue;
	};
	std::vector<VariableNameTypePair> m_variableNameTypePairs;
	*/
	NamedProperties m_variableKeyValues;

	std::vector<AABB2> m_variableAABBs;
	int m_selectedVariableIdx = -1;	// -1 means not selected
	std::string m_selectedVariableName;

	//Render data
	std::vector<Vertex_PCU> m_textVertices;
	VertexBuffer* m_textVBO = nullptr;
	BitmapFont* m_bitmapFont = nullptr;
	Rgba8 m_textTint;

private:
	unsigned int m_maxVariablesNum = 0;
	//bool m_isAddingNewVariable = false;

	std::vector<Component*> m_components;

	bool m_isBeingDragged = false;

	std::function<void(const Vec2& normalizedCursorPos, const std::string& variableName)> m_dragAndDropVariableCallbackFunction = nullptr;
};

template<typename T>
inline void VariablesBox::AddVariable(const std::string& newVarName, T defaultValue)
{
	if ((unsigned int)m_variableKeyValues.Num() >= m_maxVariablesNum) {
		return;
	}

	m_variableKeyValues.SetValue(newVarName, defaultValue);	
	RemakeTextVBO();
}
