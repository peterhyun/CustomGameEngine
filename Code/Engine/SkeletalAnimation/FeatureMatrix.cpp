#include "Engine/SkeletalAnimation/FeatureMatrix.hpp"
#include "Engine/SkeletalAnimation/MotionMatchingAnimManager.hpp"
#include "Engine/SkeletalAnimation/SkeletalCharacter.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Quaternion.hpp"

FeatureMatrix::FeatureMatrix(MotionMatchingAnimManager& mmAnimManager): m_mmAnimManager(mmAnimManager)
{
	if (m_timeBetweenTrajectoryKeys < 0.0333333f) {
		ERROR_AND_DIE("m_timeBetweenTrajectoryKeys should be bigger than 0.0333333f");
	}
}

void FeatureMatrix::LoadClip(const std::vector<BVHPose>& clip, const std::string& clipName)
{
	m_clips.push_back(clip);
	m_clipNames.push_back(clipName);
}

void FeatureMatrix::ExtractFeaturesFromLoadedClips()
{
	if (m_clips.size() == 0)
		ERROR_AND_DIE("No frames loaded yet FeatureMatrix::ExtractFeaturesFromLoadedFrames() called!");

	unsigned int totalNumClips = (unsigned int)m_clips.size();
	unsigned int numClipsProcessed = 0;
	for (unsigned int clipIdx = m_latestProcessedClipIdx; clipIdx < totalNumClips; clipIdx++) {
		std::vector<BVHPose>& clip = m_clips[clipIdx];
		unsigned int clipLength = (unsigned int)clip.size();
		for (unsigned int frameIdx = 0; frameIdx < clipLength; frameIdx++) {
			Feature feature = ExtractFeatureFromFrame(clipIdx, frameIdx);
			m_features.push_back(feature);
		}
		numClipsProcessed++;
	}

	m_latestProcessedClipIdx += numClipsProcessed;
}

std::vector<BVHPose> FeatureMatrix::GetFramesOfClip(int clipIndex, int startFrameIndex, int endFrameIndex) const
{
	if (startFrameIndex > endFrameIndex || startFrameIndex >= m_clips[clipIndex].size()) {
		ERROR_AND_DIE("Sth is fucked up in your code");
	}
	if (endFrameIndex < m_clips[clipIndex].size()) {
		std::vector<BVHPose> framesToReturn(m_clips[clipIndex].begin() + startFrameIndex, m_clips[clipIndex].begin() + endFrameIndex);
		return framesToReturn;
	}
	else {
		std::vector<BVHPose> framesToReturn(m_clips[clipIndex].begin() + startFrameIndex, m_clips[clipIndex].end());
		return framesToReturn;
	}
}

const std::vector<std::string>& FeatureMatrix::GetClipNamesConstRef() const
{
	return m_clipNames;
}

int FeatureMatrix::GetNumFutureKeysForTrajectory() const
{
	return m_futureFrameNumsForTrajectory;
}

float FeatureMatrix::GetFutureTimeForTrajectory() const
{
	return m_futureTimeToTrackTrajectory;
}

Feature FeatureMatrix::GetBestMatchForQueryVector(const Feature& queryVector)
{
	//TODO: kd trees for best match broad phase. For now just Brute-Force

	Feature* currentBestMatch = &m_features[0];
	m_currentLowestCost = GetCostBetweenQueryVectorAndFeature(queryVector, *currentBestMatch);
	for (int i = 1; i < m_features.size(); i++) {
		Feature& feature = m_features[i];
		float cost = GetCostBetweenQueryVectorAndFeature(queryVector, feature);
		if (cost < m_currentLowestCost) {
			currentBestMatch = &feature;
			m_currentLowestCost = cost;
		}
	}
	return *currentBestMatch;
}

float FeatureMatrix::GetCurrentLowestCost() const
{
	return m_currentLowestCost;
}

