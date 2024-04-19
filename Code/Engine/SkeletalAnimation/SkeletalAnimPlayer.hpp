#pragma once
#include "Engine/SkeletalAnimation/BVHJoint.hpp"
#include "Engine/SkeletalAnimation/BVHPose.hpp"
#include "Engine/Core/Clock.hpp"
#include <vector>
#include <string>

class SkeletalAnimPlayerConfig {
public:
	SkeletalAnimPlayerConfig(Renderer& rendererForVBOs, Clock& parentClock) : m_renderer(rendererForVBOs), m_parentClock(parentClock) {};
	Renderer& m_renderer;
	Clock& m_parentClock;
};

//Class responsible for lerping the frames and calculating global transforms of each joint recursively
class SkeletalAnimPlayer {
public:
	SkeletalAnimPlayer(const SkeletalAnimPlayerConfig& config);
	~SkeletalAnimPlayer();
	void SetRig(BVHJoint& rig);
	BVHJoint* GetRig() const;
	void SetDesiredSecondsPerFrame(float desiredSecondsPerFrame);
	void SetFramesAndName(const std::vector<BVHPose>& frames, const std::string& sequenceName);

	int  GetNumFrames() const;

	void SetStartFrameIndex(int startFrameIndex);
	void SetEndFrameIndex(int endFrameIndex);
	int  GetStartFrameIndex() const;
	int  GetEndFrameIndex() const;

	std::string GetSequenceName() const;

	void SetTimeScaleForClock(float timeScale);
	void PauseClock();
	void UnpauseClock();
	void Update();
	void Render() const;

	void GetCurrentAnimFrameInterpolationData(int& out_firstAnimFrameIdx, int& out_secondAnimFrameIdx, float& out_alpha) const;

private:
	void GetNearestFrameIndicesOfElapsedTime(int& out_firstFrameIndex, int& out_secondFrameIndex, float& out_alpha) const;
	BVHPose LerpPoses(int firstFrameIndex, int secondFrameIndex, float alpha) const;

private:
	SkeletalAnimPlayerConfig m_config;
	BVHJoint* m_rig = nullptr;
	std::vector<BVHPose> m_frames;
	float m_secondsPerFrame = 0.0f;

	int m_startFrameIndex = 0;
	int m_endFrameIndex = 0;

	int m_currentFirstAnimFrameIndex = 0;
	int m_currentSecondAnimFrameIndex = 0;
	float m_alpha = 0.0f;

	Clock m_animPlayerClock;

	std::string m_sequenceName;
};