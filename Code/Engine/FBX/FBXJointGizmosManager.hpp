#pragma once
#include "Engine/FBX/FBXJointRotatorGizmo.hpp"
#include "Engine/FBX/FBXJointTranslatorGizmo.hpp"
#include "Engine/Core/EngineCommon.hpp"

class Renderer;
class FBXJoint;
class FBXModel;
class Camera;

enum class FBXJointGizmosManagerState {
	ROTATOR,
	TRANSLATOR,
	NUM
};

class FBXJointGizmosManager {
public:
	FBXJointGizmosManager(FBXModel& model, Renderer& renderer);
	~FBXJointGizmosManager();
	void Startup();
	void Update(const Camera& worldCamera);
	void Render() const;
	FBXJoint* GetSelectedJoint() const;
	void SetSelectedJoint(FBXJoint* selectedJoint);

	bool IsIKSocketMode() const;
	void SetIsShowingLocalOri(bool isShowingLocalOri);
	bool IsShowingLocalOri() const;

private:
	void UpdateFromKeyboard();

private:
	FBXJointGizmosManagerState m_state = FBXJointGizmosManagerState::ROTATOR;
	FBXModel& m_model;
	Renderer& m_renderer;

	FBXJointRotatorGizmo m_rotatorGizmo;
	FBXJointTranslatorGizmo m_translatorGizmo;

	FBXJoint* m_selectedJoint = nullptr;

	const unsigned char m_ikSocketModeToggleKey = KEYCODE_ALT;
	const unsigned char m_gizmoTypeToggleKey = KEYCODE_CONTROL;
	bool m_isIKSocketMode = false;

	bool m_isShowingLocalOri = true;
};