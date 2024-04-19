#include "Engine/SkeletalAnimation/AnimState.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Quaternion.hpp"

AnimState::AnimState(const std::string& name, const std::vector<BVHPose>& frameData, bool isLooping, Clock& parentClock, float secondsPerFrame): m_name(name), m_originalPose(frameData), m_isLooping(isLooping), m_dynamicPose(frameData), m_animStateClock(parentClock), m_secondsPerFrame(secondsPerFrame)
{
	m_animStateClock.Pause();
}

BVHPose AnimState::GetPoseThisFrame() const
{
	int firstFrameIndex = 0;
	int secondFrameIndex = 0;
	float alpha = 0.0f;
	GetNearestFrameIndicesOfElapsedTime(firstFrameIndex, secondFrameIndex, alpha);
	BVHPose currentAnimStatePose = LerpPoses(firstFrameIndex, secondFrameIndex, alpha);
	return currentAnimStatePose;
}

void AnimState::UnpauseClock()
{
	m_animStateClock.Unpause();
}

void AnimState::PauseClock()
{
	m_animStateClock.Pause();
}

bool AnimState::IsClockPaused() const
{
	return m_animStateClock.IsPaused();
}

void AnimState::ResetClock()
{
	m_animStateClock.Reset();
}

std::string AnimState::GetName() const
{
	return m_name;
}

bool AnimState::IsCurrentAnimationDonePlaying() const
{
	float totalElapsedTime = m_animStateClock.GetTotalSeconds();
	return (totalElapsedTime >= (float)m_originalPose.size() * m_secondsPerFrame);
}

/*
bool AnimState::IsCurrentAnimationAlmostDonePlaying(float percentageOfSequencePlayed) const
{
	if (percentageOfSequencePlayed > 1.0f || percentageOfSequencePlayed < 0.0f) {
		ERROR_AND_DIE(Stringf("percentageOfSequencePlayed: %f", percentageOfSequencePlayed));
	}
	float elapsedSeconds = m_animStateClock.GetTotalSeconds();
	float totalSecondsOfSequence = (float)m_originalPose.size() * m_secondsPerFrame;
	return elapsedSeconds > (percentageOfSequencePlayed) * totalSecondsOfSequence;
}
*/

bool AnimState::IsCurrentAnimationAlmostDonePlaying(float leftSeconds) const
{
	float totalSecondsOfSequence = (float)m_originalPose.size() * m_secondsPerFrame;
	float elapsedSeconds = m_animStateClock.GetTotalSeconds();
	return elapsedSeconds > (totalSecondsOfSequence - leftSeconds);
}

float AnimState::GetRemainingTime() const
{
	float totalSeconds = (float)m_originalPose.size()* m_secondsPerFrame;
	float elapsedTime = m_animStateClock.GetTotalSeconds();

	if (elapsedTime > totalSeconds)
		elapsedTime = totalSeconds;

	return totalSeconds - elapsedTime;
}

bool AnimState::IsLooping() const
{
	return m_isLooping;
}

bool AnimState::IsUpToDate() const
{
	return m_isUpToDate;
}

void AnimState::SetIsUpToDate(bool isUpToDate)
{
	m_isUpToDate = isUpToDate;
}

void AnimState::ProcessOriginalPose(const Vec3& newStartPos, const Quaternion& rotateAroundZ)
{
	std::vector<BVHPose> newProcessedFrames;

	for (int frameIdx = 0; frameIdx < m_originalPose.size(); frameIdx++) {
		const BVHPose& currentOriginalFrame = m_originalPose[frameIdx];
		BVHPose newProcessedFrame = currentOriginalFrame;

		Vec3 rotatedPosition = rotateAroundZ * currentOriginalFrame.m_rootPosGH;
		newProcessedFrame.m_rootPosGH = Vec3(rotatedPosition.x, rotatedPosition.y, currentOriginalFrame.m_rootPosGH.z) + Vec3(newStartPos.x, newStartPos.y, 0.0f);

		//Rotate the root orientation;
		newProcessedFrame.m_jointQuatsGH[0] = (rotateAroundZ * newProcessedFrame.m_jointQuatsGH[0]).GetNormalized();

		newProcessedFrames.push_back(newProcessedFrame);
	}

	m_dynamicPose = newProcessedFrames;
}

BVHPose AnimState::LerpPoses(int firstFrameIndex, int secondFrameIndex, float alpha) const
{
	const std::vector<BVHPose>& currentAnimStateFrames = m_dynamicPose;

	if (firstFrameIndex < 0 || firstFrameIndex >= currentAnimStateFrames.size())
		ERROR_AND_DIE(Stringf("firstFrameIndex: %d passed in when m_frames.size() = %d", firstFrameIndex, currentAnimStateFrames.size()).c_str());
	if (secondFrameIndex < 0 || secondFrameIndex >= currentAnimStateFrames.size())
		ERROR_AND_DIE(Stringf("secondFrameIndex: %d passed in when m_frames.size() = %d", secondFrameIndex, currentAnimStateFrames.size()).c_str());

	const BVHPose& firstFrame = currentAnimStateFrames[firstFrameIndex];
	const BVHPose& secondFrame = currentAnimStateFrames[secondFrameIndex];

	BVHPose lerpedFrame = BVHPose::LerpPoses(firstFrame, secondFrame, alpha);
	return lerpedFrame;
}

void AnimState::GetNearestFrameIndicesOfElapsedTime(int& out_firstFrameIndex, int& out_secondFrameIndex, float& out_alpha) const
{
	float totalElapsedTime = m_animStateClock.GetTotalSeconds();
	int totalNumFrames = (int)m_originalPose.size();
	float inv_secondsPerFrame = 1.0f / m_secondsPerFrame;

	out_firstFrameIndex = GetMin(int(totalElapsedTime * inv_secondsPerFrame), totalNumFrames - 1);
	out_secondFrameIndex = (out_firstFrameIndex < totalNumFrames - 1) ? out_firstFrameIndex + 1 : out_firstFrameIndex;

	out_alpha = GetMin((totalElapsedTime - out_firstFrameIndex * m_secondsPerFrame) * inv_secondsPerFrame, 1.0f);

	if (out_alpha < 0.0f)
		ERROR_AND_DIE("Alpha is " + std::to_string(out_alpha) + ". This should never happen.");
}

void AnimState::GetLastFramePosAndFwdVectorXY(Vec3& out_lastFramePos, Vec2& out_lastFrameFwdVectorXY) const
{
	if (m_dynamicPose.size() == 0)
		ERROR_AND_DIE("m_dynamicPose.size() == 0 yet AnimState::GetLastFramePosAndOri() called!");

	const BVHPose& lastFrame = m_dynamicPose.back();
	out_lastFramePos = lastFrame.m_rootPosGH;
	out_lastFrameFwdVectorXY = lastFrame.GetForwardVectorXY();
}