#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/VisualScripting/NodeHelperComponent.hpp"
#include <vector>
#include <string>
#include <functional>

class BehaviorTreeNode;
class Stopwatch;
class NamedProperties;
class BitmapFont;

typedef NamedProperties EventArgs;

class NodeDropDownComponent : public NodeHelperComponent {
public:
	NodeDropDownComponent(BehaviorTreeNode& ownerNode, const Vec2& nodeUVMins, const Vec2& nodeUVMaxs, const std::vector<std::string>& options, const Vec2& eachOptionAABB2Dimension);
	~NodeDropDownComponent();
	virtual void UpdateInEditor() override;

	void ResetSelectedOptionIdx();

	int GetSelectedOptionIdx() const;
	void SetSelectedOptionIdx(int newIdx);

private:
	virtual void UpdateOutlineVertsForMe() override;
	virtual void UpdateTextVertsForMe() override;

private:
	std::vector<std::string> m_options;

	AABB2 m_fullBox;

	int m_hoveredOptionIdx = -1;
	int m_selectedOptionIdx = -1;

	const Vec2 m_eachOptionAABB2Dim;

	const float m_inv_numOptions = 0.0f;
};