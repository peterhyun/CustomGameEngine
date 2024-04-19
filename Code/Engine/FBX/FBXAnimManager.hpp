#pragma once
#include "Engine/Core/Clock.hpp"
#include "Engine/FBX/FBxPose.hpp"
#include "ThirdParty/fbxsdk/fbxsdk.h"
#include <vector>

class FBXModel;

class FBXAnimManager {
public:
	FBXAnimManager(FBXModel& fbxModel, Clock& parentClock);
	FBXAnimManager(const FBXAnimManager& rhs) = delete;

	std::vector<FBXPose>& GetFBXPoseSequence();
	float GetAnimStartTime() const;
	float GetAnimEndTime() const;
	FbxTime::EMode GetAnimTimeMode() const;

	void ToggleActivation();
	void ToggleAnimPause();
	void SetIsActive(bool isActive);

	void Update();

	unsigned int GetAnimFPS() const;

	bool HasAnimationData() const;

	const FBXPose& GetPoseForThisFrame() const;

	float GetAnimTimeSpan() const;
	void SetAnimDataFromParser(float animStartTime, float animEndTime, FbxTime::EMode animTimeMode, const std::vector<FBXPose>& poseSequence);

	bool IsActive() const;

	FBXAnimManager* CreateCopy() const;
	void SetFBXModel(FBXModel& model);

public:
	Clock m_animClock;

private:
	void GetKeyframeIndexAndLerpAlphaFromElapsedTime(unsigned int& out_keyframeIdx0, float& out_blendAlpha) const;

private:
	bool m_isActivated = false;	//Model should toggle this on/off in FBXModel::ToggleAnimationMode();
	bool m_isMotionMatching = false;	//There's two modes. Either play the animation from bvh, fbx. Or do motion matching

	std::vector<FBXPose> m_poseSequence;

	FBXModel* m_fbxModel = nullptr;
	
	unsigned int m_keyframeIdx0ToPlay = 0;
	float m_alpha = 0.0f;

	FBXPose m_poseForThisFrame;

	FbxTime::EMode m_animTimeMode = FbxTime::EMode::eFrames30;
	float m_animStartTime = 0.0f;
	float m_animEndTime = 0.0f;
};