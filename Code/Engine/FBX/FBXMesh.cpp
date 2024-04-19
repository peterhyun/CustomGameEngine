#include "Engine/Fbx/FBXMesh.hpp"
#include "Engine/Fbx/FBXUtils.hpp"
#include "Engine/Fbx/FBXJoint.hpp"
#include "Engine/Fbx/FBXDDMModifierCPU.hpp"
#include "Engine/Fbx/FBXDDMModifierGPU.hpp"
#include "Engine/Fbx/FBXParser.hpp"
#include "Engine/FBX/FBXAnimManager.hpp"
/*
#include "Engine/Mesh/Face.hpp"
#include "Engine/Mesh/HalfEdge.hpp"
#include "Engine/Mesh/Edge.hpp"
#include "Engine/Mesh/MeshOperationUtils.hpp"
*/
#include "Engine/Core/GPUMesh.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include <map>

const int FBXMesh::MAXTEXTURENUM = 3;

FBXMesh::FBXMesh(FBXParser& creatorParser, const std::string& name, int nodeIdx) : m_creatorParser(creatorParser), m_name(name), m_nodeIdx(nodeIdx)
{
}

FBXMesh::~FBXMesh()
{
	for (int i = 0; i < m_controlPointsRestPose.size() ; i++) {
		if (m_controlPointsRestPose[i]) {
			delete m_controlPointsRestPose[i];
		}
	}
	delete m_fbxMeshCBO;
	delete m_debugGPUMesh;
	delete m_gpuMesh;
}

void FBXMesh::ProcessFbxMesh(FbxNode& node, FbxMesh& mesh, const std::vector<FBXJoint*>& joints, std::vector<FBXPose>& inout_poseSequence, FbxScene& scene)
{
	//Store the coordinate change of mesh (Remove the translation) 
	FbxAMatrix meshGlobalTransform = node.EvaluateGlobalTransform();
	Mat44 meshGlobalTransformGH = ConvertFbxAMatrixToMat44(meshGlobalTransform);
	meshGlobalTransformGH.m_values[Mat44::Tx] = 0.0f;
	meshGlobalTransformGH.m_values[Mat44::Ty] = 0.0f;
	meshGlobalTransformGH.m_values[Mat44::Tz] = 0.0f;
	meshGlobalTransformGH.m_values[Mat44::Tw] = 1.0f;
	Mat44 fbxToGHCoordSysMatrixForMesh = meshGlobalTransformGH;

	//Store the control points in a long vector and also in matrix rows so that I can use it for laplacian matrix calculation
	int numControlPoints = mesh.GetControlPointsCount();
	m_controlPointsMatrixRestPose.resize(numControlPoints, Eigen::NoChange);

	for (int i = 0; i < numControlPoints; i++) {
		FbxVector4 controlPointPos = mesh.GetControlPointAt(i);
		Vec3 controlPointGHPos = fbxToGHCoordSysMatrixForMesh.TransformPosition3D(Vec3((float)controlPointPos.mData[0], (float)controlPointPos.mData[1], (float)controlPointPos.mData[2]));

		if (m_boundingBox.m_mins != Vec3() || m_boundingBox.m_maxs != Vec3()) {
			m_boundingBox.StretchToIncludePoint(controlPointGHPos);
		}
		else {
			m_boundingBox.m_mins = controlPointGHPos;
			m_boundingBox.m_maxs = controlPointGHPos;
		}
		m_controlPointsRestPose.push_back(new FBXControlPoint(controlPointGHPos));
		m_controlPointsMatrixRestPose.row(i) = Eigen::RowVector3d((double)controlPointGHPos.x, (double)controlPointGHPos.y, (double)controlPointGHPos.z);
	}

	//Process skinning data first cause you need to store them in control points when you make the vertices!
	ProcessSkinningDataOfMesh(mesh, joints);
	ProcessMaterialOfMesh(mesh);

	bool isMeshMappingModeAllTheSame = IsMeshMappingModeAllTheSame(mesh);
	FbxGeometryElementMaterial* materialElement = nullptr;
	if (isMeshMappingModeAllTheSame == false) {
		materialElement = mesh.GetElementMaterial();
		GUARANTEE_OR_DIE(materialElement != nullptr, "materialElement == nullptr!");
	}
	/*
	if (isMeshMappingModeAllTheSame) {
		// IDK what to do here for now
	}
	else {	//Per polygon texture mapping
		int polygonCount = mesh.GetPolygonCount();
		FbxGeometryElementMaterial* materialElement = mesh.GetElementMaterial();
		GUARANTEE_OR_DIE(materialElement != nullptr, "materialElement == nullptr!");
		for (int polygonIdx = 0; polygonIdx < polygonCount; polygonIdx++) {
			int materialIdx = materialElement->GetIndexArray().GetAt(polygonIdx);
			DebuggerPrintf("Mesh: %s - polygon %d / materialIdx %d\n", m_name.c_str(), polygonIdx, materialIdx);
		}
	}
	*/

	int polygonCount = mesh.GetPolygonCount();
	m_facesMatrix.resize(polygonCount, Eigen::NoChange);

	/*
	//Create normals, tangents if it doesn't exist
	if (mesh.GetElementNormalCount() < 1) {
		DebuggerPrintf("Mesh doesn't have a normal element!\n");
		if (!mesh.GenerateNormals(false, true, false)) {
			ERROR_RECOVERABLE("Normals data generation unsuccessful!");
		}
	}
	if (mesh.GetElementTangentCount() < 1) {
		DebuggerPrintf("Mesh doesn't have a tangent element, so generating them...\n");
		if (!mesh.GenerateTangentsData(0, false, false)) {
			ERROR_RECOVERABLE("Tangents data generation unsuccessful!");
		}
	}
	*/

	int vertexCounter = 0;
	m_renderVertexToControlPointMap.reserve(polygonCount * 3);
	//For each triangle...
	std::map<Vertex_FBX, unsigned int> verticesToIndicesMap;
	for (int triangleIndex = 0; triangleIndex < polygonCount; triangleIndex++) {	//For each face
		//Create half edge, face, and vertices
		int controlPointIndices[3] = {};
		int materialIdx = 0;
		if (isMeshMappingModeAllTheSame == false) {
			materialIdx = materialElement->GetIndexArray().GetAt(triangleIndex);
		}
		for (int i = 0; i < 3; i++) {	//A triangle has 3 verts (duh)
			//Creating vertices from all triangle face data
			controlPointIndices[i] = mesh.GetPolygonVertex(triangleIndex, i);
			CreateVertexAndPushToRenderDataArray(mesh, verticesToIndicesMap, vertexCounter, controlPointIndices[i], materialIdx);
			vertexCounter++;
		}
		m_facesMatrix.row(triangleIndex) = Eigen::RowVector3i(controlPointIndices[0], controlPointIndices[1], controlPointIndices[2]);
	}

	CalculateFBXAveragedNormals(m_renderVertices, m_renderIndices);
	CalculateFBXTangents(m_renderVertices, m_renderIndices);
	ProcessKeyAnimOfMesh(mesh, joints, inout_poseSequence, scene);
}

