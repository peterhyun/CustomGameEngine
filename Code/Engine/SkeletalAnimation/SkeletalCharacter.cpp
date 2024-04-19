#include "Engine/SkeletalAnimation/SkeletalCharacter.hpp"
#include "Engine/SkeletalAnimation/BVHJoint.hpp"
#include "Engine/SkeletalAnimation/BVHPose.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/CubicHermiteCurve2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

SkeletalCharacter::SkeletalCharacter(const SkeletalCharacterConfig& config) : m_config(config), m_smAnimManager(*this, config.m_parentClock), m_mmAnimManager(*this, config.m_parentClock)
{
	m_futureTrajectory = new CubicHermiteCurve2(Vec2(0.0f, 0.0f), Vec2(1.0f, 0.0f), Vec2(1.0f, 0.0f), Vec2(0.0f, 0.0f));
}

SkeletalCharacter::~SkeletalCharacter()
{
	if (m_rootJoint) {
		delete m_rootJoint;
		m_rootJoint = nullptr;
	}

	if (m_futureTrajectory) {
		delete m_futureTrajectory;
		m_futureTrajectory = nullptr;
	}
}

void SkeletalCharacter::SetRig(BVHJoint& rootJoint, const std::string& rigName)
{
	m_rigName = rigName;
	m_rootJoint = &rootJoint;
	m_rootJoint->RecursivelySetVertexData(m_config.m_renderer);

	RecursivelyAddJointToAllJointsArray(rootJoint);
}

Vec3 SkeletalCharacter::GetRootPos() const
{
	return m_rootJoint->GetLocalTransformMatrix().GetTranslation3D();
}

Mat44 SkeletalCharacter::GetRootOri() const
{
	Mat44 rootRotation = m_rootJoint->GetLocalRotationMatrix();
	return rootRotation;
}

Vec3 SkeletalCharacter::GetFwdVector() const
{
	Mat44 rootRotation = m_rootJoint->GetLocalRotationMatrix();
	Vec3 fwdVector(rootRotation.m_values[Mat44::Ix], rootRotation.m_values[Mat44::Iy], rootRotation.m_values[Mat44::Iz]);
	return fwdVector;
}

Vec3 SkeletalCharacter::GetLeftFootPos() const
{
	Mat44 transformMatrix = m_allJoints[0]->GetLocalTransformMatrix();
	transformMatrix.Append(m_allJoints[1]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[2]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[3]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[4]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[5]->GetLocalTransformMatrix());
	return transformMatrix.GetTranslation3D();
}

Vec3 SkeletalCharacter::GetRightFootPos() const
{
	Mat44 transformMatrix = m_allJoints[0]->GetLocalTransformMatrix();
	transformMatrix.Append(m_allJoints[6]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[7]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[8]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[9]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[10]->GetLocalTransformMatrix());
	return transformMatrix.GetTranslation3D();
}

Vec3 SkeletalCharacter::GetLeftHandPos() const
{
	Mat44 transformMatrix = m_allJoints[0]->GetLocalTransformMatrix();
	transformMatrix.Append(m_allJoints[14]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[15]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[16]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[17]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[18]->GetLocalTransformMatrix());
	return transformMatrix.GetTranslation3D();
}

Vec3 SkeletalCharacter::GetRightHandPos() const
{
	Mat44 transformMatrix = m_allJoints[0]->GetLocalTransformMatrix();
	transformMatrix.Append(m_allJoints[19]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[20]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[21]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[22]->GetLocalTransformMatrix());
	transformMatrix.Append(m_allJoints[23]->GetLocalTransformMatrix());
	return transformMatrix.GetTranslation3D();
}

