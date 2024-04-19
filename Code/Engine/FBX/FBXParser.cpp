#include "Engine/Fbx/FBXParser.hpp"
#include "Engine/Fbx/FBXJoint.hpp"
#include "Engine/Fbx/FBXModel.hpp"
#include "Engine/Fbx/FBXMesh.hpp"
#include "Engine/Fbx/FBXUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/GPUMesh.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

#pragma comment( lib, "ThirdParty/fbxsdk/libfbxsdk.lib" )

FBXParser::FBXParser(const FBXParserConfig& config) : m_config(config)
{
}

FBXParser::~FBXParser()
{
	if (m_scene)
		m_scene->Destroy();
	if (m_fbxExporter)
		m_fbxExporter->Destroy();
	if (m_fbxManager)
		m_fbxManager->Destroy();
	for (int i = 0; i < m_joints.size(); i++) {
		delete m_joints[i];
		m_joints[i] = nullptr;
	}
}

void FBXParser::ParseFile(const std::string& fbxFilePath)
{
	m_fbxManager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(m_fbxManager, IOSROOT);
	m_fbxManager->SetIOSettings(ios);

	FbxImporter* fbxImporter = FbxImporter::Create(m_fbxManager, "importer");
	if (!fbxImporter->Initialize(fbxFilePath.c_str(), -1, m_fbxManager->GetIOSettings())) {
		ERROR_AND_DIE(Stringf("Failed to initialize file %s. Error: %s", fbxFilePath.c_str(), fbxImporter->GetStatus().GetErrorString()));
	}

	m_scene = FbxScene::Create(m_fbxManager, "scene");
	if (!fbxImporter->Import(m_scene)) {
		ERROR_AND_DIE(Stringf("Failed to import fbx file %s to a scene", fbxFilePath.c_str()));
	}
	fbxImporter->Destroy();

	FbxAxisSystem::MayaZUp.ConvertScene(m_scene);

	//Triangulate the scene if necessary
	FbxGeometryConverter geometryConverter(m_fbxManager);
	geometryConverter.Triangulate(m_scene, true);

	FbxNode* rootNode = m_scene->GetRootNode();
	if (rootNode) {
		if (!StartProcessingSkeletonHierarchy(*rootNode, m_joints)) {
			ERROR_AND_DIE("FBX File doesn't contain any skeleton!");
		}
		RecursivelyPrintNode(*rootNode);
		int nodeIdx = 0;

		//Get Animation Data (puttint it here cause I need m_joints[0])
		FbxAnimStack* currAnimStack = m_scene->GetCurrentAnimationStack();	//No animation for this file
		if (currAnimStack) {
			FbxString animStackName = currAnimStack->GetName();
			FbxTakeInfo* takeInfo = m_scene->GetTakeInfo(animStackName);
			FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
			FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
			m_animStartTime = (float)start.GetSecondDouble();
			m_animEndTime = (float)end.GetSecondDouble();
			DebuggerPrintf("Animation Interval of animStack %s: %.2f to %.2f\n", currAnimStack->GetName(), m_animStartTime, m_animEndTime);
			m_animTimeMode = start.GetGlobalTimeMode();	//It's 30 frames per second

			FbxLongLong startFrameIndex = start.GetFrameCount(m_animTimeMode);
			FbxLongLong endFrameIndex = end.GetFrameCount(m_animTimeMode);
			GUARANTEE_OR_DIE(startFrameIndex <= endFrameIndex, "startFrameIndex > endFrameIndex!");
			FbxLongLong numFrames = endFrameIndex - startFrameIndex + 1;

			m_poseSequence.resize(numFrames);
			for (int i = 0; i < m_poseSequence.size(); i++) {
				m_poseSequence[i].SetRootJoint(*m_joints[0]);
			}
		}

		RecursivelyProcessMeshOfNode(*rootNode, nodeIdx, m_joints, m_meshes);
		//RecursivelyProcessTextureOfNode(*rootNode);
	}

	/*
	FbxNode* node = m_scene->GetNode(m_meshes[0]->m_nodeIdx);
	FbxMesh* mesh = node->GetMesh();
	assert(mesh != nullptr);
	*/

	m_latestParsedFileName = fbxFilePath;
}

