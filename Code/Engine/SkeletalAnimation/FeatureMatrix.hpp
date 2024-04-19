#pragma once
#include "Engine/SkeletalAnimation/BVHPose.hpp"
#include "Engine/SkeletalAnimation/Feature.hpp"
#include <vector>
#include <string>

class MotionMatchingAnimManager;

class FeatureMatrix {
public:
	FeatureMatrix(MotionMatchingAnimManager& mmAnimManager);
	void LoadClip(const std::vector<BVHPose>& clip, const std::string& clipName);
	void ExtractFeaturesFromLoadedClips();
	std::vector<BVHPose> GetFramesOfClip(int clipIndex, int startFrameIndex, int endFrameIndex) const;

	const std::vector<std::string>& GetClipNamesConstRef() const;

	int GetNumFutureKeysForTrajectory() const;
	float GetFutureTimeForTrajectory() const;

	Feature GetBestMatchForQueryVector(const Feature& queryVector);

	float GetCurrentLowestCost() const;

	void SetWeightParameters(float trajectoryRootBonePosDifferenceWeight, float trajectoryRootDirDiffWeight, float footVelocityDiffWeight, float footPosDiffWeight, bool isUsingHandEndEffector, float handVelocityDiffWeight = 1.0f, float handPosDiffWeight = 1.0f);

private:
	Feature ExtractFeatureFromFrame(unsigned int clipIndex, unsigned int frameIndex) const;
	float GetCostBetweenQueryVectorAndFeature(const Feature& queryVector, const Feature& featureToCompare) const;

private:
	MotionMatchingAnimManager& m_mmAnimManager;
	std::vector<Feature> m_features;
	std::vector<std::vector<BVHPose>> m_clips;
	std::vector<std::string> m_clipNames;

	float m_currentLowestCost = 0.0f;

	const int m_futureFrameNumsForTrajectory = 6;
	const float m_futureTimeToTrackTrajectory = 1.5f;
	const float m_timeBetweenTrajectoryKeys = (m_futureTimeToTrackTrajectory / (float)m_futureFrameNumsForTrajectory);
	const float m_timeBetweenFrames = 0.0333333f;
	const float m_inv_timeBetweenFrames = 1.0f / m_timeBetweenFrames;

	//For weight parameters you can adjust later
	float m_trajectoryRootPosDifferenceWeight = 1.0f;
	float m_trajectoryRootDirDiffWeight = 75.0f;
	float m_footVelocityDiffWeight = 1.0f;
	float m_footPosDiffWeight = 8.0f;
	bool m_isUsingHandEndEffector = false;
	float m_handVelocityDiffWeight = 1.0f;
	float m_handPosDiffWeight = 1.0f;

	//For the future
	bool m_alsoCompareHandLocation = false;

	unsigned int m_latestProcessedClipIdx = 0;
};