#pragma once
#include "Engine/Fbx/Vertex_FBX.hpp"
#include "Engine/Fbx/FBXControlPoint.hpp"
#include "Engine/Fbx/FBXModel.hpp"
#include "Engine/Fbx/FBXPose.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/AABB3.hpp"
#include "ThirdParty/fbxsdk/fbxsdk.h"
#include <Eigen/Dense>
#include <vector>
#include <string>
#include <map>
#include <memory>

struct FBXControlPoint;
struct Face;
struct HalfEdge;
template<typename Vertex_FBX>
class GPUMesh;
class Texture;
class Renderer;
class ConstantBuffer;
class FBXJoint;
class FBXDDMModifierCPU;
class FBXDDMModifierGPU;
class FBXParser;

class FBXMesh {
	friend class FBXParser;

public:
	~FBXMesh();
	FBXMesh(const FBXMesh& copyFrom) = delete;

	void ProcessFbxMesh(FbxNode& node, FbxMesh& mesh, const std::vector<FBXJoint*>& joints, std::vector<FBXPose>& inout_poseSequence, FbxScene& scene);
	void SetGPUData(Renderer& renderer);
	void Render(Renderer& renderer) const;
	void SetFBXModel(FBXModel& model);
	void AddDDMModifier();
	std::shared_ptr<FBXDDMModifierCPU> GetDDMModifierCPU() const;
	std::shared_ptr<FBXDDMModifierGPU> GetDDMModifierGPU() const;
	void PrecomputeDDM(bool isCPUSide, bool useCotangentLaplacian, int numLaplacianIterations, float lambda, float kappa, float alpha);
	void ApplyDDMv0_CPU(const std::vector<Mat44>& allJointSkinningMatrices);
	void ApplyDDMv1_CPU(const std::vector<Mat44>& allJointSkinningMatrices);
	void ApplyDDMv0_GPU(const std::vector<Mat44>& allJointSkinningMatrices);
	void ApplyDDMv1_GPU(const std::vector<Mat44>& allJointSkinningMatrices);

	void RestoreGPUVerticesToRestPose();
	void SetRigidBinding(bool isRigidBound);

	Eigen::MatrixX3d GetControlPointsMatrixRestPose() const;
	Eigen::MatrixX3i GetFacesMatrix() const;
	Eigen::MatrixXd GetWeightsMatrix() const;

	int GetNumVertices() const;
	int GetNumJoints() const;
	std::vector<unsigned int> GetRenderVertexToControlPointMap() const;

	void ToggleMeshDebugMode();

	void PrepareDDMBaker(int numPoses);
	void FillLHSMatForDDMBaker(const std::vector<Mat44>& allJointSkinningMatrices, int poseIdx);
	void FillRHSMatForDDMBaker(const std::vector<Mat44>& allJointSkinningMatrices, int poseIdx);
	void SolveDDMBakerLinearSystems(int numMaxBones, float pruneThreshold);
	void UpdateSceneSkinningData();

	void UpdateFBXMeshCBOConstants(Renderer& renderer);

	//Made public bc needed for parser
	void ProcessKeyAnimOfMesh(const FbxMesh& mesh, const std::vector<FBXJoint*>& joints, std::vector<FBXPose>& inout_poseSequence, FbxScene& scene);

	//float GetAnimTimeSpan() const;

	AABB3 GetBoundingBox() const;

	FBXMesh* CreateCopy() const;
	std::string GetName() const;

private:
	FBXMesh(FBXParser& creatorParser, const std::string& name, int nodeIdx);	//Only FBXParser can make this
	Eigen::MatrixX3f GetDDMv0_GPU_Deformation(const std::vector<Mat44>& allJointSkinningMatrices);	//ALWAYS calculate

private:
	void ProcessSkinningDataOfMesh(FbxMesh& mesh, const std::vector<FBXJoint*>& joints);
	void ProcessMaterialOfMesh(FbxMesh& mesh);
	void CreateVertexAndPushToRenderDataArray(FbxMesh& mesh, std::map<Vertex_FBX, unsigned int>& verticesToIndicesMap, int vertexIndex, int controlPointIndex, int materialIdx);
	bool IsMeshMappingModeAllTheSame(FbxMesh& mesh) const;

	template<typename ReturnType, typename ElementType, typename ElementFbxVectorType>
	ReturnType ReadAttribute(const ElementType& element, int ctrlPointIndex, int vertexIndex);

	Vec3 ReadNormal(const FbxGeometryElementNormal& normalElement, int ctrlPointIndex, int vertexIndex);
	Vec3 ReadTangent(const FbxGeometryElementTangent& tangentElement, int ctrlPointIndex, int vertexIndex);
	Vec3 ReadBinormal(const FbxGeometryElementBinormal& binormalElement, int ctrlPointIndex, int vertexIndex);
	Rgba8 ReadColor(const FbxGeometryElementVertexColor& vertexColorElement, int ctrlPointIndex, int vertexIndex);
	Vec2 ReadUV(const FbxGeometryElementUV& uvElement, int ctrlPointIndex, int vertexIndex);

	JointWeightPair GetMaxInfluenceJointWeightPair(const std::vector<JointWeightPair>& jwPairs) const;

private:
	FBXParser& m_creatorParser;
	const std::string m_name;
	const int m_nodeIdx = -1;
	FBXModel* m_model = nullptr;