void FeatureMatrix::SetWeightParameters(float rootBonePosDifferenceWeight, float rootDirDiffWeight, float footVelocityDiffWeight, float footPosDiffWeight, bool isUsingHandEndEffector, float handVelocityDiffWeight, float handPosDiffWeight)
{
	m_trajectoryRootPosDifferenceWeight = rootBonePosDifferenceWeight;
	m_trajectoryRootDirDiffWeight = rootDirDiffWeight;
	m_footVelocityDiffWeight = footVelocityDiffWeight;
	m_footPosDiffWeight = footPosDiffWeight;
	m_isUsingHandEndEffector = isUsingHandEndEffector;
	m_handVelocityDiffWeight = handVelocityDiffWeight;
	m_handPosDiffWeight = handPosDiffWeight;
}

Feature FeatureMatrix::ExtractFeatureFromFrame(unsigned int clipIndex, unsigned int frameIndex) const
{
	const std::vector<BVHPose>& currentClip = m_clips[clipIndex];
	unsigned int numFrames = (unsigned int)currentClip.size();
	const BVHPose& currentFrame = currentClip[frameIndex];

	Vec2 fwdVectorXY = currentFrame.GetForwardVectorXY();
	float orientationRadians = fwdVectorXY.GetOrientationRadians();
	Quaternion rotateAroundZ = Quaternion::CreateFromAxisAndRadians(-orientationRadians, Vec3(0.0f, 0.0f, 1.0f));

	//Calculating future trajectory (in local space)
	std::vector<TrajectoryPoint> futureTrajectory;
	const float currentTime = (float)frameIndex * m_timeBetweenFrames;
	Vec2 originalStartPosXY = Vec2(currentFrame.m_rootPosGH);
	for (int i = 1; i <= m_futureFrameNumsForTrajectory; i++) {
		float timeOffset = m_timeBetweenTrajectoryKeys * (float)i;
		float newTime = currentTime + timeOffset;

		unsigned int futureFrameIdx = unsigned int(newTime * m_inv_timeBetweenFrames);
		unsigned int futureFrameIdx_plus1 = futureFrameIdx + 1;

		float lerpAlpha = (newTime - ((float)futureFrameIdx * m_timeBetweenFrames)) * m_inv_timeBetweenFrames;

		if (lerpAlpha < 0.0f || lerpAlpha > 1.0f)
			ERROR_AND_DIE("Calculation is wrong");

		if (futureFrameIdx_plus1 < numFrames) {
			Vec2 firstRootPosXY = Vec2(currentClip[futureFrameIdx].m_rootPosGH);
			Vec2 secondRootPosXY = Vec2(currentClip[futureFrameIdx_plus1].m_rootPosGH);
			Vec2 interpolatedRootPosXY = Lerp(firstRootPosXY, secondRootPosXY, lerpAlpha);

			Vec2 translatedPos = interpolatedRootPosXY - originalStartPosXY;
			Vec3 rotatedPos = rotateAroundZ * Vec3(translatedPos.x, translatedPos.y, 0.0f);

			Vec2 firstRootFwdXY = currentClip[futureFrameIdx].GetForwardVectorXY();
			Vec2 secondRootFwdXY = currentClip[futureFrameIdx_plus1].GetForwardVectorXY();
			Vec2 interpolatedFwdXY = Slerp(firstRootFwdXY, secondRootFwdXY, lerpAlpha);
			Vec3 rotatedFwdXY = rotateAroundZ * Vec3(interpolatedFwdXY.x, interpolatedFwdXY.y, 0.0f);

			futureTrajectory.push_back(TrajectoryPoint(Vec2(rotatedPos.x, rotatedPos.y), Vec2(rotatedFwdXY.x, rotatedFwdXY.y)));
		}
		else if (futureFrameIdx >= numFrames) {	//Edge cases...
			Vec2 translatedPos = Vec2(currentClip.back().m_rootPosGH) - originalStartPosXY;
			Vec3 rotatedPos = rotateAroundZ * Vec3(translatedPos.x, translatedPos.y, 0.0f);

			Vec2 fwdXY = currentClip.back().GetForwardVectorXY();
			Vec3 rotatedFwdXY = rotateAroundZ * Vec3(fwdXY.x, fwdXY.y, 0.0f);

			futureTrajectory.push_back(TrajectoryPoint(Vec2(rotatedPos.x, rotatedPos.y), Vec2(rotatedFwdXY.x, rotatedFwdXY.y)));
		}
		else {	//Straddling at the end
			if (newTime >= m_timeBetweenFrames * (numFrames - 1)) {	//If time I want to sample is completely out of scope
				Vec2 translatedPos = Vec2(currentClip.back().m_rootPosGH) - originalStartPosXY;
				Vec3 rotatedPos = rotateAroundZ * Vec3(translatedPos.x, translatedPos.y, 0.0f);

				Vec2 fwdXY = currentClip.back().GetForwardVectorXY();
				Vec3 rotatedFwdXY = rotateAroundZ * Vec3(fwdXY.x, fwdXY.y, 0.0f);

				futureTrajectory.push_back(TrajectoryPoint(Vec2(rotatedPos.x, rotatedPos.y), Vec2(rotatedFwdXY.x, rotatedFwdXY.y)));
			}
			else {	//If time I want to sample is within scope
				float newLerpAlpha = 1.0f - (((m_timeBetweenFrames * (numFrames - 1)) - newTime) * m_inv_timeBetweenFrames);
				Vec2 firstRootPosXY = Vec2(currentClip[futureFrameIdx].m_rootPosGH);
				Vec2 secondRootPosXY = Vec2(currentClip.back().m_rootPosGH);
				Vec2 interpolatedRootPosXY = Lerp(firstRootPosXY, secondRootPosXY, newLerpAlpha);

				Vec2 translatedPos = interpolatedRootPosXY - originalStartPosXY;
				Vec3 rotatedPos = rotateAroundZ * Vec3(translatedPos.x, translatedPos.y, 0.0f);

				Vec2 firstRootFwdXY = currentClip[futureFrameIdx].GetForwardVectorXY();
				Vec2 secondRootFwdXY = currentClip.back().GetForwardVectorXY();
				Vec2 interpolatedFwdXY = Slerp(firstRootFwdXY, secondRootFwdXY, newLerpAlpha);
				Vec3 rotatedFwdXY = rotateAroundZ * Vec3(interpolatedFwdXY.x, interpolatedFwdXY.y, 0.0f);

				futureTrajectory.push_back(TrajectoryPoint(Vec2(rotatedPos.x, rotatedPos.y), Vec2(rotatedFwdXY.x, rotatedFwdXY.y)));
			}
		}
	}

	//Getting left foot pos and right foot pos (in local space)
	Vec3 leftFootLocalPos, rightFootLocalPos;
	m_mmAnimManager.GetConstCharacterRef().GetLeftFootAndRightFootLocalPosForFrame(currentFrame, leftFootLocalPos, rightFootLocalPos);

	Vec3 leftHandLocalPos, rightHandLocalPos;
	m_mmAnimManager.GetConstCharacterRef().GetLeftHandAndRightHandLocalPosForFrame(currentFrame, leftHandLocalPos, rightHandLocalPos);

	Vec3 leftFootLocalVel, rightFootLocalVel, leftHandLocalVel, rightHandLocalVel;
	//Calculating left foot and right foot local velocity
	if (frameIndex > 0) {
		const BVHPose& previousFrame = currentClip[frameIndex - 1];

		Vec3 leftFootPrevLocalPos, rightFootPrevLocalPos;
		m_mmAnimManager.GetConstCharacterRef().GetLeftFootAndRightFootLocalPosForFrame(previousFrame, leftFootPrevLocalPos, rightFootPrevLocalPos);

		Vec3 leftHandPrevLocalPos, rightHandPrevLocalPos;
		m_mmAnimManager.GetConstCharacterRef().GetLeftHandAndRightHandLocalPosForFrame(previousFrame, leftHandPrevLocalPos, rightHandPrevLocalPos);

		leftFootLocalVel = (leftFootLocalPos - leftFootPrevLocalPos) * m_inv_timeBetweenFrames;
		rightFootLocalVel = (rightFootLocalPos - rightFootPrevLocalPos) * m_inv_timeBetweenFrames;

		leftHandLocalVel = (leftHandLocalPos - leftHandPrevLocalPos) * m_inv_timeBetweenFrames;
		rightHandLocalVel = (rightHandLocalPos - rightHandPrevLocalPos) * m_inv_timeBetweenFrames;
	}

	Feature extractedFeature(futureTrajectory, leftFootLocalPos, rightFootLocalPos, leftFootLocalVel, rightFootLocalVel, leftHandLocalPos, rightHandLocalPos, leftHandLocalVel, rightHandLocalVel, clipIndex, frameIndex);
	return extractedFeature;
}

