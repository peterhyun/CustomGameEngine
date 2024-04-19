#include "Engine/SkeletalAnimation/StateMachineAnimManager.hpp"
#include "Engine/SkeletalAnimation/AnimState.hpp"
#include "Engine/SkeletalAnimation/SkeletalCharacter.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"

StateMachineAnimManager::StateMachineAnimManager(SkeletalCharacter& skeletalCharacter, Clock& parentClock, float secondsPerFrame): m_skeletalCharacter(skeletalCharacter), m_animManagerClock(parentClock), m_secondsPerFrame(secondsPerFrame)
{
}

StateMachineAnimManager::~StateMachineAnimManager()
{
	for (const auto& pair : m_animStates) {
		AnimState* animState = pair.second;
		if (animState) {
			delete animState;
			animState = nullptr;
		}
	}
}

void StateMachineAnimManager::AddAnimState(const std::string& name, std::vector<BVHPose>& processedData, bool isLooping)
{
	AnimState* newAnimState = new AnimState(name, processedData, isLooping, m_animManagerClock, m_secondsPerFrame);
	m_animStates.emplace(name, newAnimState);

	//Make one more state by default (in case it loops into itself)
	std::string copyName = name + "Copy";
	AnimState* copyNewAnimState = new AnimState(copyName, processedData, isLooping, m_animManagerClock, m_secondsPerFrame);
	m_animStates.emplace(copyName, copyNewAnimState);
}

AnimState* StateMachineAnimManager::GetCurrentAnimState() const
{
	return m_currentAnimState;
}

std::string StateMachineAnimManager::GetCurrentAnimStateName() const
{
	if (m_currentAnimState == nullptr)
		ERROR_AND_DIE("m_currentAnimState CANNOT be a nullptr");
	return m_currentAnimState->GetName();
}

void StateMachineAnimManager::SetDefaultAnimState(const std::string& stateName)
{
	auto foundPair = m_animStates.find(stateName);
	if (foundPair == m_animStates.end())
		ERROR_AND_DIE(Stringf("AnimState %s is not defined in StateMachineAnimManager", stateName.c_str()).c_str());
	m_defaultAnimState = foundPair->second;
	m_currentAnimState = m_defaultAnimState;
	m_currentAnimState->UnpauseClock();
}

void StateMachineAnimManager::UpdateAnimation()
{
	UpdateNextAnimState();
	UpdateCurrentAnimState();
}

BVHPose StateMachineAnimManager::GetPoseThisFrame() const
{
	return m_frameDataThisFrame;
}

void StateMachineAnimManager::UpdateCurrentAnimState()
{
	bool nextAnimStateLerpStarted = false;
	float nextStateLerpAlpha = 0.0f;

	if (m_nextAnimState != nullptr) {
		//TODO: If currentAnimState is almost done playing, activate the next Anim State clock
		if (m_currentAnimState->IsCurrentAnimationAlmostDonePlaying(m_transitionTime)) {
			if (m_nextAnimState->IsClockPaused()) {
				m_nextAnimState->ResetClock();
				m_nextAnimState->UnpauseClock();
			}
			nextAnimStateLerpStarted = true;
			nextStateLerpAlpha = 1.0f - (m_currentAnimState->GetRemainingTime()) / m_transitionTime;

			if (nextStateLerpAlpha > 1.0f || nextStateLerpAlpha < 0.0f) {
				ERROR_AND_DIE(Stringf("nextStateLerpAlpha is %.2f", nextStateLerpAlpha));
			}


			if (m_currentAnimState->IsCurrentAnimationDonePlaying()) {
				m_currentAnimState->ResetClock();
				m_currentAnimState->PauseClock();
				m_currentAnimState = m_nextAnimState;
				m_nextAnimState = nullptr;
			}
		}
	}

	if (m_currentAnimState->IsLooping() && m_currentAnimState->IsCurrentAnimationDonePlaying()) {	//If current anim state is a looping one...
		m_currentAnimState->ResetClock();
	}

	BVHPose currentAnimStatePose = m_currentAnimState->GetPoseThisFrame();
	if (nextAnimStateLerpStarted && m_nextAnimState) {
		BVHPose nextAnimStatePose = m_nextAnimState->GetPoseThisFrame();
		m_frameDataThisFrame = BVHPose::LerpPoses(currentAnimStatePose, nextAnimStatePose, nextStateLerpAlpha);
		return;
	}
	else {
		m_frameDataThisFrame = currentAnimStatePose;
	}
}

