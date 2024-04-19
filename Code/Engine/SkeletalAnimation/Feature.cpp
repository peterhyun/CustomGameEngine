#include "Engine/SkeletalAnimation/Feature.hpp"

Feature::Feature()
{
}

Feature::Feature(const std::vector<TrajectoryPoint>& futureTrajectory, const Vec3& leftFootPos, const Vec3& rightFootPos, const Vec3& leftFootVel, const Vec3& rightFootVel, const Vec3& leftHandPos, const Vec3& rightHandPos, const Vec3& leftHandVel, const Vec3& rightHandVel, int clipIndex, int frameIndex)
	: m_futureTrajectory(futureTrajectory), m_leftFootPos(leftFootPos), m_rightFootPos(rightFootPos), m_leftFootVel(leftFootVel), m_rightFootVel(rightFootVel), m_leftHandPos(leftHandPos), m_rightHandPos(rightHandPos), m_leftHandVel(leftHandVel), m_rightHandVel(rightHandVel), m_clipIndex(clipIndex), m_frameIndex(frameIndex)
{
}