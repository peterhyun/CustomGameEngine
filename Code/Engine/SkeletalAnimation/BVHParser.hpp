#pragma once
#include "Engine/SkeletalAnimation/BVHPose.hpp"
#include "Engine/SkeletalAnimation/BVHJoint.hpp"
#include <string>
#include <vector>
#include <sstream>

class SkeletalCharacter;

struct BVHParserConfig
{
	BVHParserConfig(Renderer& rendererForVBOs): m_renderer(rendererForVBOs) {};
	Renderer& m_renderer;
};

//enum for bvh channel data parsing
typedef enum class BVHChannel { Xposition = 0, Yposition, Zposition, Zrotation, Xrotation, Yrotation } BVHChannel;

class BVHParser
{
public:
	/*
	//GHSpaceCoords is x fwd, y left, z up
	//BVHSpaceCoords is z fwd, x left, y up
	static Vec3 ConvertGHSpaceCoordsToBVHSpaceCoords(const Vec3& GHSpaceCoords);
	static Vec3 ConvertBVHSpaceCoordsToGHSpaceCoords(const Vec3& BVHSpaceCoords);

	//BVH z rotation - GH x rotation, BVH x rotation - GH y rotation, BVH y rotation - GH z rotation
	static Vec3 ConvertGHSpaceRotationToBVHSpaceRotation(const Vec3& GHRotation);
	static Vec3 ConvertBVHSpaceRotationToGHSpaceRotation(const Vec3& BVHRotation);
	*/
	
	BVHParser(BVHParserConfig const& config);
	~BVHParser();
	void ParseFile(const std::string& filePath);
	BVHJoint* GetDeepCopiedRig() const;	//Creates a seperate rig (deep copy it)
	std::vector<BVHPose> GetAllFrames() const;
	std::vector<BVHPose> GetFrames(int startFrameIndex, int endFrameIndex) const;
	std::vector<BVHPose> GetProcessedFrames(int startFrameIndex, int endFrameIndex) const;
	float GetSecondsPerFrame() const;
	std::string GetLatestParsedFileName() const;

private:
	//Helper functions for ParseFile()
	void RecursivelySetCurrentJointDataAndHierarchy(std::istringstream& iss, BVHJoint& currentJoint);
	BVHChannel GetBVHChannelFromString(const std::string& bvhChannelString);
	void LoadPose(std::istringstream& iss);

	//Rendering related functions
	void SetJointsVertexData();

	//Helper functions for deep copying rig
	void RecursivelyDeepCopyJointDataAndHierarchy(const BVHJoint& sourceJoint, BVHJoint& copyJoint) const;

	void ResetVariables();

private:
	BVHParserConfig m_config;
	BVHJoint* m_rootJoint = nullptr;
	std::vector<BVHChannel> m_bvhChannels;
	std::vector<BVHPose> m_frames;
	int m_numChannels = 0;
	int m_numFrames = 0;
	float m_secondsPerFrame = 0.0f;
	std::string m_latestParsedFileNameWithoutFolders;
	std::string m_latestParsedFilePath;
};