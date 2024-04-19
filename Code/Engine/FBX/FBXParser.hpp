#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Fbx/Vertex_FBX.hpp"
#include "Engine/Fbx/FBXPose.hpp"
#include "ThirdParty/fbxsdk/fbxsdk.h"
#include <string>
#include <vector>
#include <map>

class Renderer;
class FBXModel;
class FBXModelConfig;
class FBXJoint;
class FBXMesh;
struct Face;
struct HalfEdge;
struct FBXControlPoint;

struct FBXParserConfig
{
	FBXParserConfig(Renderer& rendererForVBOs) : m_renderer(rendererForVBOs) {};
	Renderer& m_renderer;
};

class FBXParser {
public:
	FBXParser(const FBXParserConfig& config);
	~FBXParser();

public:
	void ParseFile(const std::string& fbxFilePath);
	FBXModel* CreateFBXModelWithOwnership(const FBXModelConfig& modelConfig);
	bool ExportParsedFile(const std::string& filePath);
	FbxScene* GetScene() const;
	bool LoadCompatibleAnimationDataToModel(FBXModel& model, const std::string& filePath, std::string* errorStr = nullptr);

private:
	bool StartProcessingSkeletonHierarchy(FbxNode& rootNode, std::vector<FBXJoint*>& out_jointsArray);
	void RecursivelyProcessSkeletonHierarchy(FbxNode& pNode, FBXJoint& parentJoint, unsigned int& stencilRef, std::vector<FBXJoint*>& out_jointsArray);
	void RecursivelyProcessMeshOfNode(FbxNode& pNode, int& nodeIdx, const std::vector<FBXJoint*>& in_joints, std::vector<FBXMesh*>& out_meshes);
	void RecursivelyProcessMeshOfNodeAnimOnly(FbxNode& pNode, int& nodeIdx, const std::vector<FBXJoint*>& in_joints, std::vector<FBXMesh*>& out_meshes);
	//void RecursivelyProcessTextureOfNode(const FbxNode& pNode);	//This is a WRONG function (assumes a single mesh uses a single texture, which is NOT the case for some fbx. Do other tasks first and fix this later)

	//For copying rig data to another class
	//std::vector<FBXJoint*> GetDeepCopiedRig(FBXModel& modelOfRig) const;
	//void RecursivelyDeepCopyJointDataAndHierarchy(FBXModel& modelOfRig, std::vector<FBXJoint*>& deepCopiedRig, int& jointToCopyIndex, FBXJoint& copyJoint) const;

	/* Debug functions */
	void RecursivelyPrintNode(const FbxNode& pNode);
	void PrintAttribute(const FbxNodeAttribute& pNodeAttribute) const;
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type) const;
	void PrintTabs() const;

private:
	FBXParserConfig m_config;

	int m_numTabsToPrint = 0;
	std::vector<FBXJoint*> m_joints;

	FbxManager* m_fbxManager = nullptr;
	FbxScene* m_scene = nullptr;

	FbxExporter* m_fbxExporter = nullptr;

	std::vector<FBXMesh*> m_meshes;

	std::string m_latestParsedFileName;

	FbxTime::EMode m_animTimeMode = FbxTime::EMode::eFrames30;
	float m_animStartTime = 0.0f;
	float m_animEndTime = 0.0f;
	std::vector<FBXPose> m_poseSequence;
};