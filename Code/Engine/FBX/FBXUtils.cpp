#include "Engine/Fbx/FBXUtils.hpp"
#include "Engine/Fbx/FBXJoint.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <queue>

Mat44 ConvertFbxAMatrixToMat44(const FbxAMatrix& fbxMatrix)
{
	Mat44 matrix;
	matrix.m_values[Mat44::Ix] = (float)fbxMatrix.Get(0, 0);
	matrix.m_values[Mat44::Iy] = (float)fbxMatrix.Get(0, 1);
	matrix.m_values[Mat44::Iz] = (float)fbxMatrix.Get(0, 2);
	matrix.m_values[Mat44::Iw] = (float)fbxMatrix.Get(0, 3);

	matrix.m_values[Mat44::Jx] = (float)fbxMatrix.Get(1, 0);
	matrix.m_values[Mat44::Jy] = (float)fbxMatrix.Get(1, 1);
	matrix.m_values[Mat44::Jz] = (float)fbxMatrix.Get(1, 2);
	matrix.m_values[Mat44::Jw] = (float)fbxMatrix.Get(1, 3);

	matrix.m_values[Mat44::Kx] = (float)fbxMatrix.Get(2, 0);
	matrix.m_values[Mat44::Ky] = (float)fbxMatrix.Get(2, 1);
	matrix.m_values[Mat44::Kz] = (float)fbxMatrix.Get(2, 2);
	matrix.m_values[Mat44::Kw] = (float)fbxMatrix.Get(2, 3);

	matrix.m_values[Mat44::Tx] = (float)fbxMatrix.Get(3, 0);
	matrix.m_values[Mat44::Ty] = (float)fbxMatrix.Get(3, 1);
	matrix.m_values[Mat44::Tz] = (float)fbxMatrix.Get(3, 2);
	matrix.m_values[Mat44::Tw] = (float)fbxMatrix.Get(3, 3);
	return matrix;
}

int GetJointIndexByName(const std::string& jointName, const std::vector<FBXJoint*>& joints)
{
	for (int i = 0; i < joints.size(); i++) {
		if (joints[i] && joints[i]->GetName() == jointName) {
			return i;
		}
	}
	ERROR_AND_DIE(Stringf("Joint %s is not added to the m_joints array", jointName.c_str()));
}

std::string GetTexturePathFromMaterial(const FbxSurfaceMaterial& material, const char* surfaceMaterialTypeName)
{
	FbxProperty property;
	property = material.FindProperty(surfaceMaterialTypeName);
	if (property.IsValid() == false) {
		ERROR_AND_DIE(Stringf("Property %s is not valid for this surface material!", surfaceMaterialTypeName));
	}
	int textureCount = property.GetSrcObjectCount<FbxTexture>();
	for (int i = 0; i < textureCount ; i++) {
		FbxTexture* texture = FbxCast<FbxTexture>(property.GetSrcObject<FbxTexture>(i));
		if (texture) {
			FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(texture);
			if (strcmp(surfaceMaterialTypeName, FbxSurfaceMaterial::sDiffuse) == 0) {
				return fileTexture->GetFileName();
			}
			else if (strcmp(surfaceMaterialTypeName, FbxSurfaceMaterial::sSpecular) == 0) {
				return fileTexture->GetFileName();
			}
		}
		else {
			ERROR_AND_DIE("texture is a nullptr which doesn't make sense (Debug it)");
		}
	}
	return "";
}

void CalculateFBXTangents(std::vector<Vertex_FBX>& verts, const std::vector<unsigned int>& indices)
{
	size_t vertexCount = verts.size();
	std::vector<Vec3> tangents;
	tangents.resize(vertexCount, Vec3());
	std::vector<Vec3> bitangents;
	bitangents.resize(vertexCount, Vec3());

	for (size_t i = 0; i < indices.size(); i += 3) {
		int i0 = indices[i];
		int i1 = indices[i + 1];
		int i2 = indices[i + 2];
		const Vec3& p0 = verts[i0].m_position;
		const Vec3& p1 = verts[i1].m_position;
		const Vec3& p2 = verts[i2].m_position;
		const Vec2& uv0 = verts[i0].m_uvTexCoords;
		const Vec2& uv1 = verts[i1].m_uvTexCoords;
		const Vec2& uv2 = verts[i2].m_uvTexCoords;

		Vec3 e1 = p1 - p0;
		Vec3 e2 = p2 - p0;

		float x1 = uv1.x - uv0.x;
		float x2 = uv2.x - uv0.x;
		float y1 = uv1.y - uv0.y;
		float y2 = uv2.y - uv0.y;

		float r = 1.0f / (x1 * y2 - x2 * y1);
		Vec3 t = (e1 * y2 - e2 * y1) * r;
		Vec3 b = (e2 * x1 - e1 * x2) * r;

		tangents[i0] += t;
		tangents[i1] += t;
		tangents[i2] += t;
		bitangents[i0] += b;
		bitangents[i1] += b;
		bitangents[i2] += b;
	}

	for (int i = 0; i < vertexCount; i++) {
		tangents[i] = (tangents[i] - DotProduct3D(tangents[i], verts[i].m_normal) * verts[i].m_normal).GetNormalized();
		bitangents[i] = (bitangents[i] - DotProduct3D(bitangents[i], verts[i].m_normal) * verts[i].m_normal - DotProduct3D(bitangents[i], tangents[i]) * tangents[i]).GetNormalized();

		verts[i].m_tangent = tangents[i];
		verts[i].m_binormal = bitangents[i];
	}
}