void FBXMesh::SetGPUData(Renderer& renderer)
{
	GUARANTEE_OR_DIE(m_diffuseTextures.size() == 0, "Calling FBXMesh::SetGPUData() twice!");
	for (int i = 0; i < m_diffuseTexturePaths.size(); i++) {
		if (DoesFileExistOnDisk(m_diffuseTexturePaths[i])) {
			m_diffuseTextures.push_back(renderer.CreateOrGetTextureFromFile(m_diffuseTexturePaths[i].c_str()));
		}
	}
	if (m_diffuseTextures.size() == 0) {
		m_diffuseTextures.push_back(renderer.CreateOrGetTextureFromFile("Data/Images/PureIvory.png"));
	}

	for (int i = 0; i < m_specularTexturePaths.size(); i++) {
		if (DoesFileExistOnDisk(m_specularTexturePaths[i])) {
			m_specularTextures.push_back(renderer.CreateOrGetTextureFromFile(m_specularTexturePaths[i].c_str()));
		}
	}

	for (int i = 0; i < m_normalTexturePaths.size(); i++) {
		if (DoesFileExistOnDisk(m_normalTexturePaths[i])) {
			m_normalTextures.push_back(renderer.CreateOrGetTextureFromFile(m_normalTexturePaths[i].c_str()));
		}
	}

	for (int i = 0; i < m_glossTexturePaths.size(); i++) {
		if (DoesFileExistOnDisk(m_glossTexturePaths[i])) {
			m_glossTextures.push_back(renderer.CreateOrGetTextureFromFile(m_glossTexturePaths[i].c_str()));
		}
	}

	for (int i = 0; i < m_ambientTexturePaths.size(); i++) {
		if (DoesFileExistOnDisk(m_ambientTexturePaths[i])) {
			m_ambientTextures.push_back(renderer.CreateOrGetTextureFromFile(m_ambientTexturePaths[i].c_str()));
		}
	}

	m_gpuMesh = new GPUMesh<Vertex_FBX>(GPUMeshConfig(renderer), m_renderVertices, m_renderIndices);

	std::vector<Vertex_PCU> meshDebugVerts;
	for (int i = 0; i < m_renderVertices.size(); i++) {
		meshDebugVerts.push_back(Vertex_PCU(m_renderVertices[i].m_position, Rgba8::RED));
		meshDebugVerts.push_back(Vertex_PCU(m_renderVertices[i].m_position + m_renderVertices[i].m_tangent, Rgba8::RED));

		meshDebugVerts.push_back(Vertex_PCU(m_renderVertices[i].m_position, Rgba8::GREEN));
		meshDebugVerts.push_back(Vertex_PCU(m_renderVertices[i].m_position + m_renderVertices[i].m_binormal, Rgba8::GREEN));

		meshDebugVerts.push_back(Vertex_PCU(m_renderVertices[i].m_position, Rgba8::BLUE));
		meshDebugVerts.push_back(Vertex_PCU(m_renderVertices[i].m_position + m_renderVertices[i].m_normal, Rgba8::BLUE));
	}
	m_debugGPUMesh = new GPUMesh<Vertex_PCU>(GPUMeshConfig(renderer), meshDebugVerts, false);

	m_fbxMeshCBO = renderer.CreateConstantBuffer(sizeof(m_fbxMeshConstants));
	m_fbxMeshConstants.FBXMeshHasNormalTexture = (int)m_normalTextures.size();
	m_fbxMeshConstants.FBXMeshHasSpecularTexture = (int)m_specularTextures.size();
	m_fbxMeshConstants.FBXMeshHasGlossTexture = (int)m_glossTextures.size();
	m_fbxMeshConstants.IsFBXMeshLBS = 1;
	renderer.CopyCPUToGPU(&m_fbxMeshConstants, m_fbxMeshCBO->m_size, m_fbxMeshCBO);
}

void FBXMesh::Render(Renderer& renderer) const
{
	if (m_gpuMesh == nullptr)
		return;

	renderer.SetModelConstants();
	renderer.SetBlendMode(BlendMode::OPAQUE);
	/*
	else if (skinningModifier == FBXModelSkinningModifier::DDM_GPU_v0 || skinningModifier == FBXModelSkinningModifier::DDM_GPU_v1) {
		renderer.BindShader(m_ddmModifierGPU->m_DDMGPUShader);
		renderer.BindStructuredBufferToVS(m_ddmModifierGPU->m_inputRenderVertsToCPsSBO, 1);
		renderer.BindComputeOutputBufferToVS(m_ddmModifierGPU->m_outputCPsCMBO, 2);
	}
	*/
	for (int i = 0; i < MAXTEXTURENUM; i++) {
		if (i < m_diffuseTextures.size())
			renderer.BindTexture(m_diffuseTextures[i], i);
		else
			renderer.BindTexture(nullptr, i);

		if (i < m_specularTextures.size())
			renderer.BindTexture(m_specularTextures[i], i + 3);
		else
			renderer.BindTexture(nullptr, i + 3);

		if (i < m_normalTextures.size())
			renderer.BindTexture(m_normalTextures[i], i + 6);
		else
			renderer.BindTexture(nullptr, i + 6);

		if (i < m_glossTextures.size())
			renderer.BindTexture(m_glossTextures[i], i + 9);
		else
			renderer.BindTexture(nullptr, i + 9);

		if (i < m_normalTextures.size())
			renderer.BindTexture(m_normalTextures[i], i + 12);
		else
			renderer.BindTexture(nullptr, i + 12);
	}

	renderer.BindConstantBuffer(m_fbxMeshCBO, k_fbxMeshConstantsSlot);
	//renderer.BindTexture(m_debugCheckeredTexture);
	m_gpuMesh->Render();

	if (m_isMeshDebugMode) {
		renderer.BindShader(nullptr);
		m_debugGPUMesh->Render();
	}

	/*
	//Unbind output buffer if done
	if (skinningModifier == FBXModelSkinningModifier::DDM_GPU_v0 || skinningModifier == FBXModelSkinningModifier::DDM_GPU_v1) {
		renderer.BindComputeOutputBufferToVS(nullptr, 2);
	}
	*/
}

void FBXMesh::SetFBXModel(FBXModel& model)
{
	m_model = &model;
}

void FBXMesh::AddDDMModifier()
{
	m_ddmModifierCPU = std::make_shared<FBXDDMModifierCPU>(*this);
	m_ddmModifierGPU = std::make_shared<FBXDDMModifierGPU>(*this);
}

std::shared_ptr<FBXDDMModifierCPU> FBXMesh::GetDDMModifierCPU() const
{
	return m_ddmModifierCPU;
}

std::shared_ptr<FBXDDMModifierGPU> FBXMesh::GetDDMModifierGPU() const
{
	return m_ddmModifierGPU;
}

