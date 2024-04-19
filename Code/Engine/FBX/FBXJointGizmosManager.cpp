#include "Engine/FBX/FBXJointGizmosManager.hpp"
#include "Engine/FBX/FBXModel.hpp"
#include "Engine/FBX/FBXUtils.hpp"
#include "Engine/FBX/FBXJoint.hpp"
#include "Engine/Renderer/Renderer.hpp"

FBXJointGizmosManager::FBXJointGizmosManager(FBXModel& model, Renderer& renderer)
	:m_rotatorGizmo(*this, model, renderer), m_translatorGizmo(*this, model, renderer), m_model(model), m_renderer(renderer)
{
}

FBXJointGizmosManager::~FBXJointGizmosManager()
{
}

void FBXJointGizmosManager::Startup()
{
	m_rotatorGizmo.Startup();
	m_translatorGizmo.Startup();
}

void FBXJointGizmosManager::Update(const Camera& worldCamera)
{
	if (m_state == FBXJointGizmosManagerState::ROTATOR) {
		m_rotatorGizmo.Update(worldCamera);
	}
	else if (m_state == FBXJointGizmosManagerState::TRANSLATOR) {
		m_translatorGizmo.Update(worldCamera);
	}
	UpdateFromKeyboard();
}

void FBXJointGizmosManager::Render() const
{
	if (m_selectedJoint == nullptr)
		return;
	if (m_state == FBXJointGizmosManagerState::ROTATOR) {
		m_rotatorGizmo.Render();
	}
	else if (m_state == FBXJointGizmosManagerState::TRANSLATOR) {
		m_translatorGizmo.Render();
	}
}

FBXJoint* FBXJointGizmosManager::GetSelectedJoint() const
{
	return m_selectedJoint;
}

void FBXJointGizmosManager::SetSelectedJoint(FBXJoint* selectedJoint)
{
	m_selectedJoint = selectedJoint;
	if (m_selectedJoint && m_selectedJoint->IsRoot()) {
		m_isIKSocketMode = false;
	}
}

bool FBXJointGizmosManager::IsIKSocketMode() const
{
	return m_isIKSocketMode;
}

void FBXJointGizmosManager::SetIsShowingLocalOri(bool isShowingLocalOri)
{
	m_isShowingLocalOri = isShowingLocalOri;
}

bool FBXJointGizmosManager::IsShowingLocalOri() const
{
	return m_isShowingLocalOri;
}

void FBXJointGizmosManager::UpdateFromKeyboard()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB)) {
		if ((m_state == FBXJointGizmosManagerState::ROTATOR && m_rotatorGizmo.m_isLMBBeingUsed == false) || (m_state == FBXJointGizmosManagerState::TRANSLATOR && m_translatorGizmo.m_isLMBBeingUsed == false)) {
			IntVec2 cursorClientPos = g_theInput->GetCursorClientPos();
			uint8_t selectedStencilValueFromGlobalTexture = m_renderer.GetStencilValueOfClientPos(cursorClientPos, nullptr);

			FBXJoint* possibleNewJoint = m_model.GetJointByStencilRef(selectedStencilValueFromGlobalTexture);
			if (m_selectedJoint != possibleNewJoint) {
				SetSelectedJoint(possibleNewJoint);
			}
		}
		/*
		else if (m_state == FBXJointGizmosManagerState::TRANSLATOR && m_translatorGizmo.m_isLMBBeingUsed == false) {
			IntVec2 cursorClientPos = g_theInput->GetCursorClientPos();
			uint8_t selectedStencilValueFromGlobalTexture = m_renderer.GetStencilValueOfClientPos(cursorClientPos, nullptr);

			FBXJoint* possibleNewJoint = m_model.GetJointByStencilRef(selectedStencilValueFromGlobalTexture);
			if (m_selectedJoint != possibleNewJoint) {
				if (possibleNewJoint) {
					m_selectedJoint = &GetEndEffectorOfJoint(*possibleNewJoint);
				}
				else {
					m_selectedJoint = nullptr;
				}
			}
		}
		*/
	}

	if (m_selectedJoint == nullptr)
		return;

	if (g_theInput->WasKeyJustPressed(m_ikSocketModeToggleKey)) {
		if (m_state == FBXJointGizmosManagerState::ROTATOR && m_selectedJoint->IsRoot() == false) {
			m_isIKSocketMode = !m_isIKSocketMode;
		}
		else if (m_state == FBXJointGizmosManagerState::TRANSLATOR) {
			m_translatorGizmo.ToggleIsAlignedToGlobalAxis();
		}
	}

	if (g_theInput->WasKeyJustPressed(m_gizmoTypeToggleKey)) {
		if (m_state == FBXJointGizmosManagerState::ROTATOR) {
			m_state = FBXJointGizmosManagerState::TRANSLATOR;
			if (m_selectedJoint->IsRoot()) {
				m_isIKSocketMode = false;
			}
			else {
				m_isIKSocketMode = true;
			}
		}
		else if (m_state == FBXJointGizmosManagerState::TRANSLATOR) {
			m_state = FBXJointGizmosManagerState::ROTATOR;
		}
	}
}