FBXModel* FBXParser::CreateFBXModelWithOwnership(const FBXModelConfig& modelConfig)
{
	FBXModel* newModel = new FBXModel(modelConfig, m_latestParsedFileName);

	newModel->SetMeshesAndJointsFromParser(m_meshes, m_joints);
	newModel->m_animManager = new FBXAnimManager(*newModel, modelConfig.m_parentClock);
	newModel->m_animManager->SetAnimDataFromParser(m_animStartTime, m_animEndTime, m_animTimeMode, m_poseSequence);

	//Ownership is transferred
	m_meshes.clear();
	m_joints.clear();

	newModel->Startup();
	return newModel;
}

bool FBXParser::ExportParsedFile(const std::string& filePath)
{
	m_fbxExporter = FbxExporter::Create(m_fbxManager, "exporter");
	//const char* exportFilePath = "testExport.fbx";
	m_fbxManager->GetIOSettings()->SetBoolProp(EXP_FBX_EMBEDDED, true);
	//int lFileFormat = m_fbxManager->GetIOPluginRegistry()->GetNativeWriterFormat();
	if (!m_fbxExporter->Initialize(filePath.c_str(), -1, m_fbxManager->GetIOSettings())) {
		ERROR_AND_DIE(Stringf("Failed to initialize file %s. Error: %s", filePath.c_str(), m_fbxExporter->GetStatus().GetErrorString()));
	}
	if (!m_fbxExporter->Export(m_scene)) {
		ERROR_AND_DIE(Stringf("Failed to export file %s. Error: %s", filePath.c_str(), m_fbxExporter->GetStatus().GetErrorString()));
	}
	return false;
}

FbxScene* FBXParser::GetScene() const
{
	return m_scene;
}

bool FBXParser::LoadCompatibleAnimationDataToModel(FBXModel& model, const std::string& fbxFilePath, std::string* errorStr)
{
	FbxImporter* fbxImporter = FbxImporter::Create(m_fbxManager, "importer");
	if (!fbxImporter->Initialize(fbxFilePath.c_str(), -1, m_fbxManager->GetIOSettings())) {
		ERROR_AND_DIE(Stringf("Failed to initialize file %s. Error: %s", fbxFilePath.c_str(), fbxImporter->GetStatus().GetErrorString()));
	}

	FbxScene* scene = FbxScene::Create(m_fbxManager, "scene");
	if (!fbxImporter->Import(scene)) {
		ERROR_AND_DIE(Stringf("Failed to import fbx file %s to a scene", fbxFilePath.c_str()));
	}
	fbxImporter->Destroy();

	FbxAxisSystem::MayaZUp.ConvertScene(scene);

	FbxNode* rootNode = scene->GetRootNode();

	if (rootNode) {	
		std::vector<FBXJoint*> tempJoints;
		std::vector<FBXMesh*> tempMeshes;
		if (!StartProcessingSkeletonHierarchy(*rootNode, tempJoints)) {
			ERROR_AND_DIE("FBX File doesn't contain any skeleton!");
		}

		//Get Animation Data (puttint it here cause I need m_joints[0])
		FbxAnimStack* currAnimStack = scene->GetCurrentAnimationStack();	//No animation for this file
		if (currAnimStack) {
			FbxString animStackName = currAnimStack->GetName();
			FbxTakeInfo* takeInfo = scene->GetTakeInfo(animStackName);
			FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
			FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
			m_animStartTime = (float)start.GetSecondDouble();
			m_animEndTime = (float)end.GetSecondDouble();
			DebuggerPrintf("Animation Interval of animStack %s: %.2f to %.2f\n", currAnimStack->GetName(), m_animStartTime, m_animEndTime);
			m_animTimeMode = start.GetGlobalTimeMode();	//It's 30 frames per second

			FbxLongLong startFrameIndex = start.GetFrameCount(m_animTimeMode);
			FbxLongLong endFrameIndex = end.GetFrameCount(m_animTimeMode);
			GUARANTEE_OR_DIE(startFrameIndex <= endFrameIndex, "startFrameIndex > endFrameIndex!");
			FbxLongLong numFrames = endFrameIndex - startFrameIndex + 1;

			m_poseSequence.resize(numFrames);
		}

		bool returnVal = false;
		if (AreSkeletonHierarchiesIdentical(*model.GetJointsArray()[0], *tempJoints[0])) {
			//Load animation data here
			int nodeIdx = 0;
			RecursivelyProcessMeshOfNodeAnimOnly(*rootNode, nodeIdx, tempJoints, tempMeshes);
			//model.UpdateJointsKeyFrameAnimation(tempJoints);

			for (int i = 0; i < m_poseSequence.size(); i++) {
				m_poseSequence[i].SetRootJoint(model.GetRootJoint());
			}

			model.m_animManager->SetAnimDataFromParser(m_animStartTime, m_animEndTime, m_animTimeMode, m_poseSequence);

			returnVal = true;
		}
		else {
			if (errorStr) {
				*errorStr = Stringf("model has %d joints whereas new file has %d joints", model.GetNumJoints(), tempJoints.size());
			}
		}

		//Remove garbage memory
		for (int i = 0; i < tempJoints.size(); i++) {
			delete tempJoints[i];
		}
		for (int i = 0; i < tempMeshes.size(); i++) {
			delete tempMeshes[i];
		}

		return returnVal;
	}
	else {
		ERROR_AND_DIE("Scene doesn't have a root node? Check fbx file");
	}
}