void FBXMesh::PrecomputeDDM(bool isCPUSide, bool useCotangentLaplacian, int numLaplacianIterations, float lambda, float kappa, float alpha)
{
	if (isCPUSide) {
		if (m_ddmModifierCPU == nullptr)
			ERROR_AND_DIE("Cannot precompute ddm stuff when FBXMesh::m_ddmModifierCPU == nullptr");
		m_ddmModifierCPU->Precompute(useCotangentLaplacian, numLaplacianIterations, lambda, kappa, alpha);
	}
	else {
		if (m_ddmModifierGPU == nullptr)
			ERROR_AND_DIE("Cannot precompute ddm stuff when FBXMesh::m_ddmModifierGPU == nullptr");
		m_ddmModifierGPU->Precompute(useCotangentLaplacian, numLaplacianIterations, lambda, kappa, alpha);
	}
}

void FBXMesh::ApplyDDMv0_CPU(const std::vector<Mat44>& allJointSkinningMatrices)
{
	if (m_ddmModifierCPU == nullptr)
		ERROR_AND_DIE("Cannot compute ddm stuff when FBXMesh::m_ddmModifierCPU == nullptr");

	bool didRecalculateThisFrame = false;
	Eigen::MatrixX3f deformedControlPointsMatrix = m_ddmModifierCPU->GetVariantv0Deform(allJointSkinningMatrices, didRecalculateThisFrame);

	//Eigen::MatrixX3d dummyData = m_ddmModifierGPU->GetVariantv0Deform(allJointSkinningMatrices);
	if (didRecalculateThisFrame == false)
		return;

	for (int i = 0; i < m_renderVertexToControlPointMap.size(); i++) {
		auto newCtrlPoint = deformedControlPointsMatrix.row(m_renderVertexToControlPointMap[i]);
		m_renderVertices[i].m_position.x = newCtrlPoint[0];
		m_renderVertices[i].m_position.y = newCtrlPoint[1];
		m_renderVertices[i].m_position.z = newCtrlPoint[2];
	}

	m_gpuMesh->UpdateVerticesData(m_renderVertices);
}

void FBXMesh::ApplyDDMv1_CPU(const std::vector<Mat44>& allJointSkinningMatrices)
{
	if (m_ddmModifierCPU == nullptr)
		ERROR_AND_DIE("Cannot compute ddm stuff when FBXMesh::m_ddmModifierCPU == nullptr");

	bool didRecalculateThisFrame = false;
	Eigen::MatrixX3f deformedControlPointsMatrix = m_ddmModifierCPU->GetVariantv1Deform(allJointSkinningMatrices, didRecalculateThisFrame);
	
	if (didRecalculateThisFrame == false)
		return;

	for (int i = 0; i < m_renderVertexToControlPointMap.size(); i++) {
		auto newCtrlPoint = deformedControlPointsMatrix.row(m_renderVertexToControlPointMap[i]);
		m_renderVertices[i].m_position.x = newCtrlPoint[0];
		m_renderVertices[i].m_position.y = newCtrlPoint[1];
		m_renderVertices[i].m_position.z = newCtrlPoint[2];
	}

	m_gpuMesh->UpdateVerticesData(m_renderVertices);
}

void FBXMesh::ApplyDDMv0_GPU(const std::vector<Mat44>& allJointSkinningMatrices)
{
	if (m_ddmModifierGPU == nullptr)
		ERROR_AND_DIE("Cannot compute ddm stuff when FBXMesh::m_ddmModifierGPU == nullptr");

	bool didRecalculateThisFrame = false;
	Eigen::MatrixX3f deformedControlPointsMatrix = m_ddmModifierGPU->GetVariantv0Deform(allJointSkinningMatrices, didRecalculateThisFrame);

	if (didRecalculateThisFrame == false)
		return;

	for (int i = 0; i < m_renderVertexToControlPointMap.size(); i++) {
		auto newCtrlPoint = deformedControlPointsMatrix.row(m_renderVertexToControlPointMap[i]);
		m_renderVertices[i].m_position.x = newCtrlPoint[0];
		m_renderVertices[i].m_position.y = newCtrlPoint[1];
		m_renderVertices[i].m_position.z = newCtrlPoint[2];
	}

	m_gpuMesh->UpdateVerticesData(m_renderVertices);
}

void FBXMesh::ApplyDDMv1_GPU(const std::vector<Mat44>& allJointSkinningMatrices)
{
	if (m_ddmModifierGPU == nullptr)
		ERROR_AND_DIE("Cannot compute ddm stuff when FBXMesh::m_ddmModifierGPU == nullptr");

	bool didRecalculateThisFrame = false;
	Eigen::MatrixX3f deformedControlPointsMatrix = m_ddmModifierGPU->GetVariantv1Deform(allJointSkinningMatrices, didRecalculateThisFrame);

	if (didRecalculateThisFrame == false)
		return;

	for (int i = 0; i < m_renderVertexToControlPointMap.size(); i++) {
		auto newCtrlPoint = deformedControlPointsMatrix.row(m_renderVertexToControlPointMap[i]);
		m_renderVertices[i].m_position.x = newCtrlPoint[0];
		m_renderVertices[i].m_position.y = newCtrlPoint[1];
		m_renderVertices[i].m_position.z = newCtrlPoint[2];
	}

	m_gpuMesh->UpdateVerticesData(m_renderVertices);
}

Eigen::MatrixX3f FBXMesh::GetDDMv0_GPU_Deformation(const std::vector<Mat44>& allJointSkinningMatrices)
{
	if (m_ddmModifierGPU == nullptr)
		ERROR_AND_DIE("Cannot compute ddm stuff when FBXMesh::m_ddmModifierGPU == nullptr");
	Eigen::MatrixX3f deformedControlPointsMatrix = m_ddmModifierGPU->GetVariantv0DeformAlwaysCalculated(allJointSkinningMatrices);
	return deformedControlPointsMatrix;
}

void FBXMesh::RestoreGPUVerticesToRestPose()
{
	for (int i = 0; i < m_renderVertexToControlPointMap.size(); i++) {
		auto newCtrlPoint = m_controlPointsRestPose[m_renderVertexToControlPointMap[i]];
		if (newCtrlPoint)
			m_renderVertices[i].m_position = newCtrlPoint->m_position;
		else
			ERROR_AND_DIE("There is a nullptr in m_controlPointsRestPose");
	}
	m_gpuMesh->UpdateVerticesData(m_renderVertices);
}

