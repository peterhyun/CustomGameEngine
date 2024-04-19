#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/VisualScripting/Command.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include <list>
#include <map>
#include <string>
#include <queue>
#include <vector>

constexpr int MAXCOMMANDHISTORYNUM = 50;
constexpr float TREEEDITOR_DIMX = 4096;
constexpr float TREEEDITOR_DIMY = 2048;
constexpr float FONT_ASPECT = 0.6f;
constexpr float TREEEDITOR_CONNECTIONLINETHICKNESS = 0.5f;

class BehaviorTreeNode;
class BehaviorTreeRootNode;
class Shader;
class VertexBuffer;
class BitmapFont;
class AvailableNodeOptionsHelper;
class VariablesBox;
class NamedProperties;

class Overlay;
class Button;
class PopupPlayer;

class VisualNovelManager;

struct BehaviorTreeEditorConfig {
public:
	BehaviorTreeEditorConfig(class Renderer& rendererToUse, class Window& window, Shader& shaderToUse, Clock& parentClock, bool useDefaultSettings = true, unsigned int overriddenDimX = TREEEDITOR_DIMX, unsigned int overridenDimY = TREEEDITOR_DIMY)
		: m_renderer(rendererToUse), m_window(window), m_shaderToUse(shaderToUse), m_parentClock(parentClock), m_useDefaultSettings(useDefaultSettings), m_overridenDimX(overriddenDimX), m_overridenDimY(overridenDimY)
	{};

public:
	class Renderer& m_renderer;
	class Window& m_window;
	Shader& m_shaderToUse;
	Clock& m_parentClock;
	bool m_useDefaultSettings;
	unsigned int m_overridenDimX;
	unsigned int m_overridenDimY;
};

class BehaviorTreeEditor {
public:
	BehaviorTreeEditor(const BehaviorTreeEditorConfig& config);
	~BehaviorTreeEditor();
	void Startup();	//Set the camera and border dimensions
	void Shutdown();
	void Update();
	void Render() const;

	void SetDifferentTreeToManipulate(BehaviorTreeRootNode& treeRootToManipulate);
	void RegisterTreeNodeType(const BehaviorTreeNode& prototypeNode);	//If game code inherits to a new Node type, you have to register it here with a prototype instance.

	const std::map<std::string, const BehaviorTreeNode*>& GetNameAndPrototypeNodeMap() const;
	Vec2 GetCursorPosInEditor() const;

	void SetHelperVisibility(bool isVisible);

	void AddCommandToExecuteQueue(Command& commandToExecute);

	//Use Commands to call this
	BehaviorTreeNode* PlaceNodeAtPos(const std::string& nodeName, const Vec2& nodePos);
	void DeleteNodeToGarbageCollection(BehaviorTreeNode& node);	//<- I'm gonna change my approach. Not gonna delete, but move to garbage collection
	void BringBackNodeFromGarbageCollection(BehaviorTreeNode& node);

	AABB2 GetMovableCameraAABB2() const;

	Renderer& GetRefToRenderer() const;

	//Needed for ConditionNodes to check if their settings are valid
	NamedProperties GetBlackboardValues() const;

	std::vector<BehaviorTreeNode*> GetSelectedNodes() const;

	Clock& GetClock() const;

	bool IsTicking() const;

	//Utility function called from the root
	void DeleteTickFlowVBOIfNotInThisList(const std::vector<BehaviorTreeNode*>& arrayOfNodesThatTickedThisUpdate) const;
	
private:
	void UpdateFromKeyboard();	//Handle Mouse scrolling, Mouse translation. Also node selection, node deletion, node copy.
	void AddCommandToHistoryQueue(Command& command);
	void UndoCommand();
	void RedoCommand();

	bool IsCurrentHistoryIdxValid() const;

	void ExportToXML(const std::string& xmlFileDir) const;
	void ImportXMLFile(const std::string& xmlFileDir);
	bool IsCurrentSetupValid() const;

	void OnExportButtonPressed() const;
	void OnImportButtonPressed();
	void OnPlayButtonPressed();

	void DragAndDropVariableCallbackFunction(const Vec2& normalizedCursorPos, const std::string& variableName);

	BehaviorTreeNode* CreateNodeFromXMLElement(const XmlElement& element);

	void OnPopupRender();
	
private:
	BehaviorTreeEditorConfig m_config;
	BehaviorTreeRootNode* m_rootNodeToManipulate = nullptr;
	BehaviorTreeNode* m_currentlyActiveNode = nullptr;	//The node selected from LMB
	std::list<BehaviorTreeNode*> m_allVisibleNodes;
	std::list<BehaviorTreeNode*> m_garbageCollectioNodes;

	//Variables for panning
	bool m_isPanningMode = false;
	Vec2 m_panningStartMousePos;
	int m_minimumPanLengthSquared = 1;
	bool m_didMoveWhilePanning = false;
	IntVec2 m_previousFrameCursorPos;

	Camera m_movableCamera;
	AABB2 m_editorBorderAABB2;

	Camera m_fixedUICamera;

	Vec2 m_cameraMinDims;
	Vec2 m_cameraMaxDims;

	Command* m_commandHistory[MAXCOMMANDHISTORYNUM] = {nullptr};
	int m_currentCommandHistoryIdx = 0;
	int m_historyArrayStart = 0;
	int m_historyArrayEnd = -1;	//Should be -1 at the beginning

	//Graphics related stuff
	std::vector<Vertex_PCU> m_editorVerts;
	VertexBuffer* m_canvasVBO;

	BitmapFont* m_bitmapFont = nullptr;

	AvailableNodeOptionsHelper* m_nodeOptionsHelper = nullptr;
	bool m_isHelperVisible = false;

	//For port connecting
	BehaviorTreeNode* m_nodeWhosePortIsGettingDraggedFrom = nullptr;
	bool m_isItTheTopPort = false;

	//For node dragging
	bool m_isNodeGrabbed = false;
	std::vector<Vec2> m_offsetFromGrabbedPosToNodeStartPos;
	std::vector<Vec2> m_grabbedNodeOriginalPos;
	//For various purposes (dragging, deleting)
	std::vector<BehaviorTreeNode*> m_selectedNodes;

	std::map<std::string, const BehaviorTreeNode*> m_nameAndPrototypeTreeNodeMap;

	std::queue<Command*> m_commandsToBeExecutedQueue;

	Overlay* m_overlay = nullptr;
	Button* m_importButton = nullptr;
	Button* m_exportButton = nullptr;
	Button* m_playButton = nullptr;
	VariablesBox* m_variablesBox = nullptr;
	PopupPlayer* m_popupPlayer = nullptr;

	bool m_isTicking = false;

	VisualNovelManager* m_vnManager = nullptr;

	bool m_isMultiSelecting = false;
	//const float m_multiSelectBeginThreshold = 5.0f;
	//bool m_startMultiSelectQuery = false;
	Vec2 m_multiSelectStartPoint;
	AABB2 m_multiSelectBox;
	const float m_multiSelectBoxThickness = 1.0f;
};