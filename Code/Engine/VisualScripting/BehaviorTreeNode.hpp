#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include <string>
#include <vector>
#include <list>
#include <map>

class VertexBuffer;
class BehaviorTreeEditor;
class NodeHelperComponent;
struct AABB2;
class VisualNovelManager;
class VertexBuffer;
class Stopwatch;

constexpr float TREENODE_DIMX = 100.0f;
constexpr float TREENODE_DIMY = 50.0f;
constexpr float TREENODE_PORTRADIUS = 2.5f;

enum class BehaviorTreeTickReturnType {
	SUCCESS = 0,
	FAILURE,
	RUNNING
};

class BehaviorTreeNode {	//this is an abstract class! Inherit from it
public:
	BehaviorTreeNode(const std::string& nodeDisplayStr, bool isRootNode, bool overrideDefaultSettings = false, const Vec2& overridenDim = Vec2());
	virtual ~BehaviorTreeNode();

	virtual BehaviorTreeTickReturnType Tick() = 0;
	virtual BehaviorTreeNode* Clone(const Vec2& m_newPosInEditor) const = 0;	//For the prototype

	virtual std::map<std::string, std::string> GetAndSetXMLAttributes();

	virtual void UpdateInEditor();
	virtual void RenderInEditor() const final;

	virtual bool CheckSetupValidity();

	virtual void FillInMissingInformationFromXmlElement(const XmlElement& element);

	virtual void UpdateDimensionsRequestedFromComponent(const Vec2& dim, const NodeHelperComponent& componentThatRequestedThis);

	//Need it for commands
	void MoveNode(const Vec2& newPos);
	std::string GetNodeDisplayStr() const;
	Vec2 GetCurrentPos() const;
	const std::list<BehaviorTreeNode*>& GetChildNodes() const;

	void AddNodeToChildren(BehaviorTreeNode& treeNode);
	BehaviorTreeNode* RemoveNodeFromChildren(BehaviorTreeNode& treeNode);

	void GetTopPortDiscData(Vec2& out_portDiscCenter, float& out_discRadius) const;
	void GetBottomPortDiscData(Vec2& out_portDiscCenter, float& out_discRadius) const;

	bool IsRootNode() const;
	bool IsCursorInside(const Vec2& cursorPos) const;

	bool IsDeletable() const;
	bool IsMovable() const;

	bool IsNodeAChild(const BehaviorTreeNode& childCandidate) const;

	BehaviorTreeNode* GetParentNode() const;

	IntRange GetChildNumRange() const;
	int GetNumChildNodes() const;

	AABB2 GetAABB2() const;

	void SetBehaviorTreeEditor(const BehaviorTreeEditor& editor);
	const BehaviorTreeEditor* GetConstPtrToEditor() const;

	void ReorderChildNodes();

	Vec2 GetInitialDims() const;
	Vec2 GetPosInEditor() const;
	AABB2 GetAABB2ForInitialDims() const;

	void SetVNManager(VisualNovelManager& manager);

	BehaviorTreeNode* GetLastTickedChild() const;
	virtual void UpdateTickFlowIndicatorVertsForMe();

	virtual void AlertTickStopped();
	virtual void DeleteTickFlowVBO();

protected:
	virtual void UpdateOutlineVertsForMe(const Vec2& cursorPosInEditor, bool isHighlighted = false);
	virtual void UpdateTextVertsForMe();

protected:
	std::vector<NodeHelperComponent*> m_helperComponents;

	std::string m_nodeDisplayStr;
	bool m_isRootNode = false;
	Vec2 m_dimensions;
	Vec2 m_posInEditor;

	Vec2 m_initialTitleOutlineDims;

	std::list<BehaviorTreeNode*> m_childNodes;
	BehaviorTreeNode* m_parentNode = nullptr;
	BehaviorTreeNode* m_lastTickedChild = nullptr;

	IntRange m_childNumRange;

	bool m_isMovable = true;
	bool m_isDeletable = true;

	std::map<std::string, std::string> m_attributeKeyValues;

	const BehaviorTreeEditor* m_editor = nullptr;

	bool m_isSetupValidWhenLatestValidityCheck = true;

	float m_initialTitleOutlineDimsY = 0.0f;
	
	Vec2 m_initialDimensions;

	VisualNovelManager* m_manager = nullptr;

	VertexBuffer* m_outlineVBO = nullptr;
	std::vector<Vertex_PCU> m_outlineVerts;

	VertexBuffer* m_textVBO = nullptr;
	std::vector<Vertex_PCU> m_textVerts;

	VertexBuffer* m_tickFlowVBO = nullptr;
	std::vector<Vertex_PCU> m_tickFlowVerts;
	const float m_totalTimeForFlowCircleToMove = 1.0f;
	Stopwatch* m_tickFlowStopwatch;
	const float m_flowCircleRadius = 5.0f;
};