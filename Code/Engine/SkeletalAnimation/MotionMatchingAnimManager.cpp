#include "Engine/SkeletalAnimation/MotionMatchingAnimManager.hpp"
#include "Engine/SkeletalAnimation/AnimState.hpp"
#include "Engine/SkeletalAnimation/SkeletalCharacter.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/CubicHermiteCurve2.hpp"

MotionMatchingAnimManager::MotionMatchingAnimManager(SkeletalCharacter& skeletalCharacter, Clock& parentClock, float secondsPerFrame) : m_skeletalCharacter(skeletalCharacter), m_animClipClock1(parentClock), m_animClipClock2(parentClock), m_currentAnimFramesClock(&m_animClipClock1), m_nextAnimFramesClock(&m_animClipClock2), m_secondsPerFrame(secondsPerFrame), m_invSecondsPerFrame(1.0f / secondsPerFrame), m_featureMatrix(*this)
{
	/*
	m_currentAnimFramesClock->Pause();
	m_nextAnimFramesClock->Pause();
	*/

	if (m_transitionTime > m_timeIntervalForSearchingNewMotion) {
		ERROR_AND_DIE("Transition time should not be longer than time interval for searching new motion!");
	}
}

MotionMatchingAnimManager::~MotionMatchingAnimManager()
{
}

