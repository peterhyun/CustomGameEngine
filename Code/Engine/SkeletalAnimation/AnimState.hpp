#pragma once
#include "Engine/SkeletalAnimation/BVHPose.hpp"
#include "Engine/Core/Clock.hpp"
#include <string>
#include <vector>

struct Quaternion;

class AnimState {
public:
	friend class StateMachineAnimManager;

	void GetLastFramePosAndFwdVectorXY(Vec3& out_lastFramePos, Vec2& out_lastFrameFwdVectorXY) const;
	//const BVHPose& GetCurrentFrame();
	BVHPose GetPoseThisFrame() const;
	void UnpauseClock();
	void PauseClock();
	bool IsClockPaused() const;
	void ResetClock();
	std::string GetName() const;
	bool IsCurrentAnimationDonePlaying() const;
	//bool IsCurrentAnimationAlmostDonePlaying(float percentageOfSequencePlayed) const;
	bool IsCurrentAnimationAlmostDonePlaying(float leftSeconds) const;
	float GetRemainingTime() const;
	bool IsLooping() const;
	//void AppointedAsCurrentAnimState();

	//For processing nextAnimStatePose
	bool IsUpToDate() const;
	void SetIsUpToDate(bool isUpToDate);
	void ProcessOriginalPose(const Vec3& newStartPos, const Quaternion& rotateAroundZ);

private:
	AnimState(const std::string& name, const std::vector<BVHPose>& frameData, bool isLooping, Clock& parentClock, float secondsPerFrame);
	BVHPose LerpPoses(int firstFrameIndex, int secondFrameIndex, float alpha) const;
	void GetNearestFrameIndicesOfElapsedTime(int& out_firstFrameIndex, int& out_secondFrameIndex, float& out_alpha) const;

private:
	std::string m_name;
	std::vector<BVHPose> m_dynamicPose;	//This is for making new frame data from skeleton's updated position from previous animation
	//std::vector<BVHPose> m_forLoopingStateDynamicPose;	//This is for looping frame data (Make sure to move this over to m_dynamicPose if this becomes new frame)

	bool m_isLooping = true;
	bool m_isUpToDate = false; //Variable to check if m_dynamicPose is updated to latest
	const std::vector<BVHPose> m_originalPose;
	Clock m_animStateClock;
	float m_secondsPerFrame;
};