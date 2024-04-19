#pragma once
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Quaternion.hpp"
#include "ThirdParty/fbxsdk/fbxsdk.h"
#include <vector>

class FBXJoint;

class FBXPose {
	friend class FBXJoint;

public:
	FBXPose(const FBXJoint& skeletonRoot);
	FBXPose() {};

	FBXPose(FBXPose&& rhs) = default;
	FBXPose& operator=(FBXPose&& rhs) = default;
	
	FBXPose(const FBXPose& rhs) = delete;
	FBXPose& operator=(const FBXPose& rhs) = delete;

	static FBXPose LerpPoses(const FBXPose& pose1, const FBXPose& pose2, float alpha);

	void SetRootJoint(const FBXJoint& skeletalRoot);

	void SetJointPoseEntry(unsigned int jointIdx, const Vec4& localScaling, const Quaternion& localQuat, const Vec4& localLoc);
	void SetJointPoseEntry(unsigned int jointIdx, const FbxVector4& localScaling, const FbxQuaternion& localQuat, const FbxVector4& localLoc);

	const FBXJoint* GetRootJoint() const;

	void CopyFrom(const FBXPose& rhs);

	void FixRootMotionXY();

private:
	//The skeleton
	const FBXJoint* m_rootJoint = nullptr;

	std::vector<Vec4> m_localScalings;
	std::vector<Quaternion> m_localQuats;
	std::vector<Vec4> m_localLocs;

	unsigned int m_numJoints = 0;	//For error checking when calling AddJointPoseEntry();
};