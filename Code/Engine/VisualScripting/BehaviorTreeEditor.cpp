#include "Engine/VisualScripting/BehaviorTreeEditor.hpp"
#include "Engine/VisualScripting/BehaviorTreeRootNode.hpp"
#include "Engine/VisualScripting/AvailableNodeOptionsHelper.hpp"
#include "Engine/VisualScripting/SequenceNode.hpp"
#include "Engine/VisualScripting/FallbackNode.hpp"
#include "Engine/VisualScripting/ConditionNode.hpp"
#include "Engine/VisualScripting/PlaySpriteAndDialogueNode.hpp"
#include "Engine/VisualScripting/SetBackgroundNode.hpp"
#include "Engine/VisualScripting/SetForegroundNode.hpp"
#include "Engine/VisualScripting/DeleteNodeCommand.hpp"
#include "Engine/VisualScripting/AddConnectionCommand.hpp"
#include "Engine/VisualScripting/RemoveConnectionCommand.hpp"
#include "Engine/VisualScripting/MoveNodeCommand.hpp"
#include "Engine/VisualScripting/DropVariableToConditionNodeCommand.hpp"
#include "Engine/VisualScripting/CompositeCommand.hpp"
#include "Engine/VisualNovel/VisualNovelManager.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/UI/Overlay.hpp"
#include "Engine/UI/Button.hpp"
#include "Engine/UI/VariablesBox.hpp"
#include "Engine/UI/PopupPlayer.hpp"
#include "Engine/Window/Window.hpp"
#include <stack>

BehaviorTreeEditor::BehaviorTreeEditor(const BehaviorTreeEditorConfig& config) : m_config(config)
{
}

BehaviorTreeEditor::~BehaviorTreeEditor()
{
}

void BehaviorTreeEditor::Startup()
{
	//UI Setup
	m_fixedUICamera.SetCameraMode(CameraMode::ORTHO);
	m_fixedUICamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(TREEEDITOR_DIMX, TREEEDITOR_DIMY));
	m_overlay = new Overlay(m_config.m_renderer, m_fixedUICamera.GetCameraAABB2(), Rgba8(0, 0, 0, 0));
	m_exportButton = new Button(m_config.m_renderer, *m_overlay, m_overlay->GetBounds().GetBoxWithin(Vec2(0.01f, 0.86f), Vec2(0.1f, 0.95f)), "Export XML", 20.0f, Vec2(0.5f, 0.5f), Rgba8::WHITE, Rgba8::WHITE, Rgba8::BLUE, Rgba8::PURPLE, "Export XML Button");
	m_importButton = new Button(m_config.m_renderer, *m_overlay, m_overlay->GetBounds().GetBoxWithin(Vec2(0.11f, 0.86f), Vec2(0.2f, 0.95f)), "Import XML", 20.0f, Vec2(0.5f, 0.5f), Rgba8::WHITE, Rgba8::WHITE, Rgba8::BLUE, Rgba8::PURPLE, "Import XML Button");
	m_exportButton->RegisterCallbackFunction([this]() { this->OnExportButtonPressed(); });
	m_importButton->RegisterCallbackFunction([this]() { this->OnImportButtonPressed(); });
	m_playButton = new Button(m_config.m_renderer, *m_overlay, m_overlay->GetBounds().GetBoxWithin(Vec2(0.21f, 0.86f), Vec2(0.3f, 0.95f)), "Tick Tree", 20.0f, Vec2(0.5f, 0.5f), Rgba8::WHITE, Rgba8::WHITE, Rgba8::BLUE, Rgba8::PURPLE, "Tick Tree");
	m_playButton->RegisterCallbackFunction([this]() {this->OnPlayButtonPressed(); });
	m_popupPlayer = new PopupPlayer(m_config.m_renderer, *m_overlay, m_overlay->GetBounds().GetBoxWithin(Vec2(0.55f, 0.55f), Vec2(0.99f, 0.99f)), Rgba8::WHITE, "Debug Popup Player");
	m_popupPlayer->RegisterPopupRenderCallbackFunction([this]() {this->OnPopupRender(); });

	m_variablesBox = new VariablesBox(m_config.m_renderer, *m_overlay, m_overlay->GetBounds().GetBoxWithin(Vec2(0.0f, 0.0f), Vec2(0.15f, 0.5f)), 15, Rgba8::WHITE, Rgba8::BLACK, "Blackboard");
	m_variablesBox->RegisterDragAndDropVariableCallbackFunction([this](const Vec2& normalizedCursorPos, const std::string& variableName) {this->DragAndDropVariableCallbackFunction(normalizedCursorPos, variableName); });

	m_overlay->AddWidget(*m_exportButton);
	m_overlay->AddWidget(*m_importButton);
	m_overlay->AddWidget(*m_playButton);
	m_overlay->AddWidget(*m_variablesBox);
	m_overlay->AddWidget(*m_popupPlayer);

	//VNManager setup
	Texture* dialogueBoxTexture = m_config.m_renderer.CreateOrGetTextureFromFile("Data/Images/DialogueBox.png");
	m_vnManager = new VisualNovelManager(VisualNovelManagerConfig(m_config.m_renderer), m_config.m_parentClock);
	m_vnManager->SetDialogueBoxTexture(dialogueBoxTexture);
	AABB2 playerBounds = m_popupPlayer->GetBounds();
	m_vnManager->SetBounds(playerBounds);

	//Nodes setup
	m_rootNodeToManipulate = new BehaviorTreeRootNode;
	m_allVisibleNodes.push_back(m_rootNodeToManipulate);
	m_rootNodeToManipulate->SetBehaviorTreeEditor(*this);
	m_rootNodeToManipulate->SetVNManagerRecursively(*m_vnManager);

	m_overlay->SetWhetherWidgetIsDisabledFromName("Debug Popup Player", true);

	if (m_config.m_useDefaultSettings) {
		m_editorBorderAABB2 = AABB2(0.0f, 0.0f, (float)TREEEDITOR_DIMX, (float)TREEEDITOR_DIMY);
	}
	else {
		m_editorBorderAABB2 = AABB2(0.0f, 0.0f, (float)m_config.m_overridenDimX, (float)m_config.m_overridenDimY);
	}

	m_cameraMaxDims = m_editorBorderAABB2.GetDimensions() * 0.5f;
	m_movableCamera.SetCameraMode(CameraMode::ORTHO);
	AABB2 cameraViewAABB2 = AABB2(m_editorBorderAABB2.GetBoxWithin(Vec2(0.375f, 0.375f), Vec2(0.625f, 0.625f)));
	m_movableCamera.SetOrthoView(cameraViewAABB2.m_mins, cameraViewAABB2.m_maxs, 0.0f, 1.0f);
	m_cameraMinDims = cameraViewAABB2.GetDimensions();

	//Setup GPU data
	AddVertsForAABB2(m_editorVerts, m_editorBorderAABB2, Rgba8::CHELSEA_GREY);
	m_canvasVBO = m_config.m_renderer.CreateVertexBuffer(sizeof(Vertex_PCU) * m_editorVerts.size(), sizeof(Vertex_PCU), "VBO for TreeEditor");
	m_config.m_renderer.CopyCPUToGPU(m_editorVerts.data(), sizeof(Vertex_PCU) * m_editorVerts.size(), sizeof(Vertex_PCU), m_canvasVBO);

	m_bitmapFont = m_config.m_renderer.CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	m_rootNodeToManipulate->MoveNode(cameraViewAABB2.GetPointAtUV(Vec2(0.45f, 0.8f)));

	m_nodeOptionsHelper = new AvailableNodeOptionsHelper(*this, m_config.m_renderer);

	SequenceNode* sequenceNode = new SequenceNode;
	sequenceNode->SetBehaviorTreeEditor(*this);
	RegisterTreeNodeType(*sequenceNode);
	FallbackNode* fallbackNode = new FallbackNode;
	fallbackNode->SetBehaviorTreeEditor(*this);
	RegisterTreeNodeType(*fallbackNode);
	ConditionNode* conditionNode = new ConditionNode(this, true, Vec2(100, 50));
	//conditionNode->SetBehaviorTreeEditor(*this);
	RegisterTreeNodeType(*conditionNode);
	PlaySpriteAndDialogueNode* playSpriteAndDialogueNode = new PlaySpriteAndDialogueNode(m_vnManager->GetMaxCharsPerDialogueLine(), this, true, Vec2(200, 100));
	//playSpriteAndDialogueNode->SetBehaviorTreeEditor(*this);
	RegisterTreeNodeType(*playSpriteAndDialogueNode);
	SetForegroundNode* setForegroundNode = new SetForegroundNode(this, true, Vec2(150, 40));
	RegisterTreeNodeType(*setForegroundNode);
	SetBackgroundNode* backgroundNode = new SetBackgroundNode(this, true, Vec2(150, 40));
	RegisterTreeNodeType(*backgroundNode);
}

