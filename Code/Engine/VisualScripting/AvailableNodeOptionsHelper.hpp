#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"
#include <string>
#include <vector>

constexpr float HELPER_DIMX = 100.0f;
constexpr float HELPER_DIMY = 250.0f;

class BehaviorTreeEditor;
class Renderer;
class VertexBuffer;
class BehaviorTreeNode;

constexpr unsigned int AVAILABLENODEOPTIONSHELPER_MAXITEMS = 10;

class AvailableNodeOptionsHelper {
public:
	AvailableNodeOptionsHelper(BehaviorTreeEditor& editor, Renderer& renderer);
	~AvailableNodeOptionsHelper();
	void UpdateFromKeyboard();
	void SetPositionToFitEditor(const Vec2& newDesiredPos);

	void UpdateInEditor();
	void Render() const;

	void ConnectNextNewNodeToThisNode(BehaviorTreeNode& thisNode, bool shouldConnectToTopPort);

private:
	void UpdateTextVertsForMe();
	void UpdateOutlineVertsForMe();

private:
	BehaviorTreeEditor& m_editor;
	Renderer& m_renderer;
	Vec2 m_posInEditor;
	Vec2 m_dim;
	int m_selectedOptionIdx = -1;	//-1 means no option is selected

	std::vector<Vertex_PCU> m_outlineVerts;
	VertexBuffer* m_outlineVBO = nullptr;

	std::vector<Vertex_PCU> m_textVerts;
	VertexBuffer* m_textVBO = nullptr;

	BehaviorTreeNode* m_nodeToConnectNewNodeTo = nullptr;
	bool m_shouldConnectToTopPort = false;
};