void SkeletalCharacter::DebugRenderFeature(const Feature& feature, const Rgba8& leftFootColor, const Rgba8& rightFootColor, const Rgba8& trajectoryColor, float timeToRenderFeature) const
{
	Vec3 leftFootWorldPos, leftFootWorldVel;
	ConvertModelSpacePosAndVecToGlobalSpace(feature.m_leftFootPos, feature.m_leftFootVel, leftFootWorldPos, leftFootWorldVel);

	Vec3 rightFootWorldPos, rightFootWorldVel;
	ConvertModelSpacePosAndVecToGlobalSpace(feature.m_rightFootPos, feature.m_rightFootVel, rightFootWorldPos, rightFootWorldVel);

	DebugAddWorldWireBox(AABB3(leftFootWorldPos, 1.0f, 1.0f, 1.0f), 2.0f, leftFootColor, leftFootColor, timeToRenderFeature, DebugRenderMode::X_RAY);
	DebugAddWorldWireBox(AABB3(rightFootWorldPos, 1.0f, 1.0f, 1.0f), 2.0f, rightFootColor, rightFootColor, timeToRenderFeature, DebugRenderMode::X_RAY);

	float arrowDebugRenderScale = 2.0f;

	DebugAddWorldArrow(leftFootWorldPos, leftFootWorldPos + leftFootWorldVel * arrowDebugRenderScale, 1.0f, timeToRenderFeature, leftFootColor, leftFootColor, DebugRenderMode::X_RAY);
	DebugAddWorldArrow(rightFootWorldPos, rightFootWorldPos + rightFootWorldVel * arrowDebugRenderScale, 1.0f, timeToRenderFeature, rightFootColor, rightFootColor, DebugRenderMode::X_RAY);

	for (int i = 0; i < feature.m_futureTrajectory.size() ; i++) {
		const TrajectoryPoint& tp = feature.m_futureTrajectory[i];

		Vec3 tpPos = Vec3(tp.m_posXY, 0.0f);
		Vec3 tpFwd = Vec3(tp.m_fwdXY, 0.0f);

		Vec3 globalTPPos, globalTPFwd;
		ConvertModelSpacePosAndVecToGlobalSpace(tpPos, tpFwd, globalTPPos, globalTPFwd);

		float fwdDebugRenderScale = 6.0f;
		DebugAddWorldWireBox(AABB3(globalTPPos, 1.0f, 1.0f, 1.0f), 2.0f, trajectoryColor, trajectoryColor, timeToRenderFeature, DebugRenderMode::X_RAY);
		DebugAddWorldArrow(globalTPPos, globalTPPos + globalTPFwd * fwdDebugRenderScale, 1.0f, timeToRenderFeature, trajectoryColor, trajectoryColor, DebugRenderMode::X_RAY);
	}
}

void SkeletalCharacter::ConvertModelSpacePosAndVecToGlobalSpace(const Vec3& localPos, const Vec3& localVec, Vec3& out_worldPos, Vec3& out_worldVec) const
{
	Vec3 rootWorldPos = GetRootPos();

	Vec2 rootFwdXY = Vec2(GetFwdVector());
	rootFwdXY.Normalize();
	float orientationRads = rootFwdXY.GetOrientationRadians();
	Quaternion rotateAroundZ = Quaternion::CreateFromAxisAndRadians(orientationRads, Vec3(0.0f, 0.0f, 1.0f));

	Vec3 rotatedPos = rotateAroundZ * localPos;
	out_worldPos = Vec3(rotatedPos.x + rootWorldPos.x, rotatedPos.y + rootWorldPos.y, rotatedPos.z + rootWorldPos.z);

	out_worldVec = rotateAroundZ * localVec;
}

void SkeletalCharacter::ConvertGlobalSpacePosAndVecToModelSpace(const Vec3& worldPos, const Vec3& worldVec, Vec3& out_localPos, Vec3& out_localVec) const
{
	Vec3 rootWorldPos = GetRootPos();
	Vec2 rootFwdXY = Vec2(GetFwdVector());
	rootFwdXY.Normalize();
	float orientationRads = rootFwdXY.GetOrientationRadians();
	Quaternion rotateAroundZ = Quaternion::CreateFromAxisAndRadians(-orientationRads, Vec3(0.0f, 0.0f, 1.0f));

	Vec3 fromCharacterToPos = worldPos - rootWorldPos;
	Vec3 rotatedPos = rotateAroundZ * fromCharacterToPos;
	out_localPos = rotatedPos;
	
	Vec3 rotatedVec = rotateAroundZ * worldVec;
	out_localVec = rotatedVec;
}

