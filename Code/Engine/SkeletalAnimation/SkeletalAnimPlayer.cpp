#include "Engine/SkeletalAnimation/SkeletalAnimPlayer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

SkeletalAnimPlayer::SkeletalAnimPlayer(const SkeletalAnimPlayerConfig& config)
	: m_config(config), m_animPlayerClock(config.m_parentClock)
{
}

SkeletalAnimPlayer::~SkeletalAnimPlayer()
{
	if (m_rig) {
		delete m_rig;
		m_rig = nullptr;
	}
}

void SkeletalAnimPlayer::SetRig(BVHJoint& rig)
{
	//Delete it first
	if (m_rig) {
		delete m_rig;
		m_rig = nullptr;
	}
	m_rig = &rig;
	m_rig->RecursivelySetVertexData(m_config.m_renderer);
}

BVHJoint* SkeletalAnimPlayer::GetRig() const
{
	return m_rig;
}

void SkeletalAnimPlayer::SetDesiredSecondsPerFrame(float desiredSecondsPerFrame)
{
	m_secondsPerFrame = desiredSecondsPerFrame;
}

void SkeletalAnimPlayer::SetFramesAndName(const std::vector<BVHPose>& frames, const std::string& sequenceName)
{
	m_frames = frames;
	/*
	for (int i = 0; i < frames.size(); i++) {
		if (frames[i].m_channelData.size() != m_bvhChannels.size())
			ERROR_AND_DIE("BVHPose's channel size doesn't match the current joint's channel size");
	}
	*/
	m_startFrameIndex = 0;
	m_endFrameIndex = (int)frames.size() - 1;

	m_sequenceName = sequenceName;
}

int SkeletalAnimPlayer::GetNumFrames() const
{
	return (int)m_frames.size();
}

void SkeletalAnimPlayer::SetStartFrameIndex(int startFrameIndex)
{
	if (startFrameIndex < 0 || startFrameIndex >= m_frames.size()) {
		DebuggerPrintf("Cannot set invalid start frame index %d. Current m_frames.size() is %d", startFrameIndex, m_frames.size());
		return;
	}
	m_startFrameIndex = startFrameIndex;
}

void SkeletalAnimPlayer::SetTimeScaleForClock(float timeScale)
{
	m_animPlayerClock.SetTimeScale(timeScale);
}

std::string SkeletalAnimPlayer::GetSequenceName() const
{
	return m_sequenceName;
}

void SkeletalAnimPlayer::PauseClock()
{
	m_animPlayerClock.Pause();
}

void SkeletalAnimPlayer::SetEndFrameIndex(int endFrameIndex)
{
	if (endFrameIndex < m_startFrameIndex) {
		DebuggerPrintf("Cannot set invalid end frame index %d. Current m_startFrameIndex is %d", endFrameIndex, m_startFrameIndex);
		return;
	}
	m_endFrameIndex = endFrameIndex;
}

void SkeletalAnimPlayer::Update()
{
	//TODO: Play logic accordingly to the m_startFrameIndex and m_endFrameIndex


	if (m_animPlayerClock.GetTotalSeconds() >= m_frames.size() * m_secondsPerFrame) {
		m_animPlayerClock.Reset();
	}

	int firstFrameIndex = 0;
	int secondFrameIndex = 0;
	float alpha = 0.0f;
	GetNearestFrameIndicesOfElapsedTime(firstFrameIndex, secondFrameIndex, alpha);

	BVHPose frameData = LerpPoses(firstFrameIndex, secondFrameIndex, alpha);

	m_currentFirstAnimFrameIndex = firstFrameIndex;
	m_currentSecondAnimFrameIndex = secondFrameIndex;
	m_alpha = alpha;

	m_rig->SetPoseIfThisIsRoot(frameData);
}

void SkeletalAnimPlayer::UnpauseClock()
{
	m_animPlayerClock.Unpause();
}

int SkeletalAnimPlayer::GetStartFrameIndex() const
{
	return m_startFrameIndex;
}

int SkeletalAnimPlayer::GetEndFrameIndex() const
{
	return m_endFrameIndex;
}

void SkeletalAnimPlayer::Render() const
{
	m_config.m_renderer.BindShader(nullptr);
	m_rig->RecursivelyRender(m_config.m_renderer, Mat44());
}


void SkeletalAnimPlayer::GetCurrentAnimFrameInterpolationData(int& out_firstAnimFrameIndex, int& out_secondAnimFrameIndex, float& out_alpha) const
{
	out_firstAnimFrameIndex = m_currentFirstAnimFrameIndex;
	out_secondAnimFrameIndex = m_currentSecondAnimFrameIndex;
	out_alpha = m_alpha;
}

void SkeletalAnimPlayer::GetNearestFrameIndicesOfElapsedTime(int& out_firstFrameIndex, int& out_secondFrameIndex, float& out_alpha) const
{
	float totalElapsedTime = m_animPlayerClock.GetTotalSeconds();
	int totalNumberOfFrames = (m_endFrameIndex - m_startFrameIndex) + 1;
	float totalTimeOfFrames = totalNumberOfFrames * m_secondsPerFrame;

	float elapsedTimeWithinRange = (float)fmod(totalElapsedTime, totalTimeOfFrames);

	out_firstFrameIndex = int(elapsedTimeWithinRange / m_secondsPerFrame) + m_startFrameIndex;
	out_secondFrameIndex = (out_firstFrameIndex < m_endFrameIndex) ? out_firstFrameIndex + 1 : out_firstFrameIndex;

	out_alpha = (elapsedTimeWithinRange - (out_firstFrameIndex - m_startFrameIndex) * m_secondsPerFrame) / m_secondsPerFrame;

	if (out_alpha > 1.0f || out_alpha < 0.0f)
		ERROR_AND_DIE("Should never happen. Alpha is " + std::to_string(out_alpha));
}

BVHPose SkeletalAnimPlayer::LerpPoses(int firstFrameIndex, int secondFrameIndex, float alpha) const
{
	if (firstFrameIndex < 0 || firstFrameIndex >= m_frames.size())
		ERROR_AND_DIE(Stringf("firstFrameIndex: %d passed in when m_frames.size() = %d", firstFrameIndex, m_frames.size()).c_str());
	if (secondFrameIndex < 0 || secondFrameIndex >= m_frames.size())
		ERROR_AND_DIE(Stringf("secondFrameIndex: %d passed in when m_frames.size() = %d", secondFrameIndex, m_frames.size()).c_str());
	
	const BVHPose& firstFrame = m_frames[firstFrameIndex];
	const BVHPose& secondFrame = m_frames[secondFrameIndex];

	BVHPose lerpedFrame = BVHPose::LerpPoses(firstFrame, secondFrame, alpha);
	return lerpedFrame;
}