//Assumes there will be only one skeleton per file! (which is fine for my thesis)
bool FBXParser::StartProcessingSkeletonHierarchy(FbxNode& rootNode, std::vector<FBXJoint*>& out_jointsArray) {
	GUARANTEE_OR_DIE(out_jointsArray.size() == 0, "out_jointsArray should be empty when calling StartProcessingSkeletonHierarchy!");

	const FbxNodeAttribute* nodeAttribute = rootNode.GetNodeAttribute();
	if (nodeAttribute && nodeAttribute->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
		FBXJoint* rootJoint = new FBXJoint;
		unsigned int stencilRef = 1;
		rootJoint->SetStencilRefForThisJoint(stencilRef);
		rootJoint->SetIsRoot(true);
		rootJoint->SetName(rootNode.GetName());
		auto fbxMat = rootNode.EvaluateGlobalTransform();
		auto fbxT = fbxMat.GetT();
		auto fbxQ = fbxMat.GetQ();
		rootJoint->SetOriginalLocalTranslate(fbxT);
		rootJoint->SetOriginalLocalRotate(fbxQ);
		out_jointsArray.push_back(rootJoint);
		for (int i = 0; i < rootNode.GetChildCount(); i++) {
			if (rootNode.GetChild(i)) {
				RecursivelyProcessSkeletonHierarchy(*rootNode.GetChild(i), *rootJoint, ++stencilRef, out_jointsArray);
			}
		}
		return true;
	}
	else {
		for (int i = 0; i < rootNode.GetChildCount(); i++) {
			if (rootNode.GetChild(i)) {
				bool startedProcessingSkeleton = StartProcessingSkeletonHierarchy(*rootNode.GetChild(i), out_jointsArray);
				if (startedProcessingSkeleton) {
					return true;
				}
			}
		}
	}
	return false;
}