void FBXMesh::SetRigidBinding(bool isRigidBound)
{
	if (isRigidBound) {
		for (int i = 0; i < m_renderVertexToControlPointMap.size(); i++) {
			auto newCtrlPoint = m_controlPointsRestPose[m_renderVertexToControlPointMap[i]];
			if (newCtrlPoint) {
				JointWeightPair jwPair = GetMaxInfluenceJointWeightPair(newCtrlPoint->m_jointWeightPairs);
				m_renderVertices[i].m_jointIndices1.Clear();
				m_renderVertices[i].m_jointIndices2.Clear();
				m_renderVertices[i].m_jointWeights1.Clear();
				m_renderVertices[i].m_jointWeights2.Clear();
				m_renderVertices[i].m_jointIndices1[0] = jwPair.m_jointIndex;
				m_renderVertices[i].m_jointWeights1[0] = 1.0f;
			}
			else {
				ERROR_AND_DIE("There is a nullptr in m_controlPointsRestPose");
			}
		}
	}
	else {
		for (int i = 0; i < m_renderVertexToControlPointMap.size(); i++) {
			auto restCtrlPoint = m_controlPointsRestPose[m_renderVertexToControlPointMap[i]];
			if (restCtrlPoint) {
				const std::vector<JointWeightPair>& jointWeightPairs = restCtrlPoint->m_jointWeightPairs;
				for (int jwPairIdx = 0; jwPairIdx < jointWeightPairs.size(); jwPairIdx++) {
					if (jwPairIdx < 4) {
						m_renderVertices[i].m_jointIndices1[jwPairIdx] = jointWeightPairs[jwPairIdx].m_jointIndex;
						m_renderVertices[i].m_jointWeights1[jwPairIdx] = jointWeightPairs[jwPairIdx].m_weight;
					}
					else {
						int secondArraySetIdx = jwPairIdx - 4;
						m_renderVertices[i].m_jointIndices2[secondArraySetIdx] = jointWeightPairs[jwPairIdx].m_jointIndex;
						m_renderVertices[i].m_jointWeights2[secondArraySetIdx] = jointWeightPairs[jwPairIdx].m_weight;
					}
				}
			}
			else {
				ERROR_AND_DIE("There is a nullptr in m_controlPointsRestPose");
			}
		}
	}

	m_isRigidBinding = isRigidBound;
	m_gpuMesh->UpdateVerticesData(m_renderVertices);

	m_ddmModifierCPU->ResetIsPrecomputed();
	m_ddmModifierGPU->ResetIsPrecomputed();
}

Eigen::MatrixX3d FBXMesh::GetControlPointsMatrixRestPose() const
{
	return m_controlPointsMatrixRestPose;
}

Eigen::MatrixX3i FBXMesh::GetFacesMatrix() const
{
	return m_facesMatrix;
}

Eigen::MatrixXd FBXMesh::GetWeightsMatrix() const
{
	if (m_model == nullptr) {
		ERROR_AND_DIE("FBXMesh::m_model is nullptr but is calling FBXMesh::GetWeightsMatrix()!");
	}
	if ((size_t)m_controlPointsMatrixRestPose.rows() != m_controlPointsRestPose.size()) {
		ERROR_AND_DIE("FBXMesh fucked up because m_controlPointsMatrixRestPose.rows() != m_controlPointsRestPose.size()");
	}
	Eigen::MatrixXd weightsMatrix;
	int numJoints = m_model->GetNumJoints();
	weightsMatrix.resize(m_controlPointsMatrixRestPose.rows(), numJoints);
	weightsMatrix.setZero();

	if (m_isRigidBinding == false) {
		for (int rowIdx = 0; rowIdx < m_controlPointsMatrixRestPose.rows(); rowIdx++) {
			const FBXControlPoint& currentCtrlPoint = *m_controlPointsRestPose[rowIdx];
			for (const auto& jointWeightPair : currentCtrlPoint.m_jointWeightPairs) {
				if (jointWeightPair.m_jointIndex >= (unsigned int)numJoints) {
					ERROR_AND_DIE("Sth fucked up in your ctrl point settings");
				}
				weightsMatrix(rowIdx, jointWeightPair.m_jointIndex) = jointWeightPair.m_weight;
			}
		}
	}
	else {
		for (int rowIdx = 0; rowIdx < m_controlPointsMatrixRestPose.rows(); rowIdx++) {
			const FBXControlPoint& currentCtrlPoint = *m_controlPointsRestPose[rowIdx];
			JointWeightPair jwPair = GetMaxInfluenceJointWeightPair(currentCtrlPoint.m_jointWeightPairs);
			weightsMatrix(rowIdx, jwPair.m_jointIndex) = jwPair.m_weight;
		}
	}

	return weightsMatrix;
}

int FBXMesh::GetNumVertices() const
{
	return (int)m_renderVertices.size();
}

int FBXMesh::GetNumJoints() const
{
	if (m_model) {
		return m_model->GetNumJoints();
	}
	else {
		ERROR_AND_DIE("Cannot call FBXMesh::GetNumJoints() when FBXMesh::m_model == nullptr");
	}
}

std::vector<unsigned int> FBXMesh::GetRenderVertexToControlPointMap() const
{
	return m_renderVertexToControlPointMap;
}

void FBXMesh::ToggleMeshDebugMode()
{
	m_isMeshDebugMode = !m_isMeshDebugMode;
}

void FBXMesh::PrepareDDMBaker(int numPoses)
{
	m_ddmBakerLHSMatrices.clear();
	m_ddmBakerRHSMatrices.clear();
	GUARANTEE_OR_DIE(numPoses > 0, "numPoses <= 0");
	int numControlPoints = (int)m_controlPointsRestPose.size();
	int numJoints = m_model->GetNumJoints();
	for (int i = 0; i < numControlPoints; i++) {
		Eigen::MatrixXf newLHSMat;
		newLHSMat.resize(3 * numPoses, numJoints);
		newLHSMat.setZero();
		m_ddmBakerLHSMatrices.push_back(newLHSMat);

		Eigen::MatrixXf newRHSMat;
		newRHSMat.resize(3 * numPoses, 1);
		newRHSMat.setZero();
		m_ddmBakerRHSMatrices.push_back(newRHSMat);
	}
}

void FBXMesh::FillLHSMatForDDMBaker(const std::vector<Mat44>& allJointSkinningMatrices, int poseIdx)
{
	int jointNum = m_model->GetNumJoints();
	GUARANTEE_OR_DIE(jointNum == (int)allJointSkinningMatrices.size(), "jointNum != allJointSkinningMatrices.size()");
	int numControlPoints = (int)m_controlPointsRestPose.size();
	GUARANTEE_OR_DIE(numControlPoints == (int)m_ddmBakerLHSMatrices.size(), "Check m_ddmBakerLHSMatrices.size()");

	for (int cpIdx = 0; cpIdx < numControlPoints; cpIdx++) {
		const FBXControlPoint* cp = m_controlPointsRestPose[cpIdx];
		GUARANTEE_OR_DIE(cp != nullptr, "cp == nullptr");
		for (int jointIdx = 0; jointIdx < jointNum; jointIdx++) {
			Vec3 transformedPos = allJointSkinningMatrices[jointIdx].TransformPosition3D(cp->m_position);
			m_ddmBakerLHSMatrices[cpIdx].block(poseIdx * 3, jointIdx, 3, 1) << transformedPos.x, transformedPos.y, transformedPos.z;
		}
	}
}