void BehaviorTreeEditor::Shutdown()
{
	delete m_vnManager;
	delete m_overlay;
	delete m_canvasVBO;
	delete m_nodeOptionsHelper;

	for (Command*& command : m_commandHistory) {
		if (command) {
			delete command;
			command = nullptr;
		}
	}
	for (auto node : m_allVisibleNodes) {
		delete node;
	}
	for (auto node : m_garbageCollectioNodes) {
		delete node;
	}
}

void BehaviorTreeEditor::Update()
{
	GUARANTEE_OR_DIE(m_rootNodeToManipulate != nullptr, "m_rootNodeToManipulate == nullptr");

	g_theInput->SetCursorMode(false, false);

	if (m_isTicking) {
		m_vnManager->Update();
		m_rootNodeToManipulate->Tick();
		m_rootNodeToManipulate->UpdateTickFlowVertsRecursively();
	}
	else {
		for (auto node : m_allVisibleNodes) {
			node->UpdateInEditor();
		}
		UpdateFromKeyboard();
		if (m_isHelperVisible && m_nodeOptionsHelper) {
			m_nodeOptionsHelper->UpdateInEditor();
		}

		while (!m_commandsToBeExecutedQueue.empty()) {
			Command* commandToProcess = m_commandsToBeExecutedQueue.front();
			m_commandsToBeExecutedQueue.pop();
			commandToProcess->Execute();
			AddCommandToHistoryQueue(*commandToProcess);
		}
	}
	//Overlay should update regardless of whether the editor is ticking or not
	m_overlay->Update();
}