void FBXParser::RecursivelyProcessSkeletonHierarchy(FbxNode& currentNode, FBXJoint& parentJoint, unsigned int& stencilRef, std::vector<FBXJoint*>& out_jointsArray)
{
	const FbxNodeAttribute* nodeAttribute = currentNode.GetNodeAttribute();
	if (nodeAttribute && nodeAttribute->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
		FBXJoint* newChildJoint = new FBXJoint;
		newChildJoint->SetName(currentNode.GetName());
		auto fbxLocalTransform = currentNode.EvaluateLocalTransform();
		auto fbxT = fbxLocalTransform.GetT();
		auto fbxQ = fbxLocalTransform.GetQ();
		newChildJoint->SetOriginalLocalTranslate(fbxT);
		newChildJoint->SetOriginalLocalRotate(fbxQ);
		newChildJoint->SetStencilRefForThisJoint(stencilRef);
		out_jointsArray.push_back(newChildJoint);
		parentJoint.AddChildJoints(*newChildJoint);

		int childrenCount = currentNode.GetChildCount();
		if (childrenCount == 0) {
			newChildJoint->SetIsEndJoint(true);
			return;
		}
		for (int i = 0; i < childrenCount; i++) {
			if (currentNode.GetChild(i)) {
				RecursivelyProcessSkeletonHierarchy(*currentNode.GetChild(i), *newChildJoint, ++stencilRef, out_jointsArray);
			}
		}
	}
	else {
		for (int i = 0; i < currentNode.GetChildCount(); i++) {
			if (currentNode.GetChild(i)) {
				RecursivelyProcessSkeletonHierarchy(*currentNode.GetChild(i), parentJoint, stencilRef, out_jointsArray);
			}
		}
	}
}

void FBXParser::RecursivelyProcessMeshOfNode(FbxNode& pNode, int& nodeIdx, const std::vector<FBXJoint*>& in_joints, std::vector<FBXMesh*>& out_meshes)
{
	FbxMesh * currMesh = pNode.GetMesh();
	if (currMesh == nullptr) {
		for (int i = 0; i < pNode.GetChildCount(); i++) {
			if (pNode.GetChild(i)) {
				nodeIdx++;
				RecursivelyProcessMeshOfNode(*pNode.GetChild(i), nodeIdx, in_joints, out_meshes);
			}
		}
		return;
	}

	FBXMesh* newMesh = new FBXMesh(*this, pNode.GetName(), nodeIdx);
	out_meshes.push_back(newMesh);

	newMesh->ProcessFbxMesh(pNode, *currMesh, in_joints, m_poseSequence, *m_scene);

	for (int i = 0; i < pNode.GetChildCount(); i++) {
		if (pNode.GetChild(i)) {
			nodeIdx++;
			RecursivelyProcessMeshOfNode(*pNode.GetChild(i), nodeIdx, in_joints, out_meshes);
		}
	}
	return;
}

void FBXParser::RecursivelyProcessMeshOfNodeAnimOnly(FbxNode& pNode, int& nodeIdx, const std::vector<FBXJoint*>& in_joints, std::vector<FBXMesh*>& out_meshes)
{
	FbxMesh* currMesh = pNode.GetMesh();
	if (currMesh == nullptr) {
		for (int i = 0; i < pNode.GetChildCount(); i++) {
			if (pNode.GetChild(i)) {
				nodeIdx++;
				RecursivelyProcessMeshOfNodeAnimOnly(*pNode.GetChild(i), nodeIdx, in_joints, out_meshes);
			}
		}
		return;
	}

	FBXMesh* newMesh = new FBXMesh(*this, pNode.GetName(), nodeIdx);
	out_meshes.push_back(newMesh);
	newMesh->ProcessKeyAnimOfMesh(*currMesh, in_joints, m_poseSequence, *pNode.GetScene());

	for (int i = 0; i < pNode.GetChildCount(); i++) {
		if (pNode.GetChild(i)) {
			nodeIdx++;
			RecursivelyProcessMeshOfNodeAnimOnly(*pNode.GetChild(i), nodeIdx, in_joints, out_meshes);
		}
	}
	return;
}

void FBXParser::RecursivelyPrintNode(const FbxNode& pNode)
{
	PrintTabs();
	const char* nodeName = pNode.GetName();
	FbxDouble3 translation = pNode.LclTranslation.Get();
	FbxDouble3 rotation = pNode.LclRotation.Get();
	FbxDouble3 scaling = pNode.LclScaling.Get();

	// Print the contents of the node.
	DebuggerPrintf("<node name='%s' translation='(%f, %f, %f)' rotation='(%f, %f, %f)' scaling='(%f, %f, %f)'>\n",
		nodeName,
		translation[0], translation[1], translation[2],
		rotation[0], rotation[1], rotation[2],
		scaling[0], scaling[1], scaling[2]
	);
	m_numTabsToPrint++;

	// Print the node's attributes.
	for (int i = 0; i < pNode.GetNodeAttributeCount(); i++) {
		if(pNode.GetNodeAttributeByIndex(i))
			PrintAttribute(*pNode.GetNodeAttributeByIndex(i));
	}

	// Recursively print the children.
	for (int j = 0; j < pNode.GetChildCount(); j++) {
		if(pNode.GetChild(j))
			RecursivelyPrintNode(*pNode.GetChild(j));
	}

	m_numTabsToPrint--;
	PrintTabs();
	DebuggerPrintf("</node>\n");
}