void FBXMesh::FillRHSMatForDDMBaker(const std::vector<Mat44>& allJointSkinningMatrices, int poseIdx)
{
	int jointNum = m_model->GetNumJoints();
	GUARANTEE_OR_DIE(jointNum == (int)allJointSkinningMatrices.size(), "jointNum != allJointSkinningMatrices.size()");
	int numControlPoints = (int)m_controlPointsRestPose.size();
	GUARANTEE_OR_DIE(numControlPoints == (int)m_ddmBakerRHSMatrices.size(), "Check m_ddmBakerRHSMatrices.size()");

	Eigen::MatrixX3f deformedCPsMat = GetDDMv0_GPU_Deformation(allJointSkinningMatrices);
	
	for (int cpIdx = 0; cpIdx < numControlPoints; cpIdx++) {
		m_ddmBakerRHSMatrices[cpIdx].block(3 * poseIdx, 0, 3, 1) << deformedCPsMat(cpIdx, 0), deformedCPsMat(cpIdx, 1), deformedCPsMat(cpIdx, 2);
	}
}

void FBXMesh::SolveDDMBakerLinearSystems(int numMaxBones, float pruneThreshold)
{
	GUARANTEE_OR_DIE(numMaxBones > 0, "numMaxBones <= 0");
	GUARANTEE_OR_DIE(pruneThreshold >= 0.0f, "pruneThreshold < 0");

	m_ddmBakerWeightsForCPs.clear();
	m_ddmBakerCPWeightPairsForJoints.clear();

	int numCPs = (int)m_controlPointsRestPose.size();
	for (int cpIdx = 0; cpIdx < numCPs; cpIdx++) {
		Eigen::MatrixXf finalWeightsForThisCP = m_ddmBakerLHSMatrices[cpIdx].colPivHouseholderQr().solve(m_ddmBakerRHSMatrices[cpIdx]);

		if (numMaxBones < m_model->GetNumJoints()) {
			//For the finalWeightsForThisCP vector, remove the weights that are not the numMaxBones
			std::vector<float> weightsArray(finalWeightsForThisCP.data(), finalWeightsForThisCP.data() + finalWeightsForThisCP.size());
			std::partial_sort(weightsArray.begin(), weightsArray.begin() + numMaxBones, weightsArray.end(), std::greater<float>());

			for (int i = 0; i < finalWeightsForThisCP.rows(); i++) {
				if (std::find(weightsArray.begin(), weightsArray.begin() + numMaxBones, finalWeightsForThisCP(i, 0)) == weightsArray.begin() + numMaxBones) {
					finalWeightsForThisCP(i, 0) = 0.0f;
				}
			}
		}

		for (int i = 0; i < finalWeightsForThisCP.rows(); i++) {
			//Prune the threshold values
			if (finalWeightsForThisCP(i, 0) <= pruneThreshold) {
				finalWeightsForThisCP(i, 0) = 0.0f;
			}
		}

		//Make the sum to 1.0
		float sum = finalWeightsForThisCP.sum();
		if (sum > 0.0f) {
			finalWeightsForThisCP /= sum;
		}
		m_ddmBakerWeightsForCPs.push_back(finalWeightsForThisCP);
	}

	int numJoints = m_model->GetNumJoints();
	m_ddmBakerCPWeightPairsForJoints.resize((size_t)numJoints);

	//Reorganize it so that each joint stores a pair of control point index and a weight
	for (int cpIdx = 0; cpIdx < numCPs; cpIdx++) {
		GUARANTEE_OR_DIE(numJoints == (int)m_ddmBakerWeightsForCPs[cpIdx].rows(), "numJoints != (int)m_ddmBakerWeightsForCPs[cpIdx].rows()");
		GUARANTEE_OR_DIE((int)m_ddmBakerWeightsForCPs[cpIdx].cols() == 1, "(int)m_ddmBakerWeightsForCPs[cpIdx].cols() != 1");
		for (int jointIdx = 0; jointIdx < numJoints; jointIdx++) {
			float weightVal = m_ddmBakerWeightsForCPs[cpIdx](jointIdx, 0);
			if (weightVal > 0.0f) {
				m_ddmBakerCPWeightPairsForJoints[jointIdx].push_back(CPIdxWeightPair(cpIdx, weightVal));
			}
		}
	}
}

void FBXMesh::UpdateSceneSkinningData()
{
	GUARANTEE_OR_DIE(m_ddmBakerWeightsForCPs.size() == m_controlPointsRestPose.size(), "Didn't bake yet but FbxMesh::UpdateSceneSkinningData() called!");

	FbxScene* scene = m_creatorParser.GetScene();
	GUARANTEE_OR_DIE(scene != nullptr, "m_creatorParser.GetScene() returned nullptr");
	FbxNode* nodeWithMesh = scene->GetNode(m_nodeIdx);
	GUARANTEE_OR_DIE(nodeWithMesh != nullptr, "scene->GetNode(m_nodeIdx) returned nullptr");
	FbxMesh* mesh = nodeWithMesh->GetMesh();
	GUARANTEE_OR_DIE(mesh != nullptr, "nodeWithMesh->GetMesh() returned nullptr");
	
	int numDeformers = mesh->GetDeformerCount();
	for (int deformerIdx = 0; deformerIdx < numDeformers; deformerIdx++) {
		FbxSkin* currSkin = FbxCast<FbxSkin>(mesh->GetDeformer(deformerIdx, FbxDeformer::eSkin));
		if (currSkin == nullptr)
			continue;
		int numClusters = currSkin->GetClusterCount();
		for (int clusterIdx = 0; clusterIdx < numClusters; clusterIdx++) {
			FbxCluster* currCluster = currSkin->GetCluster(clusterIdx);
			const FbxNode* currJointNode = currCluster->GetLink();
			GUARANTEE_OR_DIE(currJointNode != nullptr, "Skin cluster does not have a corresponding joint! (Check fbx file)");
			const char* jointNameForCurrCluster = currJointNode->GetName();
			int currJointIndex = GetJointIndexByName(jointNameForCurrCluster, m_model->GetJointsArray());
			/*
			int controlPointIndicesCount = currCluster->GetControlPointIndicesCount();
			if (controlPointIndicesCount <= 0)
				continue;
			int* controlPointIndices = currCluster->GetControlPointIndices();
			//double* controlPointWeights = currCluster->GetControlPointWeights();
			
			//Make a deep copy of the two arrays above
			int* cpIndicesCopy = new int[controlPointIndicesCount];
			//double* cpWeightsCopy = new double[controlPointIndicesCount];
			for (int i = 0; i < controlPointIndicesCount; i++) {
				cpIndicesCopy[i] = controlPointIndices[i];
				//cpWeightsCopy[i] = controlPointWeights[i];
			}
			*/

			//Remove all skinning data from the joint
			currCluster->SetControlPointIWCount(0);
			const std::vector<CPIdxWeightPair>& cpIdxWeightPairs = m_ddmBakerCPWeightPairsForJoints[currJointIndex];
			for (int i = 0; i < cpIdxWeightPairs.size(); i++) {
				currCluster->AddControlPointIndex(cpIdxWeightPairs[i].m_cpIndex, cpIdxWeightPairs[i].m_weight);
			}

			/*
			for (int i = 0; i < controlPointIndicesCount; i++) {
				if (m_ddmBakerWeightsForCPs[cpIndicesCopy[i]](currJointIndex, 0) > 0.0f) {
					//DebuggerPrintf("cpIndex: %d, jointIdx: %d, weight: %.2f (deformerIdx: %d, clusterIdx: %d)\n", cpIndicesCopy[i], currJointIndex, m_ddmBakerWeightsForCPs[cpIndicesCopy[i]](currJointIndex, 0), deformerIdx, clusterIdx);
					currCluster->AddControlPointIndex(cpIndicesCopy[i], m_ddmBakerWeightsForCPs[cpIndicesCopy[i]](currJointIndex, 0));
				}
			}
			*/
		}
	}
}

