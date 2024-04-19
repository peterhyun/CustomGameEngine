#pragma once
#include <map>
#include "Engine/SkeletalAnimation/AnimState.hpp"
#include "Engine/SkeletalAnimation/BVHPose.hpp"
#include "Engine/Core/Clock.hpp"

class SkeletalCharacter;

//Handles transitions, etc
class StateMachineAnimManager {
public:
	StateMachineAnimManager(SkeletalCharacter& skeletalCharacter, Clock& parentClock, float secondsPerFrame = 0.0333333);
	~StateMachineAnimManager();
	void AddAnimState(const std::string& name, std::vector<BVHPose>& processedData, bool isLooping);
	AnimState* GetCurrentAnimState() const;
	std::string GetCurrentAnimStateName() const;
	void SetDefaultAnimState(const std::string& stateName);
	void UpdateAnimation();

	BVHPose GetPoseThisFrame() const;

private:
	void UpdateNextAnimState();
	void UpdateNextAnimStateFromKeyboardInput();
	void UpdateCurrentAnimState();
	void ProcessNextAnimStatePose();

private:
	SkeletalCharacter& m_skeletalCharacter;
	std::map<std::string, AnimState*> m_animStates;
	AnimState* m_defaultAnimState = nullptr;
	AnimState* m_currentAnimState = nullptr;
	AnimState* m_nextAnimState = nullptr;

	Clock m_animManagerClock;
	float m_secondsPerFrame = 0.0f;
	BVHPose m_frameDataThisFrame;

	const float m_transitionTime = 0.15f;
};