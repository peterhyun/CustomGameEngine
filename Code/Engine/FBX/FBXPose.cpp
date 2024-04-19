#include "Engine/FBX/FBXPose.hpp"
#include "Engine/FBX/FBXJoint.hpp"
#include "Engine/FBX/FBxUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

FBXPose::FBXPose(const FBXJoint& skeletonRoot) : m_rootJoint(&skeletonRoot)
{
	GUARANTEE_OR_DIE(skeletonRoot.IsRoot(), "FBXPose constructor received a non-root joint!");

	m_numJoints = GetNumberOfJointsUnderThisJointInclusive(skeletonRoot);
	size_t numJoints = (size_t)m_numJoints;
	m_localLocs.resize(numJoints);
	m_localQuats.resize(numJoints);
	m_localScalings.resize(numJoints);
}

FBXPose FBXPose::LerpPoses(const FBXPose& pose1, const FBXPose& pose2, float alpha)
{
	GUARANTEE_OR_DIE(pose1.m_rootJoint == pose2.m_rootJoint, "pose1.m_rootJoint != pose2.m_rootJoint");
	GUARANTEE_OR_DIE(pose1.m_numJoints == pose2.m_numJoints, "pose1.m_numJoints != pose2.m_numJoints");

	FBXPose lerpedPose;
	lerpedPose.m_rootJoint = pose1.m_rootJoint;
	lerpedPose.m_numJoints = pose1.m_numJoints;
	lerpedPose.m_localScalings.resize((size_t)lerpedPose.m_numJoints);
	lerpedPose.m_localQuats.resize((size_t)lerpedPose.m_numJoints);
	lerpedPose.m_localLocs.resize((size_t)lerpedPose.m_numJoints);

	for (unsigned int jointIdx = 0; jointIdx < lerpedPose.m_numJoints; jointIdx++) {
		lerpedPose.m_localScalings[jointIdx] = Lerp(pose1.m_localScalings[jointIdx], pose2.m_localScalings[jointIdx], alpha);
		lerpedPose.m_localQuats[jointIdx] = Quaternion::Slerp(pose1.m_localQuats[jointIdx], pose2.m_localQuats[jointIdx], alpha);
		lerpedPose.m_localLocs[jointIdx] = Lerp(pose1.m_localLocs[jointIdx], pose2.m_localLocs[jointIdx], alpha);
	}

	return lerpedPose;
}

void FBXPose::SetRootJoint(const FBXJoint& skeletonRoot)
{
	GUARANTEE_OR_DIE(skeletonRoot.IsRoot(), "FBXPose::SetRootJoint() received a non-root joint!");

	m_numJoints = GetNumberOfJointsUnderThisJointInclusive(skeletonRoot);
	size_t numJoints = (size_t)m_numJoints;
	m_localLocs.resize(numJoints);
	m_localQuats.resize(numJoints);
	m_localScalings.resize(numJoints);
	m_rootJoint = &skeletonRoot;
}

void FBXPose::SetJointPoseEntry(unsigned int jointIdx, const Vec4& localScaling, const Quaternion& localQuat, const Vec4& localLoc)
{
	GUARANTEE_OR_DIE(jointIdx < m_numJoints, "JointIdx >= m_numJoints!");
	m_localScalings[jointIdx] = localScaling;
	m_localQuats[jointIdx] = localQuat;
	m_localLocs[jointIdx] = localLoc;
}

void FBXPose::SetJointPoseEntry(unsigned int jointIdx, const FbxVector4& localScaling, const FbxQuaternion& localQuat, const FbxVector4& localLoc)
{
	GUARANTEE_OR_DIE(jointIdx < m_numJoints, "JointIdx >= m_numJoints!");
	m_localScalings[jointIdx] = Vec4((float)localScaling.mData[0], (float)localScaling.mData[1], (float)localScaling.mData[2], (float)localScaling.mData[3]);
	m_localQuats[jointIdx] = Quaternion((float)localQuat.mData[3], (float)localQuat.mData[0], (float)localQuat.mData[1], (float)localQuat.mData[2]);
	m_localLocs[jointIdx] = Vec4((float)localLoc.mData[0], (float)localLoc.mData[1], (float)localLoc.mData[2], (float)localLoc.mData[3]);
}

const FBXJoint* FBXPose::GetRootJoint() const
{
	return m_rootJoint;
}

void FBXPose::CopyFrom(const FBXPose& rhs)
{
	m_localLocs = rhs.m_localLocs;
	m_localQuats = rhs.m_localQuats;
	m_localScalings = rhs.m_localScalings;
	m_numJoints = rhs.m_numJoints;
	m_rootJoint = rhs.m_rootJoint;
}

void FBXPose::FixRootMotionXY()
{
	m_localLocs[0].x = 0.0f;
	m_localLocs[0].y = 0.0f;
}
