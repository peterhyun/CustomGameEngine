#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/SkeletalAnimation/BVHPose.hpp"
#include <vector>

struct TrajectoryPoint {
public:
	TrajectoryPoint(const Vec2& posXY, const Vec2& fwdXY) : m_posXY(posXY), m_fwdXY(fwdXY) {};
public:
	Vec2 m_posXY;
	Vec2 m_fwdXY;
};

class Feature {
public:
	Feature();
	Feature(const std::vector<TrajectoryPoint>& futureTrajectory, const Vec3& leftFootPos, const Vec3& rightFootPos, const Vec3& leftFootVel, const Vec3& rightFootVel, const Vec3& leftHandPos, const Vec3& rightHandPos, const Vec3& leftHandVel, const Vec3& rightHandVel, int clipIndex = -1, int frameIndex = -1);

public:
	std::vector<TrajectoryPoint> m_futureTrajectory;	//My plan: 7 frames in the future
	Vec3 m_leftFootPos;
	Vec3 m_rightFootPos;
	Vec3 m_leftFootVel;
	Vec3 m_rightFootVel;

	Vec3 m_leftHandPos;
	Vec3 m_rightHandPos;
	Vec3 m_leftHandVel;
	Vec3 m_rightHandVel;

	int m_clipIndex = -1;
	int m_frameIndex = -1;
};