float FeatureMatrix::GetCostBetweenQueryVectorAndFeature(const Feature& queryVector, const Feature& featureToCompare) const
{
	float totalCost = 0.0f;
	int futureTrajectoryVecSize = (int)queryVector.m_futureTrajectory.size();
	/*
	for (int i = 0; i < futureTrajectoryVecSize; i++) {
		totalCost += (featureToCompare.m_futureTrajectory[i].m_posXY - queryVector.m_futureTrajectory[i].m_posXY).GetLength();// * 8.0f;
		totalCost += (featureToCompare.m_futureTrajectory[i].m_fwdXY - queryVector.m_futureTrajectory[i].m_fwdXY).GetLength() * 75.0f;// *float(i + 1);
	}

	totalCost += (queryVector.m_leftFootPos - featureToCompare.m_leftFootPos).GetLength() * 8.0f;
	totalCost += (queryVector.m_rightFootPos - featureToCompare.m_rightFootPos).GetLength() * 8.0f;

	totalCost += (queryVector.m_leftFootVel - featureToCompare.m_leftFootVel).GetLength();
	totalCost += (queryVector.m_rightFootVel - featureToCompare.m_rightFootVel).GetLength();
	*/

	for (int i = 0; i < futureTrajectoryVecSize; i++) {
		totalCost += (featureToCompare.m_futureTrajectory[i].m_posXY - queryVector.m_futureTrajectory[i].m_posXY).GetLength() * m_trajectoryRootPosDifferenceWeight;
		totalCost += (featureToCompare.m_futureTrajectory[i].m_fwdXY - queryVector.m_futureTrajectory[i].m_fwdXY).GetLength() * m_trajectoryRootDirDiffWeight;
	}

	totalCost += (queryVector.m_leftFootPos - featureToCompare.m_leftFootPos).GetLength() * m_footPosDiffWeight;
	totalCost += (queryVector.m_rightFootPos - featureToCompare.m_rightFootPos).GetLength() * m_footPosDiffWeight;

	totalCost += (queryVector.m_leftFootVel - featureToCompare.m_leftFootVel).GetLength() * m_footVelocityDiffWeight;
	totalCost += (queryVector.m_rightFootVel - featureToCompare.m_rightFootVel).GetLength() * m_footVelocityDiffWeight;

	if (m_isUsingHandEndEffector) {
		//To implement...
		totalCost += (queryVector.m_leftHandPos - featureToCompare.m_leftHandPos).GetLength() * m_handPosDiffWeight;
		totalCost += (queryVector.m_rightHandPos - featureToCompare.m_rightHandPos).GetLength() * m_handPosDiffWeight;

		totalCost += (queryVector.m_leftHandVel - featureToCompare.m_leftHandVel).GetLength() * m_handVelocityDiffWeight;
		totalCost += (queryVector.m_rightHandVel - featureToCompare.m_rightHandVel).GetLength() * m_handVelocityDiffWeight;
	}

	return totalCost;
}