void StateMachineAnimManager::ProcessNextAnimStatePose()
{
	if (m_nextAnimState == nullptr)
		ERROR_AND_DIE("m_nextAnimState is empty yet StateMachineAnimManager::ProcessNextAnimStatePose() called!");
	if (m_currentAnimState == nullptr)
		ERROR_AND_DIE("m_currentAnimState is empty yet StateMachineAnimManager::ProcessNextAnimStatePose() called!");

	Vec3 currentAnimLastFramePos;
	Vec2 currentAnimLastFrameFwdXY;
	m_currentAnimState->GetLastFramePosAndFwdVectorXY(currentAnimLastFramePos, currentAnimLastFrameFwdXY);

	Quaternion rotateAroundZ = Quaternion::CreateFromAxisAndRadians(currentAnimLastFrameFwdXY.GetOrientationRadians(), Vec3(0.0f, 0.0f, 1.0f));

	m_nextAnimState->ProcessOriginalPose(currentAnimLastFramePos, rotateAroundZ);
}

void StateMachineAnimManager::UpdateNextAnimState()
{
	//First check if the current animation is almost ending 
	if (m_nextAnimState == nullptr && m_currentAnimState->IsCurrentAnimationAlmostDonePlaying(m_transitionTime)) {
		if (m_currentAnimState->IsLooping() == false) {
			//Really specific transition rules for non-looping states
			std::string currentAnimStateName = m_currentAnimState->GetName();
			if (currentAnimStateName == "WalkStart") {
				auto foundPair = m_animStates.find("Walk");
				if (foundPair == m_animStates.end()) {
					ERROR_AND_DIE("You should set up AnimState Walk for StateMachineAnimManager");
				}
				m_nextAnimState = foundPair->second;
				m_nextAnimState->SetIsUpToDate(false);
			}
			if (currentAnimStateName == "WalkStop") {
				auto foundPair = m_animStates.find("Idle");
				if (foundPair == m_animStates.end()) {
					ERROR_AND_DIE("You should set up AnimState Idle for StateMachineAnimManager");
				}
				m_nextAnimState = foundPair->second;
				m_nextAnimState->SetIsUpToDate(false);
			}
			if (currentAnimStateName == "TurnLeft45" || currentAnimStateName == "TurnLeft45Copy") {
				auto foundPair = m_animStates.find("Walk");
				if (foundPair == m_animStates.end()) {
					ERROR_AND_DIE("You should set up AnimState Walk for StateMachineAnimManager");
				}
				m_nextAnimState = foundPair->second;
				m_nextAnimState->SetIsUpToDate(false);
			}
			if (currentAnimStateName == "TurnRight45" || currentAnimStateName == "TurnRight45Copy") {
				auto foundPair = m_animStates.find("Walk");
				if (foundPair == m_animStates.end()) {
					ERROR_AND_DIE("You should set up AnimState Walk for StateMachineAnimManager");
				}
				m_nextAnimState = foundPair->second;
				m_nextAnimState->SetIsUpToDate(false);
			}
			if (currentAnimStateName == "SkyUppercut") {
				auto foundPair = m_animStates.find("Walk");
				if (foundPair == m_animStates.end()) {
					ERROR_AND_DIE("You should set up AnimState Walk for StateMachineAnimManager");
				}
				m_nextAnimState = foundPair->second;
				m_nextAnimState->SetIsUpToDate(false);
			}
		}
		else {	//If it is looping
			if (m_nextAnimState == nullptr) {
				//Find the looping counterpart
				std::string currentAnimStateName = m_currentAnimState->GetName();
				if (currentAnimStateName.find("Copy") != std::string::npos) {	//If this is the copy...
					std::string nextAnimStateName = RemoveAllSubstringsIfExists(currentAnimStateName, "Copy");
					auto foundPair = m_animStates.find(nextAnimStateName);
					if (foundPair == m_animStates.end()) {
						ERROR_AND_DIE("You need to set up two anim states in StateMachineAnimManager for an anim state!");
					}
					m_nextAnimState = foundPair->second;
					m_nextAnimState->SetIsUpToDate(false);
				}
				else {	//If this is not the copy
					std::string nextAnimStateName = currentAnimStateName + "Copy";
					auto foundPair = m_animStates.find(nextAnimStateName);
					if (foundPair == m_animStates.end()) {
						ERROR_AND_DIE("You need to set up two anim states in StateMachineAnimManager for an anim state!");
					}
					m_nextAnimState = foundPair->second;
					m_nextAnimState->SetIsUpToDate(false);
				}
			}
		}
	}

	//Keyboard input has biggest priority
	UpdateNextAnimStateFromKeyboardInput();

	//Updating nextAnimState's dynamic frame data
	if (m_nextAnimState && m_nextAnimState->IsUpToDate() == false) {
		ProcessNextAnimStatePose();
		m_nextAnimState->SetIsUpToDate(false);
	}
}