void BehaviorTreeEditor::Render() const
{
	std::vector<Vertex_PCU> lineVertices;
	if (m_nodeWhosePortIsGettingDraggedFrom) {
		Vec2 cursorPos = GetCursorPosInEditor();
		Vec2 portPos;
		float temp = 0.0f;
		if (m_isItTheTopPort) {
			m_nodeWhosePortIsGettingDraggedFrom->GetTopPortDiscData(portPos, temp);
		}
		else {
			m_nodeWhosePortIsGettingDraggedFrom->GetBottomPortDiscData(portPos, temp);
		}
		AddVertsForLineSegment2D(lineVertices, portPos, cursorPos, TREEEDITOR_CONNECTIONLINETHICKNESS, Rgba8::WHITE);
	}

	if (m_isMultiSelecting) {
		AddVertsForWireframeAABB2(lineVertices, m_multiSelectBox, m_multiSelectBoxThickness, Rgba8::BLUEGREEN);
	}

	/*
	if (m_nodeWhosePortIsGettingDraggedFrom)
		m_bitmapFont->AddVertsForTextInBox2D(textVertices, cameraAABB2.GetBoxWithin(Vec2(0.6f, 0.45f), Vec2(0.9f, 0.75f)), 100.0f, "Node is being dragged around", Rgba8::WHITE, 1.0f, Vec2(0.5f, 0.0f));
	else 
		m_bitmapFont->AddVertsForTextInBox2D(textVertices, cameraAABB2.GetBoxWithin(Vec2(0.6f, 0.45f), Vec2(0.9f, 0.75f)), 100.0f, "Node is NOT being dragged around", Rgba8::WHITE, 1.0f, Vec2(0.5f, 0.0f));
	*/

	m_config.m_renderer.BeginCamera(m_movableCamera);
	m_config.m_renderer.BindShader(&m_config.m_shaderToUse);

	m_config.m_renderer.SetModelConstants();
	m_config.m_renderer.BindTexture(nullptr);
	if (m_canvasVBO) {
		m_config.m_renderer.DrawVertexBuffer(m_canvasVBO, (int)m_editorVerts.size());
	}
	m_config.m_renderer.BindShader(nullptr);


	for (BehaviorTreeNode* visibleNode : m_allVisibleNodes) {
		auto it = std::find(m_selectedNodes.begin(), m_selectedNodes.end(), visibleNode);
		if (it == m_selectedNodes.end())
			visibleNode->RenderInEditor();
	}
	if (m_selectedNodes.size() > 0) {
		for (auto selectedNode : m_selectedNodes) {
			selectedNode->RenderInEditor();
		}
	}

	if (m_isHelperVisible && m_nodeOptionsHelper) {
		//m_nodeOptionsHelper->AddOutlineVertsForMe(nodeVertices);
		//m_nodeOptionsHelper->AddTextVertsForMe(*m_bitmapFont, textVertices);
		m_nodeOptionsHelper->Render();
	}

	m_config.m_renderer.BindTexture(nullptr);
	m_config.m_renderer.BindShader(nullptr);
	m_config.m_renderer.DrawVertexArray((int)lineVertices.size(), lineVertices.data());

	/* 
	//DebugRender
	std::vector<Vertex_PCU> debugTextVertices;
	AABB2 cameraAABB2 = m_movableCamera.GetCameraAABB2();
	Vec2 cameraCenter = cameraAABB2.GetCenter();
	Vec2 cursorPos = GetCursorPosInEditor();
	m_bitmapFont->AddVertsForTextInBox2D(debugTextVertices, cameraAABB2.GetBoxWithin(Vec2(0.6f, 0.3f), Vec2(0.9f, 0.45f)), 100.0f, Stringf("%.1f, %.1f", cursorPos.x, cursorPos.y), Rgba8::WHITE, FONT_ASPECT, Vec2(0.5f, 0.0f));
	m_bitmapFont->AddVertsForTextInBox2D(debugTextVertices, cameraAABB2.GetBoxWithin(Vec2(0.6f, 0.1f), Vec2(0.9f, 0.3f)), 100.0f, Stringf("Start: %d, End: %d, Curr: %d", m_historyArrayStart, m_historyArrayEnd, m_currentCommandHistoryIdx), Rgba8::WHITE, FONT_ASPECT, Vec2(0.5f, 0.0f));

	m_config.m_renderer.BindTexture(&m_bitmapFont->GetTexture());
	m_config.m_renderer.DrawVertexArray((int)debugTextVertices.size(), debugTextVertices.data());
	*/

	m_config.m_renderer.EndCamera(m_movableCamera);

	m_config.m_renderer.BeginCamera(m_fixedUICamera);
	m_overlay->Render();
	m_config.m_renderer.EndCamera(m_fixedUICamera);
}

void BehaviorTreeEditor::SetDifferentTreeToManipulate(BehaviorTreeRootNode& treeRootToManipulate)
{
	m_rootNodeToManipulate = &treeRootToManipulate;
}

void BehaviorTreeEditor::RegisterTreeNodeType(const BehaviorTreeNode& prototypeNode)
{
	m_nameAndPrototypeTreeNodeMap[prototypeNode.GetNodeDisplayStr()] = &prototypeNode;
}

const std::map<std::string, const BehaviorTreeNode*>& BehaviorTreeEditor::GetNameAndPrototypeNodeMap() const
{
	return m_nameAndPrototypeTreeNodeMap;
}

Vec2 BehaviorTreeEditor::GetCursorPosInEditor() const
{
	Vec2 normalizedClientPos = g_theInput->GetNormalizedCursorPos();
	return m_movableCamera.GetCameraAABB2().GetPointAtUV(normalizedClientPos);
}

void BehaviorTreeEditor::SetHelperVisibility(bool isVisible)
{
	m_isHelperVisible = isVisible;
}

void BehaviorTreeEditor::AddCommandToExecuteQueue(Command& commandToExecute)
{
	m_commandsToBeExecutedQueue.push(&commandToExecute);
}

BehaviorTreeNode* BehaviorTreeEditor::PlaceNodeAtPos(const std::string& nodeName, const Vec2& nodePos)
{
	auto foundPair = m_nameAndPrototypeTreeNodeMap.find(nodeName);
	GUARANTEE_OR_DIE(foundPair != m_nameAndPrototypeTreeNodeMap.end(), Stringf("Check nodeName: %s. It doesn't exist", nodeName.c_str()));
	const BehaviorTreeNode* node = foundPair->second;
	BehaviorTreeNode* newNode = node->Clone(nodePos);
	m_allVisibleNodes.push_back(newNode);
	return newNode;
}