void FBXMesh::UpdateFBXMeshCBOConstants(Renderer& renderer)
{
	if (m_model->GetSkinningModifierState() == FBXModelSkinningModifier::LBS) {
		m_fbxMeshConstants.IsFBXMeshLBS = 1;
	}
	else {
		m_fbxMeshConstants.IsFBXMeshLBS = 0;
	}
	renderer.CopyCPUToGPU(&m_fbxMeshConstants, m_fbxMeshCBO->m_size, m_fbxMeshCBO);
}

void FBXMesh::ProcessSkinningDataOfMesh(FbxMesh& mesh, const std::vector<FBXJoint*>& joints)
{
	int numDeformers = mesh.GetDeformerCount();
	for (int i = 0; i < numDeformers; i++) {
		FbxSkin* currSkin = FbxCast<FbxSkin>(mesh.GetDeformer(i, FbxDeformer::eSkin));
		if (currSkin == nullptr)
			continue;
		int numOfClusters = currSkin->GetClusterCount();
		for (int clusterIndex = 0; clusterIndex < numOfClusters; clusterIndex++) {
			FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);
			const FbxNode* currJointNode = currCluster->GetLink();
			if (currJointNode == nullptr) {
				ERROR_AND_DIE("Skin cluster does not have a corresponding joint! (Check fbx file)");
			}
			const char* jointNameForCurrCluster = currJointNode->GetName();
			int currJointIndex = GetJointIndexByName(jointNameForCurrCluster, joints);

			FbxAMatrix transformLinkMatrix;
			FbxAMatrix globalBindPoseInverseMatrix;
			currCluster->GetTransformLinkMatrix(transformLinkMatrix);
			joints[currJointIndex]->SetGlobalBindPose(transformLinkMatrix);
			globalBindPoseInverseMatrix = transformLinkMatrix.Inverse();
			joints[currJointIndex]->SetGlobalBindPoseInverse(globalBindPoseInverseMatrix);

			int* controlPointIndices = currCluster->GetControlPointIndices();
			double* controlPointWeights = currCluster->GetControlPointWeights();
			/*
			int deleteThis = currCluster->GetControlPointIndicesCount();
			DebuggerPrintf("JointName: %s, numCPIndicesCount: %d\n", jointNameForCurrCluster, deleteThis);
			*/

			for (int j = 0; j < currCluster->GetControlPointIndicesCount(); j++) {
				m_controlPointsRestPose[controlPointIndices[j]]->m_jointWeightPairs.emplace_back(currJointIndex, (float)controlPointWeights[j]);
			}
		}
	}

	//Debugging if skinning data reading was correct
	for (int i = 0; i < m_controlPointsRestPose.size(); i++) {
		float weightSum = 0.0f;
		for (int jwPairIdx = 0; jwPairIdx < m_controlPointsRestPose[i]->m_jointWeightPairs.size(); jwPairIdx++) {
			weightSum += m_controlPointsRestPose[i]->m_jointWeightPairs[jwPairIdx].m_weight;
		}
		if (GetAbsf(weightSum - 1.0f) >= 0.1f) {
			DebuggerPrintf("For control point idx %d, weightSum value is %.3f\n", i, weightSum);
		}
	}
}

void FBXMesh::ProcessMaterialOfMesh(FbxMesh& mesh)
{
	FbxNode* node = mesh.GetNode();
	int materialCount = node->GetMaterialCount();

	if (materialCount > MAXTEXTURENUM) {
		ERROR_RECOVERABLE("There are too many materials in this mesh");
	}

	for (int i = 0; i < materialCount; i++) {
		FbxSurfaceMaterial* material = node->GetMaterial(i);
		GUARANTEE_OR_DIE(material != nullptr, "A node that has a mesh SHOULD have a surface material");

		FbxProperty diffuseMaterialProp = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
		FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(diffuseMaterialProp.GetSrcObject<FbxTexture>());
		if (fileTexture) {
			m_diffuseTexturePaths.push_back(fileTexture->GetFileName());
		}

		FbxProperty specularMaterialProp = material->FindProperty(FbxSurfaceMaterial::sSpecular);
		fileTexture = FbxCast<FbxFileTexture>(specularMaterialProp.GetSrcObject<FbxTexture>());
		if (fileTexture) {
			m_specularTexturePaths.push_back(fileTexture->GetFileName());
		}

		FbxProperty normalMaterialProp = material->FindProperty(FbxSurfaceMaterial::sNormalMap);
		fileTexture = FbxCast<FbxFileTexture>(normalMaterialProp.GetSrcObject<FbxTexture>());
		if (fileTexture) {
			m_normalTexturePaths.push_back(fileTexture->GetFileName());
		}

		FbxProperty ambientMaterialProp = material->FindProperty(FbxSurfaceMaterial::sAmbient);
		fileTexture = FbxCast<FbxFileTexture>(ambientMaterialProp.GetSrcObject<FbxTexture>());
		if (fileTexture) {
			m_ambientTexturePaths.push_back(fileTexture->GetFileName());
		}

		FbxProperty glossMaterialProp = material->FindProperty(FbxSurfaceMaterial::sShininess);
		fileTexture = FbxCast<FbxFileTexture>(glossMaterialProp.GetSrcObject<FbxTexture>());
		if (fileTexture) {
			m_glossTexturePaths.push_back(fileTexture->GetFileName());
		}
	}
}

bool FBXMesh::IsMeshMappingModeAllTheSame(FbxMesh& mesh) const
{
	FbxNode* node = mesh.GetNode();
	if (node == nullptr) {
		ERROR_AND_DIE("FbxMesh.GetNode() returned nullptr!");
	}

	int elementMaterialCount = mesh.GetElementMaterialCount();
	if (elementMaterialCount == 0) {
		ERROR_RECOVERABLE("Mesh doesn't have any materials!");
		return true;
	}

	for (int i = 0; i < mesh.GetElementMaterialCount(); i++) {
		FbxGeometryElementMaterial* materialElement = mesh.GetElementMaterial(i);
		if (materialElement->GetMappingMode() == FbxGeometryElement::eByPolygon) {
			return false;
		}
	}

	return true;
}

