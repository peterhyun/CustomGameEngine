#include "Engine/SkeletalAnimation/BVHPose.hpp"
#include "Engine/SkeletalAnimation/BVHParser.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Quaternion.hpp"
#include "Engine/Math/MathUtils.hpp"

BVHPose BVHPose::LerpPoses(const BVHPose& firstPose, const BVHPose& secondPose, float alpha)
{
	int firstFrameQuatsNum = (int)firstPose.m_jointQuatsGH.size();
	int secondFrameQuatsNum = (int)secondPose.m_jointQuatsGH.size();
	if (firstFrameQuatsNum != secondFrameQuatsNum)
		ERROR_AND_DIE(Stringf("LertFrames(): firstPose has numQuats %d while secondPose has numQuats %d", firstFrameQuatsNum, secondFrameQuatsNum));
	
	BVHPose lerpedFrame;
	lerpedFrame.m_rig = firstPose.m_rig;

	lerpedFrame.m_rootPosGH = Lerp(firstPose.m_rootPosGH, secondPose.m_rootPosGH, alpha);

	for (int i = 0; i < firstFrameQuatsNum; i++) {
		Quaternion slerpedQuat = Quaternion::Slerp(firstPose.m_jointQuatsGH[i], secondPose.m_jointQuatsGH[i], alpha);
		lerpedFrame.m_jointQuatsGH.push_back(slerpedQuat);
	}

	return lerpedFrame;
}

std::vector<BVHPose> BVHPose::ProcessPoseSequenceToMatchDesiredStartPosAndFwdXY(const std::vector<BVHPose>& snippet, const Vec3& desiredStartPos, const Vec2& desiredStartFwdXY)
{
	const BVHPose& firstFrame = snippet[0];
	Vec3 firstFrameRootPos = firstFrame.m_rootPosGH;

	Vec2 firstFrameFwdVecXY = firstFrame.GetForwardVectorXY();
	float firstFrameOriRads = firstFrameFwdVecXY.GetOrientationRadians();
	float desiredFrameOriRads = desiredStartFwdXY.GetOrientationRadians();
	float angularDispRads = GetShortestAngularDispRadians(firstFrameOriRads, desiredFrameOriRads);
	Quaternion rotateAroundZ = Quaternion::CreateFromAxisAndRadians(angularDispRads, Vec3(0.0f, 0.0f, 1.0f));

	Vec3 fromFirstFrameRootPosToDesiredStartPos = desiredStartPos - firstFrameRootPos;

	std::vector<BVHPose> processedSnippet;
	for (int frameIdx = 0; frameIdx < snippet.size(); frameIdx++) {
		BVHPose processedFrame = snippet[frameIdx];

		//First rotate the root position
		Vec3 fromFirstFrameRootPosToSnippet = processedFrame.m_rootPosGH - firstFrameRootPos;
		Vec3 rotatedPosition = rotateAroundZ * fromFirstFrameRootPosToSnippet;
		processedFrame.m_rootPosGH = Vec3(rotatedPosition.x, rotatedPosition.y, rotatedPosition.z) + desiredStartPos;

		//Then rotate the root orientation
		processedFrame.m_jointQuatsGH[0] = (rotateAroundZ * processedFrame.m_jointQuatsGH[0]).GetNormalized();

		processedSnippet.push_back(processedFrame);
	}

	return processedSnippet;
}

Vec2 BVHPose::GetForwardVectorXY() const
{
	Vec3 fwdVector = m_jointQuatsGH[0] * Vec3(1.0f, 0.0f, 0.0f);
	Vec2 fwdVectorXY(fwdVector.x, fwdVector.y);
	fwdVectorXY.Normalize();
	return fwdVectorXY;
}

void BVHPose::Clear()
{
	m_rig = nullptr;
	m_jointQuatsGH.clear();
	m_rootPosGH = Vec3();
}