void SkeletalCharacter::GetLeftFootAndRightFootLocalPosForFrame(const BVHPose& frameData, Vec3& out_leftFootLocalPos, Vec3& out_rightFootLocalPos) const
{
	m_rootJoint->SetPoseIfThisIsRoot(frameData);
	GetLeftFootAndRightFootLocalPos(out_leftFootLocalPos, out_rightFootLocalPos);
}

void SkeletalCharacter::GetLeftFootAndRightFootLocalPos(Vec3& out_leftFootPos, Vec3& out_rightFootPos) const
{
	Vec3 rootWorldPos = GetRootPos();
	Vec3 leftFootWorldPos = GetLeftFootPos();
	Vec3 rightFootWorldPos = GetRightFootPos();

	Vec3 translatedLeftFootPos = leftFootWorldPos - rootWorldPos;
	Vec3 translatedRightFootPos = rightFootWorldPos - rootWorldPos;

	Vec2 rootFwdXY = Vec2(GetFwdVector());
	rootFwdXY.Normalize();
	float orientationRads = rootFwdXY.GetOrientationRadians();
	Quaternion rotateAroundZ = Quaternion::CreateFromAxisAndRadians(-orientationRads, Vec3(0.0f, 0.0f, 1.0f));

	out_leftFootPos = rotateAroundZ * translatedLeftFootPos;
	out_rightFootPos = rotateAroundZ * translatedRightFootPos;
}

void SkeletalCharacter::GetLeftHandAndRightHandLocalPosForFrame(const BVHPose& frameData, Vec3& out_leftHandLocalPos, Vec3& out_rightHandLocalPos) const
{
	m_rootJoint->SetPoseIfThisIsRoot(frameData);
	GetLeftHandAndRightHandLocalPos(out_leftHandLocalPos, out_rightHandLocalPos);
}

void SkeletalCharacter::GetLeftHandAndRightHandLocalPos(Vec3& out_leftHandPos, Vec3& out_rightHandPos) const
{
	Vec3 rootWorldPos = GetRootPos();
	Vec3 leftHandWorldPos = GetLeftHandPos();
	Vec3 rightHandWorldPos = GetRightHandPos();

	Vec3 translatedLeftFootPos = leftHandWorldPos - rootWorldPos;
	Vec3 translatedRightFootPos = rightHandWorldPos - rootWorldPos;

	Vec2 rootFwdXY = Vec2(GetFwdVector());
	rootFwdXY.Normalize();
	float orientationRads = rootFwdXY.GetOrientationRadians();
	Quaternion rotateAroundZ = Quaternion::CreateFromAxisAndRadians(-orientationRads, Vec3(0.0f, 0.0f, 1.0f));

	out_leftHandPos = rotateAroundZ * translatedLeftFootPos;
	out_rightHandPos = rotateAroundZ * translatedRightFootPos;
}

void SkeletalCharacter::Update()
{
	//Do animation calculation...
	BVHPose frameData;
	if (m_isUsingStateMachine) {
		m_smAnimManager.UpdateAnimation();
		frameData = m_smAnimManager.GetPoseThisFrame();
	}
	else {
		UpdateFromJoystick();
		UpdateFromKeyboard();
		m_mmAnimManager.UpdateAnimation();
		frameData = m_mmAnimManager.GetPoseThisFrame();
	}

	//At the end...
	m_rootJoint->SetPoseIfThisIsRoot(frameData);
}

StateMachineAnimManager& SkeletalCharacter::GetStateMachineAnimManagerRef()
{
	return m_smAnimManager;
}

MotionMatchingAnimManager& SkeletalCharacter::GetMotionMatchingAnimManagerRef()
{
	return m_mmAnimManager;
}

const MotionMatchingAnimManager& SkeletalCharacter::GetMotionMatchingAnimManagerConstRef() const
{
	return m_mmAnimManager;
}