void CalculateFBXAveragedNormals(std::vector<Vertex_FBX>& verts, const std::vector<unsigned int>& indices)
{
	size_t vertexCount = verts.size();
	std::vector<Vec3> normals;
	normals.resize(vertexCount, Vec3());

	for (size_t i = 0; i < indices.size(); i += 3) {
		int i0 = indices[i];
		int i1 = indices[i + 1];
		int i2 = indices[i + 2];
		const Vec3& p0 = verts[i0].m_position;
		const Vec3& p1 = verts[i1].m_position;
		const Vec3& p2 = verts[i2].m_position;

		Vec3 e1 = p1 - p0;
		Vec3 e2 = p2 - p0;

		//Now calculate the normal
		Vec3 n = CrossProduct3D(e1, e2);
		normals[i0] += n;
		normals[i1] += n;
		normals[i2] += n;
	}

	for (int i = 0; i < vertexCount; i++) {
		verts[i].m_normal = normals[i].GetNormalized();
	}
}

bool AreSkeletonHierarchiesIdentical(FBXJoint& rootJointA, FBXJoint& rootJointB)
{
	int numChildJointsA = rootJointA.GetNumChildJoints();
	if (numChildJointsA != rootJointB.GetNumChildJoints())
		return false;

	for (int i = 0; i < numChildJointsA; i++) {
		if (AreSkeletonHierarchiesIdentical(*rootJointA.GetChildJointByIdx(i), *rootJointB.GetChildJointByIdx(i)) == false) {
			return false;
		}
	}

	return true;
}

unsigned int GetNumberOfJointsUnderThisJointInclusive(const FBXJoint& joint)
{
	std::queue<const FBXJoint*> jointsToProcess;
	jointsToProcess.push(&joint);
	unsigned int count = 0;
	while (jointsToProcess.empty() == false) {
		const FBXJoint* jointToProcess = jointsToProcess.front();
		jointsToProcess.pop();
		count++;
		for (unsigned int childJointIdx = 0; childJointIdx < (unsigned int)jointToProcess->GetNumChildJoints(); childJointIdx++) {
			jointsToProcess.push(jointToProcess->GetChildJointByIdx(childJointIdx));
		}
	}
	return count;
}

bool AreJointsInTheSameChain(const FBXJoint& upstreamJoint, const FBXJoint& downstreamJoint)
{
	std::queue<const FBXJoint*> jointToProcess;
	jointToProcess.push(&upstreamJoint);
	while (jointToProcess.empty() == false) {
		const FBXJoint* currentJoint = jointToProcess.front();
		jointToProcess.pop();
		GUARANTEE_OR_DIE(currentJoint != nullptr, "currentJoint should not be a nullptr");
		int numChildJoints = currentJoint->GetNumChildJoints();
		for (int childJointIdx = 0; childJointIdx < numChildJoints; childJointIdx++) {
			if (currentJoint->GetChildJointByIdx(childJointIdx) == &downstreamJoint) {
				return true;
			}
			else {
				jointToProcess.push(currentJoint->GetChildJointByIdx(childJointIdx));
			}
		}
	}
	return false;
}

float GetMaxDistanceBetweenTwoJointsOnTheSameChain(const FBXJoint& upstreamJoint, const FBXJoint& downstreamJoint)
{
	GUARANTEE_OR_DIE(AreJointsInTheSameChain(upstreamJoint, downstreamJoint), "The two joints are not on the same chain or parent-child relationship is weird");
	float totalDistance = 0.0f;
	const FBXJoint* currentJoint = &downstreamJoint;
	while (true) {
		if (currentJoint == nullptr || currentJoint == &upstreamJoint) {
			break;
		}
		totalDistance += currentJoint->GetOriginalLocalTranslate().GetLength();
		currentJoint = currentJoint->GetParentJoint();
	}
	return totalDistance;
}