void BehaviorTreeEditor::BringBackNodeFromGarbageCollection(BehaviorTreeNode& node)
{
	m_allVisibleNodes.push_back(&node);
	m_garbageCollectioNodes.remove(&node);
}

AABB2 BehaviorTreeEditor::GetMovableCameraAABB2() const
{
	return m_movableCamera.GetCameraAABB2();
}

Renderer& BehaviorTreeEditor::GetRefToRenderer() const
{
	return m_config.m_renderer;
}

NamedProperties BehaviorTreeEditor::GetBlackboardValues() const
{
	Widget* widget = m_overlay->GetWidgetFromName("Blackboard");
	VariablesBox* box = dynamic_cast<VariablesBox*>(widget);
	GUARANTEE_OR_DIE(box != nullptr, "Check VariablesBox Widget name for BehaviorTreeEditor!");
	return box->GetNamedProperties();
}

void BehaviorTreeEditor::DeleteNodeToGarbageCollection(BehaviorTreeNode& node)
{
	m_allVisibleNodes.remove(&node);
	m_garbageCollectioNodes.push_back(&node);
	m_selectedNodes.clear();
}

void BehaviorTreeEditor::UpdateFromKeyboard()
{
	int wheelScrollAmount = 0;
	if (g_theInput->WasWheelScrolled(&wheelScrollAmount)) {
		Vec2 originalCursorPos = GetCursorPosInEditor();
		AABB2 newCameraViewAABB2 = m_movableCamera.GetCameraAABB2();
		Vec2 cameraViewDim = newCameraViewAABB2.GetDimensions();
		Vec2 newDim;
		if (wheelScrollAmount < 0.0f) {
			newDim = Vec2(
				GetClamped(cameraViewDim.x * 1.05f, m_cameraMinDims.x, m_cameraMaxDims.x), 
				GetClamped(cameraViewDim.y * 1.05f, m_cameraMinDims.y, m_cameraMaxDims.y)
			);
		}
		else {
			newDim = Vec2(
				GetClamped(cameraViewDim.x * 0.95f, m_cameraMinDims.x, m_cameraMaxDims.x),
				GetClamped(cameraViewDim.y * 0.95f, m_cameraMinDims.y, m_cameraMaxDims.y)
			);
		}
		newCameraViewAABB2.SetDimensions(newDim);
		newCameraViewAABB2 = m_editorBorderAABB2.FixBoxWithinThis(newCameraViewAABB2);
		m_movableCamera.SetOrthoView(newCameraViewAABB2);
		Vec2 newCursorPos = GetCursorPosInEditor();

		Vec2 fromNewCursorPosToOriginal = originalCursorPos - newCursorPos;
		newCameraViewAABB2.Translate(fromNewCursorPosToOriginal);
		newCameraViewAABB2 = m_editorBorderAABB2.FixBoxWithinThis(newCameraViewAABB2);
		m_movableCamera.SetOrthoView(newCameraViewAABB2);
	}

	if (g_theInput->IsKeyDown(KEYCODE_RMB)) {
		if (m_isPanningMode) {
			IntVec2 currentMousePos = g_theInput->GetCursorClientPos();
			IntVec2 mouseOffset = currentMousePos - m_previousFrameCursorPos;
			if (mouseOffset.GetLengthSquared() > m_minimumPanLengthSquared) {
				m_didMoveWhilePanning = true;
				Vec2 currentCursorPos = GetCursorPosInEditor();
				Vec2 fromInitialToCurrentCursorPos = m_panningStartMousePos - currentCursorPos;
				AABB2 cameraAABB = m_movableCamera.GetCameraAABB2();
				cameraAABB.Translate(fromInitialToCurrentCursorPos);
				m_movableCamera.SetOrthoView(cameraAABB);
			}
			m_previousFrameCursorPos = g_theInput->GetCursorClientPos();
		}
		if (m_isPanningMode == false) {
			m_isPanningMode = true;
			m_panningStartMousePos = GetCursorPosInEditor();
			m_didMoveWhilePanning = false;
			m_previousFrameCursorPos = g_theInput->GetCursorClientPos();
		}
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_RMB)) {
		m_isPanningMode = false;
		if (m_didMoveWhilePanning == false) {
			m_isHelperVisible = true;
			Vec2 newPosForHelperBox = GetCursorPosInEditor();
			m_nodeOptionsHelper->SetPositionToFitEditor(newPosForHelperBox);
		}
	}

	if (m_isNodeGrabbed && m_selectedNodes.size() > 0) {
		GUARANTEE_OR_DIE(m_selectedNodes.size() == m_grabbedNodeOriginalPos.size() && m_grabbedNodeOriginalPos.size() && m_offsetFromGrabbedPosToNodeStartPos.size(), "Check logic");

		for (int selectedNodeIdx = 0; selectedNodeIdx < m_selectedNodes.size(); selectedNodeIdx++) {
			if (m_selectedNodes[selectedNodeIdx]->IsMovable()) {
				Vec2 cursorPos = GetCursorPosInEditor();
				Vec2 newNodePos = cursorPos + m_offsetFromGrabbedPosToNodeStartPos[selectedNodeIdx];
				m_selectedNodes[selectedNodeIdx]->MoveNode(newNodePos);
			}
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB)) {
		if (g_theInput->IsKeyDown(KEYCODE_ALT)) {
			Vec2 cursorPos = GetCursorPosInEditor();
			//Check for ports
			Vec2 portCenter;
			float portDiscRadius = 0.0f;
			for (auto node : m_allVisibleNodes) {
				if (node) {
					node->GetTopPortDiscData(portCenter, portDiscRadius);
					if (IsPointInsideDisc2D(cursorPos, portCenter, portDiscRadius)) {
						BehaviorTreeNode* parent = node->GetParentNode();
						if (parent) {
							AddCommandToExecuteQueue(*new RemoveConnectionCommand(*this, *parent, *node));
							break;
						}
					}
					node->GetBottomPortDiscData(portCenter, portDiscRadius);
					if (IsPointInsideDisc2D(cursorPos, portCenter, portDiscRadius)) {
						const std::list<BehaviorTreeNode*>& childNodesList = node->GetChildNodes();
						if (childNodesList.size() > 0) {
							AddCommandToExecuteQueue(*new RemoveConnectionCommand(*this, *node, childNodesList));
							break;
						}
					}
				}
			}
		}
		else {
			Vec2 cursorPos = GetCursorPosInEditor();
			//Check for ports
			Vec2 portCenter;
			float portDiscRadius = 0.0f;
			bool wasClickedUsedForNode = false;
			for (auto node : m_allVisibleNodes) {
				if (node) {
					node->GetTopPortDiscData(portCenter, portDiscRadius);
					if (IsPointInsideDisc2D(cursorPos, portCenter, portDiscRadius)) {
						m_nodeWhosePortIsGettingDraggedFrom = node;
						m_isItTheTopPort = true;
						wasClickedUsedForNode = true;
						break;
					}
					node->GetBottomPortDiscData(portCenter, portDiscRadius);
					if (IsPointInsideDisc2D(cursorPos, portCenter, portDiscRadius)) {
						m_nodeWhosePortIsGettingDraggedFrom = node;
						m_isItTheTopPort = false;
						wasClickedUsedForNode = true;
						break;
					}

					if (node->IsCursorInside(cursorPos)) {
						m_isNodeGrabbed = true;
						Vec2 nodeCurrentPos = node->GetCurrentPos();

						auto it = std::find(m_selectedNodes.begin(), m_selectedNodes.end(), node);
						if (it == m_selectedNodes.end()) {
							m_selectedNodes.clear();
							m_offsetFromGrabbedPosToNodeStartPos.clear();
							m_grabbedNodeOriginalPos.clear();
							m_selectedNodes.push_back(node);
							m_offsetFromGrabbedPosToNodeStartPos.push_back(nodeCurrentPos - cursorPos);
							m_grabbedNodeOriginalPos.push_back(nodeCurrentPos);
						}
						else {
							m_offsetFromGrabbedPosToNodeStartPos.clear();
							m_grabbedNodeOriginalPos.clear();
							for (auto selectedNode : m_selectedNodes) {
								Vec2 selectedNodePos = selectedNode->GetPosInEditor();
								m_offsetFromGrabbedPosToNodeStartPos.push_back(selectedNodePos - cursorPos);
								m_grabbedNodeOriginalPos.push_back(selectedNodePos);
							}
						}
						wasClickedUsedForNode = true;
					}
				}
			}

			if (wasClickedUsedForNode == false) {
				m_selectedNodes.clear();
				m_offsetFromGrabbedPosToNodeStartPos.clear();
				m_grabbedNodeOriginalPos.clear();
				m_multiSelectStartPoint = cursorPos;
				m_isMultiSelecting = true;
			}
		}
	}

	if (m_isMultiSelecting) {
		Vec2 cursorPos = GetCursorPosInEditor();
		m_multiSelectBox = CreateAABB2FromTwoUnorderedPoints(cursorPos, m_multiSelectStartPoint);
		for (auto visibleNode: m_allVisibleNodes) {
			if (DoAABB2sOverlap(visibleNode->GetAABB2(), m_multiSelectBox)) {
				auto it = std::find(m_selectedNodes.begin(), m_selectedNodes.end(), visibleNode);
				if (it == m_selectedNodes.end()) {
					m_selectedNodes.push_back(visibleNode);
				}
			}
		}
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_LMB)) {
		m_isMultiSelecting = false;

		if (m_nodeWhosePortIsGettingDraggedFrom) {
			Vec2 cursorPos = GetCursorPosInEditor();
			Vec2 portCenter;
			float portDiscRadius = 0.0f;
			bool wasConnectionAdded = false;
			for (auto node : m_allVisibleNodes) {
				if (node != m_nodeWhosePortIsGettingDraggedFrom) {
					if (m_isItTheTopPort) {
						node->GetBottomPortDiscData(portCenter, portDiscRadius);
						if (IsPointInsideDisc2D(cursorPos, portCenter, portDiscRadius) && node->IsNodeAChild(*m_nodeWhosePortIsGettingDraggedFrom) == false) {
							//m_nodeWhosePortIsGettingDraggedFrom->AddNodeToChildren(*node);
							AddCommandToExecuteQueue(*new AddConnectionCommand(*this, *node, *m_nodeWhosePortIsGettingDraggedFrom));
							wasConnectionAdded = true;
							break;
						}
					}
					else {
						node->GetTopPortDiscData(portCenter, portDiscRadius);
						if (IsPointInsideDisc2D(cursorPos, portCenter, portDiscRadius) && m_nodeWhosePortIsGettingDraggedFrom->IsNodeAChild(*node) == false) {
							//m_nodeWhosePortIsGettingDraggedFrom->AddNodeToChildren(*node);
							AddCommandToExecuteQueue(*new AddConnectionCommand(*this, *m_nodeWhosePortIsGettingDraggedFrom, *node));
							wasConnectionAdded = true;
							break;
						}
					}
				}
			}
			if (wasConnectionAdded == false) {
				m_isHelperVisible = true;
				m_nodeOptionsHelper->SetPositionToFitEditor(GetCursorPosInEditor());
				m_nodeOptionsHelper->ConnectNextNewNodeToThisNode(*m_nodeWhosePortIsGettingDraggedFrom, m_isItTheTopPort);
			}
			m_nodeWhosePortIsGettingDraggedFrom = nullptr;
		}

		if (m_isNodeGrabbed && m_selectedNodes.size() > 0) {
			GUARANTEE_OR_DIE(m_selectedNodes.size() == m_grabbedNodeOriginalPos.size() && m_grabbedNodeOriginalPos.size() && m_offsetFromGrabbedPosToNodeStartPos.size(), "Check logic");
			m_isNodeGrabbed = false;
			if (m_selectedNodes.size() == 1){
				if (m_selectedNodes[0]->IsMovable()) {
					AddCommandToExecuteQueue(*new MoveNodeCommand(*this, *m_selectedNodes[0], m_selectedNodes[0]->GetCurrentPos(), m_grabbedNodeOriginalPos[0]));
				}
			}
			else {
				std::vector<Command*> moveCommands;
				for (int selectedNodeIdx = 0; selectedNodeIdx < m_selectedNodes.size(); selectedNodeIdx++) {
					BehaviorTreeNode* selectedNode = m_selectedNodes[selectedNodeIdx];
					moveCommands.push_back(new MoveNodeCommand(*this, *selectedNode, selectedNode->GetCurrentPos(), m_grabbedNodeOriginalPos[selectedNodeIdx]));
				}
				AddCommandToExecuteQueue(*new CompositeCommand(moveCommands));
			}
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_DELETE)) {
		if (m_selectedNodes.size() == 1) {
			if (m_selectedNodes[0]->IsDeletable()) {
				AddCommandToExecuteQueue(*new DeleteNodeCommand(*this, *m_selectedNodes[0]));
			}
		}
		else {
			std::vector<Command*> deleteCommands;
			for (auto selectedNode : m_selectedNodes) {
				deleteCommands.push_back(new DeleteNodeCommand(*this, *selectedNode));
			}
			AddCommandToExecuteQueue(*new CompositeCommand(deleteCommands));
		}
	}

	if (g_theInput->IsKeyDown(KEYCODE_CONTROL) && g_theInput->WasKeyJustPressed('Z')) {
		if (g_theInput->IsKeyDown(KEYCODE_SHIFT) == false) {
			UndoCommand();
		}
		else {
			RedoCommand();
		}
	}
}

