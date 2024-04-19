#pragma once
#include "Engine/FBX/Vertex_FBX.hpp"
#include "ThirdParty/fbxsdk/fbxsdk.h"
#include <string>
#include <vector>

class FBXJoint;
struct Mat44;

Mat44 ConvertFbxAMatrixToMat44(const FbxAMatrix& fbxMatrix);
int  GetJointIndexByName(const std::string& jointName, const std::vector<FBXJoint*>& joints);
std::string GetTexturePathFromMaterial(const FbxSurfaceMaterial& material, const char* surfaceMaterialTypeEnum);

void CalculateFBXTangents(std::vector<Vertex_FBX>& verts, const std::vector<unsigned int>& indices);
void CalculateFBXAveragedNormals(std::vector<Vertex_FBX>& verts, const std::vector<unsigned int>& indices);

bool AreSkeletonHierarchiesIdentical(FBXJoint& rootJointA, FBXJoint& rootJointB);

unsigned int GetNumberOfJointsUnderThisJointInclusive(const FBXJoint& joint);

bool AreJointsInTheSameChain(const FBXJoint& upstreamJoint, const FBXJoint& downstreamJoint);

float GetMaxDistanceBetweenTwoJointsOnTheSameChain(const FBXJoint& upstreamJoint, const FBXJoint& downstreamJoint);