const CubicHermiteCurve2& SkeletalCharacter::GetFutureTrajectory() const
{
	return *m_futureTrajectory;
}

void SkeletalCharacter::ToggleUsingStateMachine()
{
	m_isUsingStateMachine = !m_isUsingStateMachine;

	if (m_isUsingStateMachine == false) {
		m_mmAnimManager.UnpauseCurrentClock();
	}
	else {
		m_mmAnimManager.PauseCurrentClock();
	}
}

bool SkeletalCharacter::IsUsingStateMachine() const
{
	return m_isUsingStateMachine;
}

void SkeletalCharacter::ToggleMotionMatchingDebugRender()
{
	m_isMotionMatchingDebugRenderOn = !m_isMotionMatchingDebugRenderOn;
}

bool SkeletalCharacter::IsMotionMatchingDebugRenderOn() const
{
	return m_isMotionMatchingDebugRenderOn;
}

BVHJoint* SkeletalCharacter::GetRootJoint() const
{
	return m_rootJoint;
}

void SkeletalCharacter::UpdateFromJoystick()
{
	XboxController controller = m_config.m_inputSystem.GetController(0);
	AnalogJoystick joystick = controller.GetLeftStick();
	Vec2 joystickPosition = joystick.GetPosition();
	joystickPosition = joystickPosition.GetRotatedMinus90Degrees();
	
	std::vector<Vec2> keyFrames;
	Vec2 rootPosXY = Vec2(GetRootPos());
	keyFrames.push_back(rootPosXY);
	Vec2 fwdVectorXY = Vec2(GetFwdVector()).GetNormalized();

	float joystickMagnitude = joystick.GetMagnitude();
	float tangentStrength = RangeMapClamped(joystickMagnitude, 0.0f, 1.0f, 0.0f, 100.0f);
	m_futureTrajectory->ResetValues(rootPosXY, fwdVectorXY * tangentStrength, (joystickMagnitude == 0.0f ) ? fwdVectorXY * tangentStrength : joystickPosition * tangentStrength, rootPosXY + m_trajectoryPredictionRadius * joystickPosition);
	//m_futureTrajectory->ResetValues(rootPosXY, fwdVectorXY * 100.0f, (joystickPosition.GetLengthSquared() == 0.0f ) ? fwdVectorXY * 100.0f : joystickPosition * 100.0f, rootPosXY + m_trajectoryPredictionRadius * joystickPosition);
}

void SkeletalCharacter::UpdateFromKeyboard()
{
	bool isWPressed = false;
	bool isAPressed = false;
	bool isSPressed = false;
	bool isDPressed = false;
	Vec2 virtualJoystickPos[4];

	if (m_config.m_inputSystem.IsKeyDown('W')) {
		isWPressed = true;
		virtualJoystickPos[0] = Vec2(1.0f, 0.0f);
	}
	if (m_config.m_inputSystem.IsKeyDown('A')) {
		isAPressed = true;
		virtualJoystickPos[1] = Vec2(0.0f, 1.0f);
	}
	if (m_config.m_inputSystem.IsKeyDown('S')) {
		isSPressed = true;
		virtualJoystickPos[2] = Vec2(-1.0f, 0.0f);
	}
	if (m_config.m_inputSystem.IsKeyDown('D')) {
		isDPressed = true;
		virtualJoystickPos[3] = Vec2(0.0f, -1.0f);
	}

	if (isWPressed | isAPressed | isSPressed | isDPressed) {
		Vec2 finalJoystickPos;
		for (int i = 0; i < 4; i++) {
			finalJoystickPos += virtualJoystickPos[i];
		}
		finalJoystickPos.Normalize();

		std::vector<Vec2> keyFrames;
		Vec2 rootPosXY = Vec2(GetRootPos());
		keyFrames.push_back(rootPosXY);
		Vec2 fwdVectorXY = Vec2(GetFwdVector());

		m_futureTrajectory->ResetValues(rootPosXY, fwdVectorXY * 100.0f, finalJoystickPos * 100.0f, rootPosXY + m_trajectoryPredictionRadius * finalJoystickPos);
	}
}