void FBXParser::PrintAttribute(const FbxNodeAttribute& pAttribute) const
{
	FbxString typeName = GetAttributeTypeName(pAttribute.GetAttributeType());
	FbxString attrName = pAttribute.GetName();
	PrintTabs();
	// Note: to retrieve the character array of a FbxString, use its Buffer() method.
	DebuggerPrintf("<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
}

FbxString FBXParser::GetAttributeTypeName(FbxNodeAttribute::EType type) const
{
	switch (type) {
	case FbxNodeAttribute::eUnknown: return "unidentified";
	case FbxNodeAttribute::eNull: return "null";
	case FbxNodeAttribute::eMarker: return "marker";
	case FbxNodeAttribute::eSkeleton: return "skeleton";
	case FbxNodeAttribute::eMesh: return "mesh";
	case FbxNodeAttribute::eNurbs: return "nurbs";
	case FbxNodeAttribute::ePatch: return "patch";
	case FbxNodeAttribute::eCamera: return "camera";
	case FbxNodeAttribute::eCameraStereo: return "stereo";
	case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
	case FbxNodeAttribute::eLight: return "light";
	case FbxNodeAttribute::eOpticalReference: return "optical reference";
	case FbxNodeAttribute::eOpticalMarker: return "marker";
	case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
	case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
	case FbxNodeAttribute::eBoundary: return "boundary";
	case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
	case FbxNodeAttribute::eShape: return "shape";
	case FbxNodeAttribute::eLODGroup: return "lodgroup";
	case FbxNodeAttribute::eSubDiv: return "subdiv";
	default: return "unknown";
	}
}

void FBXParser::PrintTabs() const
{
	for (int i = 0; i < m_numTabsToPrint; i++) {
		DebuggerPrintf("\t");
	}
}

/*
std::vector<FBXJoint*> FBXParser::GetDeepCopiedRig(FBXModel& modelOfRig) const
{
	std::vector<FBXJoint*> deepCopiedRig;

	FBXJoint* copiedRootJoint = new FBXJoint;
	int jointToCopyIndex = 0;
	RecursivelyDeepCopyJointDataAndHierarchy(modelOfRig, deepCopiedRig, jointToCopyIndex, *copiedRootJoint);

	return deepCopiedRig;
}

void FBXParser::RecursivelyDeepCopyJointDataAndHierarchy(FBXModel& modelOfRig, std::vector<FBXJoint*>& deepCopiedRig, int& jointToCopyIndex, FBXJoint& copyJoint) const {

	if (m_joints[jointToCopyIndex] == nullptr) {
		ERROR_AND_DIE("FBXParser::m_joints should not have a nullptr element!");
	}
	deepCopiedRig.push_back(&copyJoint);

	//Copy all the data except child joint data
	copyJoint = *m_joints[jointToCopyIndex];
	copyJoint.ClearChildJoints();

	//Also set the model
	copyJoint.SetFBXModel(modelOfRig);
	
	int numChildren = m_joints[jointToCopyIndex]->GetNumChildJoints();
	for (int i = 0; i < numChildren; i++) {
		FBXJoint* copiedChildJoint = new FBXJoint;

		copyJoint.AddChildJoints(*copiedChildJoint);

		jointToCopyIndex++;
		RecursivelyDeepCopyJointDataAndHierarchy(modelOfRig, deepCopiedRig, jointToCopyIndex, *copiedChildJoint);
	}
}
*/