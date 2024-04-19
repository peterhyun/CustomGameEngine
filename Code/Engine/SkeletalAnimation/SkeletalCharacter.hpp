#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/SkeletalAnimation/StateMachineAnimManager.hpp"
#include "Engine/SkeletalAnimation/MotionMatchingAnimManager.hpp"
#include <vector>

class BVHJoint;
class Renderer;
class InputSystem;
class BVHPose;
struct CubicHermiteCurve2;

struct SkeletalCharacterConfig
{
	SkeletalCharacterConfig(Renderer& rendererForVBOs, InputSystem& inputSystem, Clock& parentClock) : m_renderer(rendererForVBOs), m_inputSystem(inputSystem), m_parentClock(parentClock) {};
	Renderer& m_renderer;
	InputSystem& m_inputSystem;
	Clock& m_parentClock;
};

class SkeletalCharacter {
public:
	SkeletalCharacter(const SkeletalCharacterConfig& config);
	~SkeletalCharacter();
	void SetRig(BVHJoint& rootJoint, const std::string& rigName);

	Vec3 GetRootPos() const;
	Mat44 GetRootOri() const;
	Vec3 GetFwdVector() const;
	void RecursivelyAddJointToAllJointsArray(BVHJoint& jointToAdd);

	std::string GetSkeletonInfoStr() const;
	
	//For motion matching algorithm
	void GetLeftFootAndRightFootLocalPosForFrame(const BVHPose& frameData, Vec3& out_leftFootPos, Vec3& out_rightFootPos) const;
	void GetLeftFootAndRightFootLocalPos(Vec3& out_leftFootPos, Vec3& out_rightFootPos) const;

	void GetLeftHandAndRightHandLocalPosForFrame(const BVHPose& frameData, Vec3& out_leftHandPos, Vec3& out_rightHandPos) const;
	void GetLeftHandAndRightHandLocalPos(Vec3& out_leftHandPos, Vec3& out_rightHandPos) const;

	void Update();
	void Render() const;

	StateMachineAnimManager& GetStateMachineAnimManagerRef();
	MotionMatchingAnimManager& GetMotionMatchingAnimManagerRef();

	const MotionMatchingAnimManager& GetMotionMatchingAnimManagerConstRef() const;

	const CubicHermiteCurve2& GetFutureTrajectory() const;

	void ToggleUsingStateMachine();
	bool IsUsingStateMachine() const;

	void ToggleMotionMatchingDebugRender();
	bool IsMotionMatchingDebugRenderOn() const;

	BVHJoint* GetRootJoint() const;

private:
	void UpdateFromJoystick();
	void UpdateFromKeyboard();
	Vec3 GetLeftFootPos() const;
	Vec3 GetRightFootPos() const;
	Vec3 GetLeftHandPos() const;
	Vec3 GetRightHandPos() const;
	void DebugRenderFeature(const Feature& feature, const Rgba8& leftFootColor, const Rgba8& rightFootColor, const Rgba8& trajectoryColor, float timeToRenderFeature) const;
	void ConvertModelSpacePosAndVecToGlobalSpace(const Vec3& localPos, const Vec3& localVec, Vec3& out_worldPos, Vec3& out_worldVec) const;
	void ConvertGlobalSpacePosAndVecToModelSpace(const Vec3& worldPos, const Vec3& worldVec, Vec3& out_localPos, Vec3& out_localVec) const;

private:
	SkeletalCharacterConfig m_config;
	std::vector<BVHJoint*> m_allJoints;
	BVHJoint* m_rootJoint = nullptr;
	
	std::string m_rigName;

	MotionMatchingAnimManager m_mmAnimManager;
	StateMachineAnimManager m_smAnimManager;
	bool m_isUsingStateMachine = false;

	CubicHermiteCurve2* m_futureTrajectory = nullptr;
	const float m_trajectoryPredictionRadius = 150.0f;

	bool m_isMotionMatchingDebugRenderOn = true;
};