void FBXMesh::ProcessKeyAnimOfMesh(const FbxMesh& mesh, const std::vector<FBXJoint*>& joints, std::vector<FBXPose>& inout_poseSequence, FbxScene& scene)
{
	//Only supports one animation
	FbxAnimStack* currAnimStack = scene.GetCurrentAnimationStack();	//No animation for this file
	if (currAnimStack == nullptr)
		return;
	FbxString animStackName = currAnimStack->GetName();
	FbxTakeInfo* takeInfo = scene.GetTakeInfo(animStackName);
	FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
	FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
	auto animTimeMode = start.GetGlobalTimeMode();	//It's 30 frames per second
	FbxLongLong startFrameIndex = start.GetFrameCount(animTimeMode);
	FbxLongLong endFrameIndex = end.GetFrameCount(animTimeMode);

	GUARANTEE_OR_DIE(startFrameIndex <= endFrameIndex, "startFrameIndex > endFrameIndex!");

	int numDeformers = mesh.GetDeformerCount();

	FbxLongLong numFrames = endFrameIndex - startFrameIndex + 1;

	/*
	std::vector<FBXPose>& poseSequence = m_model->m_animManager->GetFBXPoseSequence();
	if (poseSequence.size() == 0) {	//If the pose sequence is empty in manager...
		poseSequence.resize(numFrames);
		for (int i = 0; i < poseSequence.size(); i++) {
			poseSequence[i].SetRootJoint(m_model->GetRootJoint());
		}
	}
	*/
	GUARANTEE_OR_DIE(inout_poseSequence.size() == (size_t)numFrames, "poseSequence.size() != numFrames!");

	for (int i = 0; i < numDeformers; i++) {
		FbxSkin* currSkin = FbxCast<FbxSkin>(mesh.GetDeformer(i, FbxDeformer::eSkin));
		if (currSkin == nullptr)
			continue;
		int numOfClusters = currSkin->GetClusterCount();
		for (int clusterIndex = 0; clusterIndex < numOfClusters; clusterIndex++) {
			FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);
			FbxNode* currJointNode = currCluster->GetLink();
			if (currJointNode == nullptr) {
				ERROR_AND_DIE("Skin cluster does not have a corresponding joint! (Check fbx file)");
			}
			const char* jointNameForCurrCluster = currJointNode->GetName();
			int currJointIndex = GetJointIndexByName(jointNameForCurrCluster, joints);

			/*
			if (joints[currJointIndex]->GetNumLocalKeyFrames() == static_cast<unsigned int>(numFrames)) {	//Remove duplicate data!
				continue;
			}
			*/

			int poseSequenceIdx = 0;
			for (FbxLongLong frameIdx = startFrameIndex; frameIdx <= endFrameIndex; frameIdx++) {
				FbxTime currTime;
				currTime.SetFrame(frameIdx);
				FbxAMatrix key = currJointNode->EvaluateLocalTransform(currTime);
				FbxVector4 translationKey = key.GetT();
				FbxQuaternion rotationKey = key.GetQ();
				FbxVector4 scaleKey = key.GetS();

				inout_poseSequence[poseSequenceIdx].SetJointPoseEntry(currJointIndex, scaleKey, rotationKey, translationKey);
				/*
				joints[currJointIndex]->AddLocalTranslationKeyFrame(translationKey);
				joints[currJointIndex]->AddLocalRotationKeyFrame(rotationKey);
				joints[currJointIndex]->AddLocalScalingKeyFrame(scaleKey);
				*/
				poseSequenceIdx++;
			}
		}
	}
}

/*
float FBXMesh::GetAnimTimeSpan() const
{
	return m_animEndTime - m_animStartTime;
}
*/

AABB3 FBXMesh::GetBoundingBox() const
{
	return m_boundingBox;
}

FBXMesh* FBXMesh::CreateCopy() const
{
	FBXMesh* copy = new FBXMesh(m_creatorParser, m_name, m_nodeIdx);

	copy->m_controlPointsRestPose.reserve(m_controlPointsRestPose.size());
	for (const FBXControlPoint* controlPoint : m_controlPointsRestPose) {
		FBXControlPoint* copiedControlPoint = new FBXControlPoint(controlPoint->m_position);
		copiedControlPoint->m_jointWeightPairs = controlPoint->m_jointWeightPairs;
		copy->m_controlPointsRestPose.push_back(copiedControlPoint);
	}

	copy->m_controlPointsMatrixRestPose = m_controlPointsMatrixRestPose;
	copy->m_facesMatrix = m_facesMatrix;

	copy->m_renderVertexToControlPointMap = m_renderVertexToControlPointMap;
	copy->m_renderVertices = m_renderVertices;
	copy->m_renderIndices = m_renderIndices;

	copy->m_diffuseTexturePaths = m_diffuseTexturePaths;
	copy->m_specularTexturePaths = m_specularTexturePaths;
	copy->m_normalTexturePaths = m_normalTexturePaths;
	copy->m_glossTexturePaths = m_glossTexturePaths;
	copy->m_ambientTexturePaths = m_ambientTexturePaths;

	//NOT! copying the modifier!
	copy->m_ddmModifierCPU = m_ddmModifierCPU;
	copy->m_ddmModifierGPU = m_ddmModifierGPU;

	copy->m_isMeshDebugMode = m_isMeshDebugMode;

	copy->m_ddmBakerLHSMatrices = m_ddmBakerLHSMatrices;
	copy->m_ddmBakerRHSMatrices = m_ddmBakerRHSMatrices;
	copy->m_ddmBakerWeightsForCPs = m_ddmBakerWeightsForCPs;

	copy->m_ddmBakerCPWeightPairsForJoints = m_ddmBakerCPWeightPairsForJoints;

	//For GetWeightsMatrix
	copy->m_isRigidBinding = m_isRigidBinding;
	copy->m_boundingBox = m_boundingBox;

	return copy;
}

std::string FBXMesh::GetName() const
{
	return m_name;
}