void BehaviorTreeEditor::AddCommandToHistoryQueue(Command& command)
{
	if ((m_historyArrayEnd != -1) && ((m_historyArrayEnd + 1) % MAXCOMMANDHISTORYNUM == m_historyArrayStart)) {
		//Delete the soon-to-be overwritten command
		delete m_commandHistory[m_historyArrayStart];
		m_historyArrayStart = (m_historyArrayStart + 1) % MAXCOMMANDHISTORYNUM;	//Since the original element at m_historyArrayStart will get overwritten, we'll increase where the start of the history array is at
	}
	m_historyArrayEnd = (m_historyArrayEnd + 1) % MAXCOMMANDHISTORYNUM;	//Increment the history array end index
	m_commandHistory[m_historyArrayEnd] = &command;	//Store the command there

	//m_currentCommandHistoryIdx will be the newly pushed command
	m_currentCommandHistoryIdx = m_historyArrayEnd;
}

Clock& BehaviorTreeEditor::GetClock() const
{
	return m_config.m_parentClock;
}

void BehaviorTreeEditor::UndoCommand()
{
	if (m_historyArrayEnd == -1) {	//-1 means nothing's there yet
		return;
	}

	if (m_currentCommandHistoryIdx != -1 && IsCurrentHistoryIdxValid()) {
		m_commandHistory[m_currentCommandHistoryIdx]->Undo();
		unsigned int distFromStartToCurrHistory = (m_currentCommandHistoryIdx - m_historyArrayStart + MAXCOMMANDHISTORYNUM) % MAXCOMMANDHISTORYNUM;
		if (distFromStartToCurrHistory > 0)
			m_currentCommandHistoryIdx = (m_currentCommandHistoryIdx - 1 + MAXCOMMANDHISTORYNUM) % MAXCOMMANDHISTORYNUM; //<- How should I Update It?
		else //If m_currentCommandHistoryIdx == m_historyArrayStart
		{
			m_currentCommandHistoryIdx = -1;
		}
	}
}

