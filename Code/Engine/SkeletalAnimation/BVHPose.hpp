#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Quaternion.hpp"
#include <vector>

class BVHJoint;

struct EulerAngles;

enum class BVHChannel;

class BVHPose {
public:
	//BVHPose util functions
	static BVHPose LerpPoses(const BVHPose& firstPose, const BVHPose& secondPose, float alpha);
	static std::vector<BVHPose> ProcessPoseSequenceToMatchDesiredStartPosAndFwdXY(const std::vector<BVHPose>& snippet, const Vec3& desiredStartPos, const Vec2& desiredStartFwdXY);

	Vec2 GetForwardVectorXY() const;
	void Clear();

public:
	Vec3 m_rootPosGH;
	std::vector<Quaternion> m_jointQuatsGH;
	BVHJoint* m_rig = nullptr;
};