void MotionMatchingAnimManager::UpdateAnimation()
{
	if (m_currentProcessedFrames.size() == 0) {
		//Setup current animation
		m_latestQueryVector = CreateQueryVector();
		m_latestBestMatchFeature = m_featureMatrix.GetBestMatchForQueryVector(m_latestQueryVector);
		m_latestBestClipIndex = m_latestBestMatchFeature.m_clipIndex;
		m_latestBestFrameIndex = m_latestBestMatchFeature.m_frameIndex;

		std::vector<BVHPose> snippet = m_featureMatrix.GetFramesOfClip(m_latestBestClipIndex, m_latestBestFrameIndex, m_latestBestFrameIndex + int(m_featureMatrix.GetFutureTimeForTrajectory() / m_secondsPerFrame));
		Vec2 currentSkeletalFwd = Vec2(m_skeletalCharacter.GetFwdVector()).GetNormalized();
		Vec3 currentSkeletalPos = m_skeletalCharacter.GetRootPos();

		m_currentProcessedFrames = BVHPose::ProcessPoseSequenceToMatchDesiredStartPosAndFwdXY(snippet, Vec3(currentSkeletalPos.x, currentSkeletalPos.y, snippet[0].m_rootPosGH.z), currentSkeletalFwd);
	}

	float totalSeconds = m_currentAnimFramesClock->GetTotalSeconds();
	if (totalSeconds > m_timeIntervalForSearchingNewMotion && m_nextProcessedFramesLerpInitiated == false) {
		//Initiate search		
		m_latestQueryVector = CreateQueryVector();
		m_latestBestMatchFeature = m_featureMatrix.GetBestMatchForQueryVector(m_latestQueryVector);

		//Only switch if it's not close to the default animation, or end of the processed clip
		if (!IsNewFeatureCloseToAlreadyActiveAnim(m_latestBestMatchFeature) || totalSeconds > m_currentProcessedFrames.size() * m_secondsPerFrame) {
			m_latestBestClipIndex = m_latestBestMatchFeature.m_clipIndex;
			m_latestBestFrameIndex = m_latestBestMatchFeature.m_frameIndex;

			//TODO: Get the clip of the future. Then process it
			std::vector<BVHPose> snippet = m_featureMatrix.GetFramesOfClip(m_latestBestClipIndex, m_latestBestFrameIndex, m_latestBestFrameIndex + int(m_featureMatrix.GetFutureTimeForTrajectory() / m_secondsPerFrame));
			Vec2 currentSkeletalFwd = Vec2(m_skeletalCharacter.GetFwdVector());
			currentSkeletalFwd.Normalize();
			Vec3 currentSkeletalPos = m_skeletalCharacter.GetRootPos();

			m_nextProcessedFrames = BVHPose::ProcessPoseSequenceToMatchDesiredStartPosAndFwdXY(snippet, Vec3(currentSkeletalPos.x, currentSkeletalPos.y, snippet[0].m_rootPosGH.z), currentSkeletalFwd);

			m_nextProcessedFramesLerpInitiated = true;

			//DEBUGGING
			m_hasUpdatedProcessedFramesThisFrame = true;
		}
		else {
			//DEBUGGING
			m_hasUpdatedProcessedFramesThisFrame = false;
		}
	}
	else {
		//DEBUGGING
		m_hasUpdatedProcessedFramesThisFrame = false;
	}

	if (m_nextProcessedFramesLerpInitiated) {
		if (m_nextAnimFramesClock->IsPaused()) {
			m_nextAnimFramesClock->Unpause();	//Unpause if the clock is paused
		}
		int firstFrameIndex = 0;
		int secondFrameIndex = 0;
		float alpha = 0.0f;

		float nextAnimFramesClockTotalSeconds = m_nextAnimFramesClock->GetTotalSeconds();
		GetNearestFrameIndicesOfElapsedTime(firstFrameIndex, secondFrameIndex, alpha, m_nextProcessedFrames, nextAnimFramesClockTotalSeconds);
		BVHPose frameFromNextAnimFrames = BVHPose::LerpPoses(m_nextProcessedFrames[firstFrameIndex], m_nextProcessedFrames[secondFrameIndex], alpha);

		float currentAnimFramesClockTotalSeconds = m_currentAnimFramesClock->GetTotalSeconds();
		GetNearestFrameIndicesOfElapsedTime(firstFrameIndex, secondFrameIndex, alpha, m_currentProcessedFrames, currentAnimFramesClockTotalSeconds);
		BVHPose frameFromCurrentAnimFrames = BVHPose::LerpPoses(m_currentProcessedFrames[firstFrameIndex], m_currentProcessedFrames[secondFrameIndex], alpha);

		float currentAndNextBlendAlpha = GetMin((nextAnimFramesClockTotalSeconds * m_invTransitionTime), 1.0f);

		if (currentAndNextBlendAlpha < 0.0f)
		{
			ERROR_AND_DIE("Fucked up code");
		}

		m_frameDataThisFrame = BVHPose::LerpPoses(frameFromCurrentAnimFrames, frameFromNextAnimFrames, currentAndNextBlendAlpha);

		if (currentAndNextBlendAlpha >= 1.0f) {
			Clock& refToOriginalCurrentAnimFramesClock = *m_currentAnimFramesClock;
			m_currentAnimFramesClock->Reset();
			m_currentAnimFramesClock->Pause();
			m_currentAnimFramesClock = m_nextAnimFramesClock;
			m_nextAnimFramesClock = &refToOriginalCurrentAnimFramesClock;
			m_currentProcessedFrames = m_nextProcessedFrames;
			m_nextProcessedFrames.clear();
			m_nextProcessedFramesLerpInitiated = false;
		}
	}
	else {
		int firstFrameIndex = 0;
		int secondFrameIndex = 0;
		float alpha = 0.0f;

		GetNearestFrameIndicesOfElapsedTime(firstFrameIndex, secondFrameIndex, alpha, m_currentProcessedFrames, m_currentAnimFramesClock->GetTotalSeconds());
		m_frameDataThisFrame = BVHPose::LerpPoses(m_currentProcessedFrames[firstFrameIndex], m_currentProcessedFrames[secondFrameIndex], alpha);
	}

	//Calculating left foot and right foot velocity. Also for the arms
	float latestDeltaTime = m_currentAnimFramesClock->GetDeltaSeconds();

	if (m_frameDataThisFrame.m_jointQuatsGH.size() != 0) {
		m_skeletalCharacter.GetLeftFootAndRightFootLocalPosForFrame(m_frameDataThisFrame, m_currentLeftFootLocalPos, m_currentRightFootLocalPos);

		float inv_latestDeltaTime = 1.0f / latestDeltaTime;
		m_leftFootLocalVel = (m_currentLeftFootLocalPos - m_previousLeftFootLocalPos) * inv_latestDeltaTime;
		m_rightFootLocalVel = (m_currentRightFootLocalPos - m_previousRightFootLocalPos) * inv_latestDeltaTime;

		m_skeletalCharacter.GetLeftHandAndRightHandLocalPosForFrame(m_frameDataThisFrame, m_currentLeftHandLocalPos, m_currentRightHandLocalPos);
		m_leftHandLocalVel = (m_currentLeftHandLocalPos - m_previousLeftHandLocalPos) * inv_latestDeltaTime;
		m_rightHandLocalVel = (m_currentRightHandLocalPos - m_previousRightHandLocalPos) * inv_latestDeltaTime;
	}

	m_previousLeftFootLocalPos = m_currentLeftFootLocalPos;
	m_previousRightFootLocalPos = m_currentRightFootLocalPos;

	m_previousLeftHandLocalPos = m_currentLeftHandLocalPos;
	m_previousRightHandLocalPos = m_currentRightHandLocalPos;
}

BVHPose MotionMatchingAnimManager::GetPoseThisFrame() const
{
	return m_frameDataThisFrame;
}

const SkeletalCharacter& MotionMatchingAnimManager::GetConstCharacterRef() const
{
	return m_skeletalCharacter;
}

FeatureMatrix& MotionMatchingAnimManager::GetFeatureMatrixRef()
{
	return m_featureMatrix;
}