void SkeletalCharacter::RecursivelyAddJointToAllJointsArray(BVHJoint& jointToAdd)
{
	m_allJoints.push_back(&jointToAdd);

	for (int i = 0; i < jointToAdd.GetNumChildJoints(); i++) {
		RecursivelyAddJointToAllJointsArray(*jointToAdd.GetChildJointOfIndex(i));
	}
}

std::string SkeletalCharacter::GetSkeletonInfoStr() const
{
	return m_rigName;
}

void SkeletalCharacter::Render() const
{
	m_config.m_renderer.BindTexture(nullptr);
	m_config.m_renderer.BindShader(nullptr);
	m_rootJoint->RecursivelyRender(m_config.m_renderer, Mat44());
	Vec3 rootPos = GetRootPos();
	Vec2 rootPosXY(rootPos);
	Vec3 fwdVector = GetFwdVector();
	if (m_isUsingStateMachine) {
		DebugAddWorldArrow(Vec3(rootPosXY), Vec3(rootPosXY + Vec2(fwdVector) * 30.0f), 2.5f, 0.0f, Rgba8::RED, Rgba8::RED);
	}
	else {
		std::vector<Vertex_PCU> splineVerts;
		AddVertsForCubicHermiteCurve2D(splineVerts, *m_futureTrajectory, 16, 2.0, Rgba8::YELLOW);
		m_config.m_renderer.SetModelConstants(Mat44::CreateTranslation3D(Vec3(0.0f, 0.0f, 1.0f)));
		m_config.m_renderer.DrawVertexArray((int)splineVerts.size(), splineVerts.data());

		const FeatureMatrix& featureMatrix = m_mmAnimManager.GetFeatureMatrixConstRef();

		Vec2 debugDrawLocationXY;
		const int numDebugSpheresToDraw = featureMatrix.GetNumFutureKeysForTrajectory();

		const float betweenKeysTime = 1.0f / (float)numDebugSpheresToDraw;

		//Draw the trajectory
		for (int i = 1; i <= numDebugSpheresToDraw; i++) {
			debugDrawLocationXY = m_futureTrajectory->EvaluateAtParametric(betweenKeysTime * float(i));
		//	DebugAddWorldWireBox(AABB3(Vec3(debugDrawLocationXY, 0.0f), 1.0f, 1.0f, 1.0f), 2.0f, Rgba8::GREEN, Rgba8::GREEN, 0.0f, DebugRenderMode::X_RAY);

			Vec2 tangent = m_futureTrajectory->EvaluateTangentAtParametric(betweenKeysTime * float(i)).GetNormalized();
			tangent *= 8.0f;
			DebugAddWorldArrow(Vec3(debugDrawLocationXY, 0.1f), Vec3(debugDrawLocationXY, 0.1f) + Vec3(tangent, 0.0f), 1.0f, 0.0f, Rgba8::CYAN, Rgba8::CYAN, DebugRenderMode::X_RAY);
		}

		if (m_isMotionMatchingDebugRenderOn && m_mmAnimManager.HasUpdatedProcessedFramesThisFrame()) {
			float timeIntervalForSearchingNewMotion = m_mmAnimManager.GetTimeIntervalForSearchingNewMotion();

			const Feature& latestQueryVector = m_mmAnimManager.GetLatestQueryVector();
			DebugRenderFeature(latestQueryVector, Rgba8::RED, Rgba8::GREEN, Rgba8::BLUE, timeIntervalForSearchingNewMotion);

			const Feature& latestBestMatchFeature = m_mmAnimManager.GetLatestBestMatchFeature();
			DebugRenderFeature(latestBestMatchFeature, Rgba8::ORANGE, Rgba8::TEAL, Rgba8::PURPLE, timeIntervalForSearchingNewMotion);
		}

		/*
		//Show the fwdXY: RED
		DebugAddWorldArrow(Vec3(rootPosXY), Vec3(rootPosXY + Vec2(fwdVector) * 30.0f), 2.5f, 0.0f, Rgba8::RED, Rgba8::RED);
		*/
	}
}