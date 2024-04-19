#include "Engine/FBX/FBXAnimManager.hpp"
#include "Engine/FBX/FBXJoint.hpp"
#include "Engine/FBX/FBXModel.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

FBXAnimManager::FBXAnimManager(FBXModel& fbxModel, Clock& parentClock): m_fbxModel(&fbxModel), m_animClock(parentClock), m_poseForThisFrame(fbxModel.GetRootJoint())
{
	m_animClock.Pause();
}

std::vector<FBXPose>& FBXAnimManager::GetFBXPoseSequence()
{
	return m_poseSequence;
}

float FBXAnimManager::GetAnimStartTime() const
{
	return m_animStartTime;
}

float FBXAnimManager::GetAnimEndTime() const
{
	return m_animEndTime;
}

FbxTime::EMode FBXAnimManager::GetAnimTimeMode() const
{
	return m_animTimeMode;
}

void FBXAnimManager::ToggleActivation()
{
	m_isActivated = !m_isActivated;
	if (m_isActivated) {
		m_animClock.Reset();
		if (m_animClock.IsPaused())
			m_animClock.Unpause();
	}
	else {
		m_animClock.Pause();
	}
}

void FBXAnimManager::ToggleAnimPause()
{
	m_animClock.TogglePause();
}

void FBXAnimManager::SetIsActive(bool isActive)
{
	m_isActivated = isActive;
	if (m_isActivated) {
		m_animClock.Reset();
		if (m_animClock.IsPaused())
			m_animClock.Unpause();
	}
	else {
		m_animClock.Pause();
	}
}

void FBXAnimManager::Update()
{
	//m_keyframeIdx0ToPlay = GetKeyframeIndexToPlayBasedOnElapsedTime();
	float lerpAlpha = 0.0f;
	GetKeyframeIndexAndLerpAlphaFromElapsedTime(m_keyframeIdx0ToPlay, lerpAlpha);

	unsigned int numPoseSequence = (unsigned int)m_poseSequence.size();
	GUARANTEE_OR_DIE(numPoseSequence > 0, "m_poseSequence.size() == 0!");

	if (m_keyframeIdx0ToPlay >= numPoseSequence - 1 || m_animClock.GetTotalSeconds() > m_fbxModel->GetAnimTimeSpan()) {
		m_animClock.Reset();
		m_keyframeIdx0ToPlay = 0;
		lerpAlpha = 0.0f;
	}

	m_poseForThisFrame.CopyFrom(FBXPose::LerpPoses(m_poseSequence[m_keyframeIdx0ToPlay], m_poseSequence[m_keyframeIdx0ToPlay + 1], lerpAlpha));

	if (m_fbxModel->IsRootMotionXYFixed()) {
		m_poseForThisFrame.FixRootMotionXY();
	}
}

unsigned int FBXAnimManager::GetAnimFPS() const
{
	switch (m_animTimeMode) {
	case FbxTime::EMode::eFrames24:
		return 24;
	case FbxTime::EMode::eFrames30:
		return 30;
	case FbxTime::EMode::eFrames60:
		return 60;
	default:
		ERROR_AND_DIE("Only 24fps, 30fps, 60fps supported so far!");
	}
}

bool FBXAnimManager::HasAnimationData() const
{
	return m_poseSequence.size() != 0;
}

const FBXPose& FBXAnimManager::GetPoseForThisFrame() const
{
	return m_poseForThisFrame;
}

float FBXAnimManager::GetAnimTimeSpan() const
{
	return m_animEndTime - m_animStartTime;
}

void FBXAnimManager::SetAnimDataFromParser(float animStartTime, float animEndTime, FbxTime::EMode animTimeMode, const std::vector<FBXPose>& poseSequence)
{
	m_animStartTime = animStartTime;
	m_animEndTime = animEndTime;
	m_animTimeMode = animTimeMode;

	m_poseSequence.resize(poseSequence.size());
	for (int i = 0; i < m_poseSequence.size(); i++) {
		m_poseSequence[i].CopyFrom(poseSequence[i]);
	}
}

bool FBXAnimManager::IsActive() const
{
	return m_isActivated;
}

FBXAnimManager* FBXAnimManager::CreateCopy() const
{
	FBXAnimManager* copy = new FBXAnimManager(*m_fbxModel, *m_animClock.GetParent());
	copy->m_isActivated = m_isActivated;
	copy->m_isMotionMatching = m_isMotionMatching;
	copy->m_poseSequence.resize(m_poseSequence.size());

	for (int poseIdx = 0; poseIdx < m_poseSequence.size(); poseIdx++) {
		copy->m_poseSequence[poseIdx].CopyFrom(m_poseSequence[poseIdx]);
	}

	copy->m_keyframeIdx0ToPlay = m_keyframeIdx0ToPlay;
	copy->m_alpha = m_alpha;

	copy->m_poseForThisFrame.CopyFrom(m_poseForThisFrame);
	copy->m_animTimeMode = m_animTimeMode;
	copy->m_animStartTime = m_animStartTime;
	copy->m_animEndTime = m_animEndTime;

	if (m_animClock.IsPaused() == false) {
		copy->m_animClock.Unpause();
	}
	copy->m_animClock.SetTotalSeconds(m_animClock.GetTotalSeconds());

	return copy;
}

void FBXAnimManager::SetFBXModel(FBXModel& model)
{
	m_fbxModel = &model;
}

void FBXAnimManager::GetKeyframeIndexAndLerpAlphaFromElapsedTime(unsigned int& out_keyframeIdx0, float& out_blendAlpha) const
{
	float elapsedTime = m_animClock.GetTotalSeconds();
	float elapsedTimeTimesFPS = 0.0f;
	switch (m_animTimeMode) {
	case FbxTime::EMode::eFrames24:
		elapsedTimeTimesFPS = elapsedTime * 24.0f;
		break;
	case FbxTime::EMode::eFrames30:
		elapsedTimeTimesFPS = elapsedTime * 30.0f;
		break;
	case FbxTime::EMode::eFrames60:
		elapsedTimeTimesFPS = elapsedTime * 60.0f;
		break;
	default:
		ERROR_AND_DIE("Only 24fps, 30fps, 60fps supported so far!");
	}
	out_keyframeIdx0 = int(elapsedTimeTimesFPS);
	//out_keyframeIdx1 = out_keyframeIdx0 + 1;
	out_blendAlpha = (elapsedTimeTimesFPS - (float)out_keyframeIdx0);
	GUARANTEE_OR_DIE(out_blendAlpha >= 0.0f && out_blendAlpha <= 1.0f, "Check math logic again");
}