const FeatureMatrix& MotionMatchingAnimManager::GetFeatureMatrixConstRef() const
{
	return m_featureMatrix;
}

float MotionMatchingAnimManager::GetTimeIntervalForSearchingNewMotion() const
{
	return m_timeIntervalForSearchingNewMotion;
}

const std::vector<BVHPose>& MotionMatchingAnimManager::GetProcessedFramesToPlayFrom() const
{
	return m_currentProcessedFrames;
}

const Feature& MotionMatchingAnimManager::GetLatestBestMatchFeature() const
{
	return m_latestBestMatchFeature;
}

const Feature& MotionMatchingAnimManager::GetLatestQueryVector() const
{
	return m_latestQueryVector;
}

bool MotionMatchingAnimManager::HasUpdatedProcessedFramesThisFrame() const
{
	return m_hasUpdatedProcessedFramesThisFrame;
}

void MotionMatchingAnimManager::PauseCurrentClock()
{
	m_currentAnimFramesClock->Pause();
}

void MotionMatchingAnimManager::UnpauseCurrentClock()
{
	m_currentAnimFramesClock->Unpause();
}

Feature MotionMatchingAnimManager::CreateQueryVector()
{
	const CubicHermiteCurve2& futureTrajectorySpline = m_skeletalCharacter.GetFutureTrajectory();
	int numFutureKeysForTrajectory = m_featureMatrix.GetNumFutureKeysForTrajectory();

	std::vector<TrajectoryPoint> futureTrajectoryVec;

	Vec2 rootPosXY = Vec2(m_skeletalCharacter.GetRootPos());

	Vec2 rootFwdXY = Vec2(m_skeletalCharacter.GetFwdVector());
	rootFwdXY.Normalize();

	float orientationRadians = rootFwdXY.GetOrientationRadians();
	Quaternion rotateAroundZ = Quaternion::CreateFromAxisAndRadians(-orientationRadians, Vec3(0.0f, 0.0f, 1.0f));

	const float betweenKeysTime = 1.0f / (float)numFutureKeysForTrajectory;
	for (int i = 1; i <= numFutureKeysForTrajectory; i++) {
		Vec2 translatedPos = futureTrajectorySpline.EvaluateAtParametric(betweenKeysTime * float(i)) - rootPosXY;
		Vec3 rotatedPos = rotateAroundZ * Vec3(translatedPos.x, translatedPos.y, 0.0f);

		Vec2 fwdXY = futureTrajectorySpline.EvaluateTangentAtParametric(betweenKeysTime * float(i));
		if(fwdXY.GetLength() == 0.0f)
			fwdXY = Vec2(m_skeletalCharacter.GetFwdVector());

		Vec3 rotatedFwdXY = (rotateAroundZ * Vec3(fwdXY.x, fwdXY.y, 0.0f)).GetNormalized();
		futureTrajectoryVec.push_back(TrajectoryPoint(Vec2(rotatedPos.x, rotatedPos.y), Vec2(rotatedFwdXY.x, rotatedFwdXY.y)));
	}

	Feature queryVector(futureTrajectoryVec, m_currentLeftFootLocalPos, m_currentRightFootLocalPos, m_leftFootLocalVel, m_rightFootLocalVel, m_currentLeftHandLocalPos, m_currentRightHandLocalPos, m_leftHandLocalVel, m_rightHandLocalVel);
	return queryVector;
}

void MotionMatchingAnimManager::GetNearestFrameIndicesOfElapsedTime(int& out_firstFrameIndex, int& out_secondFrameIndex, float& out_alpha, const std::vector<BVHPose>& frames, float elapsedTime) const
{
	if (frames.size() == 0)
		ERROR_AND_DIE("There must be processed frames before calling MMAnimManager::GetNearestFrameIndicesOfElapsedTime()");

	//TODO: Lerping logic
	int framesSize = (int)frames.size();
	
	out_firstFrameIndex = GetMin(int(elapsedTime * m_invSecondsPerFrame), framesSize - 1);
	out_secondFrameIndex = (out_firstFrameIndex < framesSize - 1) ? out_firstFrameIndex + 1 : out_firstFrameIndex;

	out_alpha = GetMin((elapsedTime - (float)out_firstFrameIndex * m_secondsPerFrame) * m_invSecondsPerFrame, 1.0f);

	if (out_alpha < 0.0f)
		ERROR_AND_DIE("Alpha is " + std::to_string(out_alpha) + ". This should never happen.");
}

bool MotionMatchingAnimManager::IsNewFeatureCloseToAlreadyActiveAnim(const Feature& newFeature)
{
	if ((m_latestBestClipIndex == newFeature.m_clipIndex) && GetAbs(m_latestBestFrameIndex - newFeature.m_frameIndex) <= 30)
		return true;
	else
		return false;
}