void StateMachineAnimManager::UpdateNextAnimStateFromKeyboardInput()
{
	std::string currentAnimStateName = m_currentAnimState->GetName();
	bool wPressed = g_theInput->IsKeyDown('W');
	bool sPressed = g_theInput->IsKeyDown('S');
	bool aPressed = g_theInput->IsKeyDown('A');
	bool dPressed = g_theInput->IsKeyDown('D');
	bool spacePressed = g_theInput->IsKeyDown(KEYCODE_SPACE);
	if (currentAnimStateName == "Idle") {
		if (wPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "WalkStart") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("WalkStart");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState WalkStart for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
	}
	else if (currentAnimStateName == "Walk" || currentAnimStateName == "WalkCopy") {
		if (sPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "WalkStop") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("WalkStop");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState WalkStop for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
		if (aPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "TurnLeft45") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("TurnLeft45");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState TurnLeft45 for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
		if (dPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "TurnRight45") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("TurnRight45");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState TurnRight45 for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
		if (spacePressed) {
			if (m_nextAnimState && m_nextAnimState->GetName() != "SkyUppercut") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("SkyUppercut");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState SkyUppercut for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
	}
	else if (currentAnimStateName == "TurnLeft45") {
		if (sPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "WalkStop") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("WalkStop");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState WalkStop for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
		if (aPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "TurnLeft45Copy") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("TurnLeft45Copy");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState TurnLeft45Copy for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
		if (dPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "TurnRight45") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("TurnRight45");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState TurnRight45 for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
		if (spacePressed) {
			if (m_nextAnimState && m_nextAnimState->GetName() != "SkyUppercut") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("SkyUppercut");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState SkyUppercut for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
	}
	else if (currentAnimStateName == "TurnRight45") {
		if (sPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "WalkStop") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("WalkStop");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState WalkStop for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
		if (aPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "TurnLeft45") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("TurnLeft45");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState TurnLeft45 for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
		if (dPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "TurnRight45Copy") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("TurnRight45Copy");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState TurnRight45Copy for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
		if (spacePressed) {
			if (m_nextAnimState && m_nextAnimState->GetName() != "SkyUppercut") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("SkyUppercut");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState SkyUppercut for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
	}
	else if (currentAnimStateName == "TurnLeft45Copy") {
		if (sPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "WalkStop") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("WalkStop");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState WalkStop for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
		if (aPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "TurnLeft45") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("TurnLeft45");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState TurnLeft45 for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
		if (dPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "TurnRight45") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("TurnRight45");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState TurnRight45 for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
		if (spacePressed) {
			if (m_nextAnimState && m_nextAnimState->GetName() != "SkyUppercut") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("SkyUppercut");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState SkyUppercut for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
	}
	else if (currentAnimStateName == "TurnRight45Copy") {
		if (sPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "WalkStop") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("WalkStop");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState WalkStop for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
		if (aPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "TurnLeft45") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("TurnLeft45");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState TurnLeft45 for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
		if (dPressed) {

			if (m_nextAnimState && m_nextAnimState->GetName() != "TurnRight45") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("TurnRight45");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState TurnRight45 for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
		if (spacePressed) {
			if (m_nextAnimState && m_nextAnimState->GetName() != "SkyUppercut") {	//If there was a prexisting anim state...
				m_nextAnimState->ResetClock();
				m_nextAnimState->PauseClock();
			}

			auto foundPair = m_animStates.find("SkyUppercut");
			if (foundPair == m_animStates.end()) {
				ERROR_AND_DIE("You should set up AnimState SkyUppercut for StateMachineAnimManager");
			}
			m_nextAnimState = foundPair->second;
			m_nextAnimState->SetIsUpToDate(false);
		}
	}
}