void FBXMesh::CreateVertexAndPushToRenderDataArray(FbxMesh& mesh, std::map<Vertex_FBX, unsigned int>& verticesToIndicesMap, int vertexIndex, int controlPointIndex, int materialIdx)
{
	Vec3 position;
	Vec3 normal;
	Vec3 tangent;
	Vec3 binormal;
	Rgba8 color;
	Vec2 uv;
	IntVec4 jointIndices1;
	IntVec4 jointIndices2;
	Vec4 jointWeights1;
	Vec4 jointWeights2;

	position = m_controlPointsRestPose[controlPointIndex]->m_position;
	/*
	normal = ReadNormal(*mesh.GetElementNormal(0), controlPointIndex, vertexIndex);
	tangent = ReadTangent(*mesh.GetElementTangent(0), controlPointIndex, vertexIndex);

	if (mesh.GetElementBinormalCount() < 1) {
		DebuggerPrintf("Mesh doesn't have a binormal element, so generating them...\n");
		binormal = CrossProduct3D(normal, tangent);
	}
	else {
		binormal = ReadBinormal(*mesh.GetElementBinormal(0), controlPointIndex, vertexIndex);
	}
	*/

	if (mesh.GetElementVertexColorCount() < 1) {
		color = Rgba8::WHITE;
	}
	else {
		color = ReadColor(*mesh.GetElementVertexColor(0), controlPointIndex, vertexIndex);
	}

	if (mesh.GetElementUVCount() < 1) {
		DebuggerPrintf("Mesh doesn't have a uv element!\n");
		uv = Vec2::ZERO;
	}
	else {
		uv = ReadUV(*mesh.GetElementUV(0), controlPointIndex, vertexIndex);
	}

	const std::vector<JointWeightPair>& jointWeightPairs = m_controlPointsRestPose[controlPointIndex]->m_jointWeightPairs;
	int jointWeightPairsNum = GetMin((int)jointWeightPairs.size(), 8);
	for (int jwPairIdx = 0; jwPairIdx < jointWeightPairsNum; jwPairIdx++) {
		if (jwPairIdx < 4) {
			jointIndices1[jwPairIdx] = jointWeightPairs[jwPairIdx].m_jointIndex;
			jointWeights1[jwPairIdx] = jointWeightPairs[jwPairIdx].m_weight;
		}
		else {
			int secondArraySetIdx = jwPairIdx - 4;
			jointIndices2[secondArraySetIdx] = jointWeightPairs[jwPairIdx].m_jointIndex;
			jointWeights2[secondArraySetIdx] = jointWeightPairs[jwPairIdx].m_weight;
		}
	}

	/*
	float debugCheckWeightsEquals1 = jointWeights1.x + jointWeights1.y + jointWeights1.z + jointWeights1.w + jointWeights2.x + jointWeights2.y + jointWeights2.z + jointWeights2.w;
	if (GetAbsf(debugCheckWeightsEquals1 - 1.0f) >= 0.1f) {
		ERROR_AND_DIE(Stringf("Sth Fucked Up for Vertex Index %d", vertexIndex));
	}
	*/

	Vertex_FBX newVertex(position, normal, tangent, binormal, color, uv, jointIndices1, jointIndices2, jointWeights1, jointWeights2, materialIdx);

	auto foundNewVertexIndexPair = verticesToIndicesMap.find(newVertex);
	if (foundNewVertexIndexPair == verticesToIndicesMap.end()) {
		verticesToIndicesMap[newVertex] = (unsigned int)m_renderVertices.size();
		m_renderIndices.push_back((unsigned int)m_renderVertices.size());
		m_renderVertices.push_back(newVertex);
		m_renderVertexToControlPointMap.push_back(controlPointIndex); 
	}
	else {
		m_renderIndices.push_back(foundNewVertexIndexPair->second);
	}
}

Vec3 FBXMesh::ReadNormal(const FbxGeometryElementNormal& normalElement, int ctrlPointIndex, int vertexIndex)
{
	return ReadAttribute<Vec3, FbxGeometryElementNormal, FbxVector4>(normalElement, ctrlPointIndex, vertexIndex);
}

Vec3 FBXMesh::ReadTangent(const FbxGeometryElementTangent& tangentElement, int ctrlPointIndex, int vertexIndex)
{
	return ReadAttribute<Vec3, FbxGeometryElementTangent, FbxVector4>(tangentElement, ctrlPointIndex, vertexIndex);
}

Vec3 FBXMesh::ReadBinormal(const FbxGeometryElementBinormal& binormalElement, int ctrlPointIndex, int vertexIndex)
{
	return ReadAttribute<Vec3, FbxGeometryElementBinormal, FbxVector4>(binormalElement, ctrlPointIndex, vertexIndex);
}

Rgba8 FBXMesh::ReadColor(const FbxGeometryElementVertexColor& vertexColorElement, int ctrlPointIndex, int vertexIndex)
{
	Rgba8 readAttribute;
	switch (vertexColorElement.GetMappingMode()) {
	case FbxLayerElement::eByControlPoint: {
		switch (vertexColorElement.GetReferenceMode()) {
		case FbxLayerElement::eDirect:
		{
			const FbxColor& fbxColor = vertexColorElement.GetDirectArray().GetAt(ctrlPointIndex);
			readAttribute.r = DenormalizeByte((float)fbxColor.mRed);
			readAttribute.g = DenormalizeByte((float)fbxColor.mGreen);
			readAttribute.b = DenormalizeByte((float)fbxColor.mBlue);
			readAttribute.a = DenormalizeByte((float)fbxColor.mAlpha);
			break;
		}
		case FbxLayerElement::eIndexToDirect:
		{
			int index = vertexColorElement.GetIndexArray().GetAt(ctrlPointIndex);
			const FbxColor& fbxColor = vertexColorElement.GetDirectArray().GetAt(index);
			readAttribute.r = DenormalizeByte((float)fbxColor.mRed);
			readAttribute.g = DenormalizeByte((float)fbxColor.mGreen);
			readAttribute.b = DenormalizeByte((float)fbxColor.mBlue);
			readAttribute.a = DenormalizeByte((float)fbxColor.mAlpha);
			break;
		}
		default:
			ERROR_RECOVERABLE("Mapping is weird in this file!");
			break;
		}
		break;
	}
	case FbxLayerElement::eByPolygonVertex: {
		switch (vertexColorElement.GetReferenceMode()) {
		case FbxLayerElement::eDirect:
		{
			const FbxColor& fbxColor = vertexColorElement.GetDirectArray().GetAt(vertexIndex);
			readAttribute.r = DenormalizeByte((float)fbxColor.mRed);
			readAttribute.g = DenormalizeByte((float)fbxColor.mGreen);
			readAttribute.b = DenormalizeByte((float)fbxColor.mBlue);
			readAttribute.a = DenormalizeByte((float)fbxColor.mAlpha);
			break;
		}
		case FbxLayerElement::eIndexToDirect:
		{
			int index = vertexColorElement.GetIndexArray().GetAt(vertexIndex);
			const FbxColor& fbxColor = vertexColorElement.GetDirectArray().GetAt(index);
			readAttribute.r = DenormalizeByte((float)fbxColor.mRed);
			readAttribute.g = DenormalizeByte((float)fbxColor.mGreen);
			readAttribute.b = DenormalizeByte((float)fbxColor.mBlue);
			readAttribute.a = DenormalizeByte((float)fbxColor.mAlpha);
			break;
		}
		default:
			ERROR_RECOVERABLE("Mapping is weird in this file!");
			break;
		}
		break;
	}
	}
	return readAttribute;
}

Vec2 FBXMesh::ReadUV(const FbxGeometryElementUV& uvElement, int ctrlPointIndex, int vertexIndex)
{
	return ReadAttribute<Vec2, FbxGeometryElementUV, FbxVector2>(uvElement, ctrlPointIndex, vertexIndex);
}

JointWeightPair FBXMesh::GetMaxInfluenceJointWeightPair(const std::vector<JointWeightPair>& jwPairs) const
{
	GUARANTEE_OR_DIE(jwPairs.size() > 0, "jwPairs.size() == 0");
	JointWeightPair currentMaximum = jwPairs[0];
	for (int i = 1; i < jwPairs.size(); i++) {
		if (currentMaximum.m_weight < jwPairs[i].m_weight) {
			currentMaximum = jwPairs[i];
		}
	}
	return currentMaximum;
}
