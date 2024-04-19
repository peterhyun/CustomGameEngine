#pragma once
#include "Engine/SkeletalAnimation/FeatureMatrix.hpp"
#include "Engine/SkeletalAnimation/BVHPose.hpp"
#include "Engine/Core/Clock.hpp"

class SkeletalCharacter;

//Handles transitions, etc
class MotionMatchingAnimManager {
public:
	MotionMatchingAnimManager(SkeletalCharacter& skeletalCharacter, Clock& parentClock, float secondsPerFrame = 0.0333333);
	~MotionMatchingAnimManager();
	void UpdateAnimation();
	BVHPose GetPoseThisFrame() const;
	const SkeletalCharacter& GetConstCharacterRef() const;
	FeatureMatrix& GetFeatureMatrixRef();
	const FeatureMatrix& GetFeatureMatrixConstRef() const;

	float GetTimeIntervalForSearchingNewMotion() const;

	//For debug rendering
	const std::vector<BVHPose>& GetProcessedFramesToPlayFrom() const;
	const Feature& GetLatestBestMatchFeature() const;
	const Feature& GetLatestQueryVector() const;
	bool HasUpdatedProcessedFramesThisFrame() const;

	void PauseCurrentClock();
	void UnpauseCurrentClock();

private:
	Feature CreateQueryVector();
	void GetNearestFrameIndicesOfElapsedTime(int& out_firstFrameIndex, int& out_secondFrameIndex, float& out_alpha, const std::vector<BVHPose>& frames, float elapsedTime) const;
	bool IsNewFeatureCloseToAlreadyActiveAnim(const Feature& newFeature);

private:
	SkeletalCharacter& m_skeletalCharacter;

	Clock m_animClipClock1;
	Clock m_animClipClock2;
	Clock* m_currentAnimFramesClock = &m_animClipClock1;
	Clock* m_nextAnimFramesClock = &m_animClipClock2;

	float m_secondsPerFrame = 0.0f;
	float m_invSecondsPerFrame = 0.0f;

	//For lerping this frame and next frame
	bool m_nextProcessedFramesLerpInitiated = false;
	std::vector<BVHPose> m_currentProcessedFrames;
	std::vector<BVHPose> m_nextProcessedFrames;

	BVHPose m_frameDataThisFrame;	//Data to return to the skeletal character

	FeatureMatrix m_featureMatrix;

	int m_latestBestClipIndex = -1;
	int m_latestBestFrameIndex = -1;

	const float m_transitionTime = 0.15f;
	const float m_invTransitionTime = 1.0f / m_transitionTime;

	const float m_timeIntervalForSearchingNewMotion = 0.35f;

	//For debugging
	Feature m_latestBestMatchFeature;
	Feature m_latestQueryVector;
	bool m_hasUpdatedProcessedFramesThisFrame = false;

	Vec3 m_currentLeftFootLocalPos;
	Vec3 m_currentRightFootLocalPos;
	Vec3 m_previousLeftFootLocalPos;
	Vec3 m_previousRightFootLocalPos;
	Vec3 m_leftFootLocalVel;
	Vec3 m_rightFootLocalVel;

	Vec3 m_currentLeftHandLocalPos;
	Vec3 m_currentRightHandLocalPos;
	Vec3 m_previousLeftHandLocalPos;
	Vec3 m_previousRightHandLocalPos;
	Vec3 m_leftHandLocalVel;
	Vec3 m_rightHandLocalVel;
};