void BehaviorTreeEditor::RedoCommand()
{
	if (m_historyArrayEnd == -1) {	//-1 means nothing's there yet
		return;
	}

	if (m_currentCommandHistoryIdx == -1) {	//This means this command->Undo() happened for the startIdx
		m_currentCommandHistoryIdx = m_historyArrayStart;
		m_commandHistory[m_currentCommandHistoryIdx]->Execute();
		return;
	}
	unsigned int distFromCurrToEndHistory = (m_historyArrayEnd - m_currentCommandHistoryIdx + MAXCOMMANDHISTORYNUM) % MAXCOMMANDHISTORYNUM;
	if (IsCurrentHistoryIdxValid() && distFromCurrToEndHistory > 0) {
		m_currentCommandHistoryIdx = (m_currentCommandHistoryIdx + 1) % MAXCOMMANDHISTORYNUM;
		m_commandHistory[m_currentCommandHistoryIdx]->Execute();
	}
}

void BehaviorTreeEditor::DeleteTickFlowVBOIfNotInThisList(const std::vector<BehaviorTreeNode*>& arrayOfNodesThatTickedThisUpdate) const
{
	for (auto visibleNode : m_allVisibleNodes) {
		bool didThisNodeTick = false;
		for (auto nodeThatTicked : arrayOfNodesThatTickedThisUpdate) {
			if (visibleNode == nodeThatTicked) {
				didThisNodeTick = true;
				break;
			}
		}
		if (didThisNodeTick == false) {
			visibleNode->DeleteTickFlowVBO();
		}
	}
}

