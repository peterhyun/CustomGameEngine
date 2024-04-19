#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <string>
#include <vector>
#include <list>
#include <map>

class VertexBuffer;

constexpr unsigned int TREENODE_DIMX = 75;
constexpr unsigned int TREENODE_DIMY = 50;
constexpr float TREENODE_PORTRADIUS = 2.5f;

enum class BehaviorTreeTickReturnType {
	SUCCESS = 0,
	FAILURE,
	RUNNING
};

class BehaviorTreeVariable {	//this is an abstract class! Inherit from it
public:
	BehaviorTreeVariable(const std::string& nodeDisplayStr, bool isRootNode, bool overrideDefaultSettings = false, const Vec2& overridenDim = Vec2());
	virtual ~BehaviorTreeVariable();

	virtual BehaviorTreeTickReturnType Tick() = 0;
	virtual BehaviorTreeVariable* Clone(const Vec2& m_newPosInEditor) const = 0;	//For the prototype

	virtual void AddOutlineVertsForMe(const Vec2& cursorPosInEditor, std::vector<Vertex_PCU>& out_verts, bool isHighlighted = false) const;
	virtual void AddTextVertsForMe(const class BitmapFont& bitmapFont, std::vector<Vertex_PCU>& out_verts) const;
	virtual std::map<std::string, std::string> GetAndSetXMLAttributes();

	//Need it for commands
	void MoveNode(const Vec2& newPos);
	std::string GetNodeDisplayStr() const;
	Vec2 GetCurrentPos() const;
	const std::list<BehaviorTreeVariable*>& GetChildNodes() const;

	void AddNodeToChildren(BehaviorTreeVariable& treeNode);
	BehaviorTreeVariable* RemoveNodeFromChildren(BehaviorTreeVariable& treeNode);

	void GetTopPortDiscData(Vec2& out_portDiscCenter, float& out_discRadius) const;
	void GetBottomPortDiscData(Vec2& out_portDiscCenter, float& out_discRadius) const;

	bool IsRootNode() const;
	bool IsCursorInside(const Vec2& cursorPos) const;

	bool IsDeletable() const;
	bool IsMovable() const;

	bool IsNodeAChild(const BehaviorTreeVariable& childCandidate) const;

	BehaviorTreeVariable* GetParentNode() const;

	IntRange GetChildNumRange() const;
	int GetNumChildNodes() const;

protected:
	std::string m_nodeDisplayStr;
	bool m_isRootNode = false;
	Vec2 m_dimensions;
	Vec2 m_posInEditor;

	std::list<BehaviorTreeVariable*> m_childNodes;
	BehaviorTreeVariable* m_parentNode = nullptr;

	IntRange m_childNumRange;

	bool m_isMovable = true;
	bool m_isDeletable = true;

	std::map<std::string, std::string> m_attributeKeyValues;
};