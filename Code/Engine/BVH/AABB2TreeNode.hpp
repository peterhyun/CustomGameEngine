#pragma once
#include "Engine/Math/AABB2.hpp"
#include <vector>

template<typename EntityType>
struct AABB2TreeNode {
public:
	virtual ~AABB2TreeNode() {};

public:
	AABB2 m_bounds;
	std::vector<EntityType*> m_entities;
	AABB2TreeNode<EntityType>* m_leftChild = nullptr;
	AABB2TreeNode<EntityType>* m_rightChild = nullptr;
};