bool BehaviorTreeEditor::IsCurrentHistoryIdxValid() const
{
	unsigned int startToEndOffset = (m_historyArrayEnd - m_historyArrayStart + MAXCOMMANDHISTORYNUM) % MAXCOMMANDHISTORYNUM;
	unsigned int startToCurrentOffset = (m_currentCommandHistoryIdx - m_historyArrayStart + MAXCOMMANDHISTORYNUM) % MAXCOMMANDHISTORYNUM;
	return startToCurrentOffset <= startToEndOffset;
}

std::vector<BehaviorTreeNode*> BehaviorTreeEditor::GetSelectedNodes() const
{
	return m_selectedNodes;
}

void BehaviorTreeEditor::ExportToXML(const std::string& xmlFileDirWithoutExtension) const
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLElement* rootElement = doc.NewElement("RootNode");
	std::map<std::string, std::string> rootNodeAttribs = m_rootNodeToManipulate->GetAndSetXMLAttributes();
	for (auto rootNodeAttrib : rootNodeAttribs) {
		rootElement->SetAttribute(rootNodeAttrib.first.c_str(), rootNodeAttrib.second.c_str());
	}
	doc.InsertFirstChild(rootElement);

	struct NodeAndXMLElement {	//Need this bc you're not using the recursive function approach, you're using a stack approach for DFS
	public:
		NodeAndXMLElement(BehaviorTreeNode& node, tinyxml2::XMLElement& parentElement) :  m_node(node), m_parentElement(parentElement) {};
		BehaviorTreeNode& m_node;
		tinyxml2::XMLElement& m_parentElement;
	};

	std::queue<NodeAndXMLElement> pairsToProcess;
	if (m_rootNodeToManipulate->GetNumChildNodes() > 0)
		pairsToProcess.push(NodeAndXMLElement(*m_rootNodeToManipulate->GetChildNodes().front(), *rootElement));
	
	while (!pairsToProcess.empty()) {
		NodeAndXMLElement pair = pairsToProcess.front();
		pairsToProcess.pop();
		tinyxml2::XMLElement* nodeElement = doc.NewElement(pair.m_node.GetNodeDisplayStr().c_str());
		const auto& map = pair.m_node.GetAndSetXMLAttributes();
		for (auto keyValAttribPair : map) {
			nodeElement->SetAttribute(keyValAttribPair.first.c_str(), keyValAttribPair.second.c_str());
		}
		pair.m_parentElement.InsertEndChild(nodeElement);
		for (auto childNode : pair.m_node.GetChildNodes()) {
			pairsToProcess.push(NodeAndXMLElement(*childNode, *nodeElement));
		}
	}

	std::string finalXMLFileDir = xmlFileDirWithoutExtension + "/" + m_rootNodeToManipulate->GetTypedFileName() + ".xml";
	tinyxml2::XMLError eResult = doc.SaveFile(finalXMLFileDir.c_str());
	if (eResult != tinyxml2::XML_SUCCESS) {
		ERROR_RECOVERABLE(Stringf("Export of %s not successful", finalXMLFileDir.c_str()));
	}
	return;
}

void BehaviorTreeEditor::ImportXMLFile(const std::string& xmlFileDir)
{
	//Clear all data
	for (auto node : m_garbageCollectioNodes){
		delete node;
	}
	m_garbageCollectioNodes.clear();
	for (auto node : m_allVisibleNodes) {
		delete node;
	}
	m_allVisibleNodes.clear();
	m_rootNodeToManipulate = nullptr;
	if(m_variablesBox)
		m_variablesBox->Clear();

	for (Command*& command : m_commandHistory) {
		if (command) {
			delete command;
			command = nullptr;
		}
	}
	m_currentCommandHistoryIdx = 0;
	m_historyArrayStart = 0;
	m_historyArrayEnd = -1;

	m_isMultiSelecting = false;
	m_selectedNodes.clear();
	m_offsetFromGrabbedPosToNodeStartPos.clear();
	m_grabbedNodeOriginalPos.clear();

	XmlDocument xmlDoc;
	if (xmlDoc.LoadFile(xmlFileDir.c_str()) == XmlResult::XML_SUCCESS) {
		XmlElement* rootNodeElementPtr = xmlDoc.RootElement();
		const XmlAttribute* attribute = rootNodeElementPtr->FindAttribute("fileName");
		GUARANTEE_OR_DIE(attribute != nullptr, "Root must have attribute 'fileName'");
		std::string fileName = attribute->Value();
		BehaviorTreeRootNode* newRootNode = new BehaviorTreeRootNode;
		newRootNode->SetBehaviorTreeEditor(*this);
		newRootNode->SetTypedFileName(fileName);
		m_allVisibleNodes.push_back(newRootNode);
		m_rootNodeToManipulate = newRootNode;
		AABB2 tempAABBForRootNocePlacement = AABB2(m_editorBorderAABB2.GetBoxWithin(Vec2(0.375f, 0.375f), Vec2(0.625f, 0.625f)));
		m_rootNodeToManipulate->MoveNode(tempAABBForRootNocePlacement.GetPointAtUV(Vec2(0.45f, 0.8f)));

		struct NodeAndXMLElement {	//Need this bc you're not using the recursive function approach, you're using a stack approach for DFS
		public:
			NodeAndXMLElement(BehaviorTreeNode& parentNode, tinyxml2::XMLElement& childElement) : m_parentNode(parentNode), m_childElement(childElement) {};
			BehaviorTreeNode& m_parentNode;
			tinyxml2::XMLElement& m_childElement;
		};
		std::queue<NodeAndXMLElement> nodeXMLElementPairsToProcess;

		if (rootNodeElementPtr->FirstChildElement()) {
			nodeXMLElementPairsToProcess.push(NodeAndXMLElement(*m_rootNodeToManipulate, *rootNodeElementPtr->FirstChildElement()));
		}

		while (!nodeXMLElementPairsToProcess.empty()) {
			auto pair = nodeXMLElementPairsToProcess.front();
			nodeXMLElementPairsToProcess.pop();
			BehaviorTreeNode* childNode = CreateNodeFromXMLElement(pair.m_childElement);
			GUARANTEE_OR_DIE(childNode != nullptr, "CreateNodeFromXMLElement() returned a nullptr");
			pair.m_parentNode.AddNodeToChildren(*childNode);
			XmlElement* childChildElement = pair.m_childElement.FirstChildElement();
			while (childChildElement != nullptr) {
				nodeXMLElementPairsToProcess.push(NodeAndXMLElement(*childNode, *childChildElement));
				childChildElement = childChildElement->NextSiblingElement();
			}
		}
	}
	else {
		ERROR_RECOVERABLE(Stringf("XML File: %s loading unsucessful", xmlFileDir.c_str()));
	}

	m_rootNodeToManipulate->SetVNManagerRecursively(*m_vnManager);
}