	std::vector<FBXControlPoint*> m_controlPointsRestPose;
	Eigen::MatrixX3d m_controlPointsMatrixRestPose;
	Eigen::MatrixX3i m_facesMatrix;

	std::vector<unsigned int> m_renderVertexToControlPointMap;
	std::vector<Vertex_FBX> m_renderVertices;
	std::vector<unsigned int> m_renderIndices;

	std::vector<std::string> m_diffuseTexturePaths;
	std::vector<std::string> m_specularTexturePaths;
	std::vector<std::string> m_normalTexturePaths;
	std::vector<std::string> m_glossTexturePaths;
	std::vector<std::string> m_ambientTexturePaths;

	std::vector<Texture*> m_diffuseTextures;
	std::vector<Texture*> m_specularTextures;
	std::vector<Texture*> m_normalTextures;
	std::vector<Texture*> m_glossTextures;
	std::vector<Texture*> m_ambientTextures;

	GPUMesh<Vertex_FBX>* m_gpuMesh = nullptr;
	GPUMesh<Vertex_PCU>* m_debugGPUMesh = nullptr;

	//FbxTime::EMode m_animTimeMode = FbxTime::EMode::eFrames30;
	//float m_animStartTime = 0.0f;
	//float m_animEndTime = 0.0f;

	std::shared_ptr<FBXDDMModifierCPU> m_ddmModifierCPU = nullptr;
	std::shared_ptr<FBXDDMModifierGPU> m_ddmModifierGPU = nullptr;

	bool m_isMeshDebugMode = false;

	std::vector<Eigen::MatrixXf> m_ddmBakerLHSMatrices;	//length should be num control points
	std::vector<Eigen::MatrixXf> m_ddmBakerRHSMatrices;	//length should be num control points
	std::vector<Eigen::MatrixXf> m_ddmBakerWeightsForCPs;	//length should be num control points
	struct CPIdxWeightPair {
	public:
		CPIdxWeightPair(unsigned int cpIndex, float weight) : m_cpIndex(cpIndex), m_weight(weight) {};

	public:
		unsigned int m_cpIndex = 0;
		float		 m_weight = 0.0f;
	};
	std::vector<std::vector<CPIdxWeightPair>> m_ddmBakerCPWeightPairsForJoints;

	const static int MAXTEXTURENUM;

	struct FBXMeshConstants {
		int FBXMeshHasNormalTexture = 0;
		int FBXMeshHasSpecularTexture = 0;
		int FBXMeshHasGlossTexture = 0;
		int IsFBXMeshLBS = 0;
	};
	FBXMeshConstants m_fbxMeshConstants;
	ConstantBuffer* m_fbxMeshCBO = nullptr;
	const int k_fbxMeshConstantsSlot = 4;

	//For GetWeightsMatrix
	bool m_isRigidBinding = false;

	AABB3 m_boundingBox;
};

//Template member function implementation
template<typename ReturnType, typename ElementType, typename ElementFbxVectorType>
inline ReturnType FBXMesh::ReadAttribute(const ElementType& element, int ctrlPointIndex, int vertexIndex)
{
	ReturnType readAttribute;
	switch (element.GetMappingMode()) {
	case FbxLayerElement::eByControlPoint: {
		switch (element.GetReferenceMode()) {
		case FbxLayerElement::eDirect:
		{
			const ElementFbxVectorType& vector = element.GetDirectArray().GetAt(ctrlPointIndex);
			readAttribute.x = (float)vector.mData[0];
			readAttribute.y = (float)vector.mData[1];
			if constexpr (std::is_same_v<ReturnType, Vec3>)
				readAttribute.z = (float)vector.mData[2];
			break;
		}
		case FbxLayerElement::eIndexToDirect:
		{
			int index = element.GetIndexArray().GetAt(ctrlPointIndex);
			const ElementFbxVectorType& vector = element.GetDirectArray().GetAt(index);
			readAttribute.x = (float)vector.mData[0];
			readAttribute.y = (float)vector.mData[1];
			if constexpr (std::is_same_v<ReturnType, Vec3>)
				readAttribute.z = (float)vector.mData[2];
			break;
		}
		default:
			ERROR_RECOVERABLE("Mapping is weird in this file!");
			break;
		}
		break;
	}
	case FbxLayerElement::eByPolygonVertex: {
		switch (element.GetReferenceMode()) {
		case FbxLayerElement::eDirect:
		{
			const ElementFbxVectorType& vector = element.GetDirectArray().GetAt(vertexIndex);
			readAttribute.x = (float)vector.mData[0];
			readAttribute.y = (float)vector.mData[1];
			if constexpr (std::is_same_v<ReturnType, Vec3>)
				readAttribute.z = (float)vector.mData[2];
			break;
		}
		case FbxLayerElement::eIndexToDirect:
		{
			int index = element.GetIndexArray().GetAt(vertexIndex);
			const ElementFbxVectorType& vector = element.GetDirectArray().GetAt(index);
			readAttribute.x = (float)vector.mData[0];
			readAttribute.y = (float)vector.mData[1];
			if constexpr (std::is_same_v<ReturnType, Vec3>)
				readAttribute.z = (float)vector.mData[2];
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
