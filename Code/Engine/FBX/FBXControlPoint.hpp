#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Mesh/ControlPoint.hpp"
#include <vector>

struct JointWeightPair {
public:
	JointWeightPair(unsigned int jointIndex, float weight): m_jointIndex(jointIndex),  m_weight(weight) {};

public:
	unsigned int m_jointIndex = 0;
	float		 m_weight = 0.0f;
};

struct FBXControlPoint : ControlPoint {
public:
	FBXControlPoint(const Vec3& position) { m_position = position; };

public:
	std::vector<JointWeightPair> m_jointWeightPairs;
};