bool BehaviorTreeEditor::IsCurrentSetupValid() const
{
	bool areAllNodesValid = true;
	for (auto node : m_allVisibleNodes) {
		if (node) {
			if (node->CheckSetupValidity() == false) {
				areAllNodesValid = false;
			}
		}
	}
	return areAllNodesValid;
}

void BehaviorTreeEditor::OnExportButtonPressed() const
{
	if (IsCurrentSetupValid() == false) {
		DebuggerPrintf("Current setup is NOT valid\n");
		return;
	}
	std::string directoryPath;
	m_config.m_window.GetDirectoryPath(directoryPath);
	if (directoryPath.length() == 0) {
		return;
	}
	ExportToXML(directoryPath);
	return;
}

void BehaviorTreeEditor::OnImportButtonPressed()
{
	std::string xmlFilePath;
	m_config.m_window.GetXMLFileName(xmlFilePath, KEYCODE_LMB);
	if (xmlFilePath.length() == 0 || DoesFileExistOnDisk(xmlFilePath) == false) {
		return;
	}
	ImportXMLFile(xmlFilePath);
	return;
}

void BehaviorTreeEditor::OnPlayButtonPressed()
{
	if (IsCurrentSetupValid() == false)
		return;

	m_isTicking = !m_isTicking;

	if (m_isTicking) {
		m_rootNodeToManipulate->SetVNManagerRecursively(*m_vnManager);
		m_overlay->SetWhetherWidgetIsDisabledFromName("Debug Popup Player", false);
		dynamic_cast<Button*>(m_playButton)->SetText("Stop Ticking");
	}
	else {
		m_rootNodeToManipulate->AlertTickStoppedRecursively();
		m_overlay->SetWhetherWidgetIsDisabledFromName("Debug Popup Player", true);
		dynamic_cast<Button*>(m_playButton)->SetText("Tick Tree");
	}
}

void BehaviorTreeEditor::DragAndDropVariableCallbackFunction(const Vec2& normalizedCursorPos, const std::string& variableName)
{
	//Check if the dropped position is an input of Condition Node! If not... do nothing
	for (BehaviorTreeNode* node : m_allVisibleNodes) {
		ConditionNode* cNode = dynamic_cast<ConditionNode*>(node);
		Vec2 editorPos =  m_movableCamera.GetCameraAABB2().GetPointAtUV(normalizedCursorPos);
		if (cNode && cNode->GetAABB2().IsPointInside(editorPos)) {
			bool isLeft = false;
			if (cNode->IsPosInTextTypeBar(editorPos, isLeft)) {
				AddCommandToExecuteQueue(*new DropVariableToConditionNodeCommand(*cNode, variableName));
				//cNode->RegisterVariable(isLeft, variableName);
			}
		}
	}
}

BehaviorTreeNode* BehaviorTreeEditor::CreateNodeFromXMLElement(const XmlElement& element)
{
	std::string elementName(element.Name());

	Vec2 pos;
	const XmlAttribute* posAttrib = element.FindAttribute("position");
	GUARANTEE_OR_DIE(posAttrib != nullptr, "All nodes MUST have attribute position when exported");

	pos.SetFromText(posAttrib->Value());
	BehaviorTreeNode* node = PlaceNodeAtPos(elementName, pos);
	GUARANTEE_OR_DIE(node != nullptr, "Node cloning unsuccessful");

	node->FillInMissingInformationFromXmlElement(element);
	return node;
}

void BehaviorTreeEditor::OnPopupRender()
{
	AABB2 playerBounds = m_popupPlayer->GetBounds();
	m_vnManager->SetBounds(playerBounds);
	//m_vnManager->SetBackground(m_config.m_renderer.CreateOrGetTextureFromFile("Data/Images/DefenseBackground.png"));
	//m_vnManager->SetForeground(m_config.m_renderer.CreateOrGetTextureFromFile("Data/Images/DefensePodiumTranslucent.png"));
	m_vnManager->Render();
}

bool BehaviorTreeEditor::IsTicking() const
{
	return m_isTicking;
}
