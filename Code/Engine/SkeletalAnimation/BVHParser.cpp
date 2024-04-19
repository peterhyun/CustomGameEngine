#include "Engine/SkeletalAnimation/BVHParser.hpp"
#include "Engine/SkeletalAnimation/SkeletalCharacter.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Quaternion.hpp"

/*
Vec3 BVHParser::ConvertGHSpaceCoordsToBVHSpaceCoords(const Vec3& GHSpaceCoords)
{
	return Vec3(GHSpaceCoords.y, GHSpaceCoords.z, GHSpaceCoords.x);
}

Vec3 BVHParser::ConvertBVHSpaceCoordsToGHSpaceCoords(const Vec3& BVHSpaceCoords)
{
	return Vec3(BVHSpaceCoords.z, BVHSpaceCoords.x, BVHSpaceCoords.y);
}

Vec3 BVHParser::ConvertGHSpaceRotationToBVHSpaceRotation(const Vec3& GHRotation)
{
	return Vec3(GHRotation.y, GHRotation.z, GHRotation.x);
}

Vec3 BVHParser::ConvertBVHSpaceRotationToGHSpaceRotation(const Vec3& BVHRotation)
{
	return Vec3(BVHRotation.z, BVHRotation.x, BVHRotation.y);
}
*/

BVHParser::BVHParser(BVHParserConfig const& config): m_config(config)
{
}

BVHParser::~BVHParser()
{
	if (m_rootJoint) {
		delete m_rootJoint;
		m_rootJoint = nullptr;
	}
}

void BVHParser::ParseFile(const std::string& filePath)
{
	if (filePath == m_latestParsedFilePath) {
		return;
	}

	//Reset variables to default just in case you use this function to parse new files
	ResetVariables();

	std::string fileString;
	FileReadToString(fileString, filePath);
	std::istringstream iss(fileString);
	std::string line;
	while (std::getline(iss, line)) {
		BVHJoint* currentJoint = nullptr;
		if (line.find("ROOT") != std::string::npos) {
			std::istringstream jointIss(line);
			std::string jointToken;
			std::string jointName;
			jointIss >> jointToken >> jointName;	//Skip the Joint Keyword
			currentJoint = new BVHJoint;
			currentJoint->SetName(jointName);
			currentJoint->SetIsRoot(true);
			m_rootJoint = currentJoint;
			RecursivelySetCurrentJointDataAndHierarchy(iss, *currentJoint);
		}
		else if (line.find("MOTION") != std::string::npos) {
			LoadPose(iss);
		}
	}

	SetJointsVertexData();

	size_t delimiterPos = filePath.rfind('/');
	if (delimiterPos != std::string::npos) {
		std::string fileNameWithoutFolders = filePath.substr(delimiterPos + 1);
		m_latestParsedFileNameWithoutFolders = fileNameWithoutFolders;
	}
	else {
		m_latestParsedFileNameWithoutFolders = filePath;
	}
	m_latestParsedFilePath = filePath;
}

BVHJoint* BVHParser::GetDeepCopiedRig() const
{
	BVHJoint* copiedRootJoint = new BVHJoint;
	RecursivelyDeepCopyJointDataAndHierarchy(*m_rootJoint, *copiedRootJoint);
	return copiedRootJoint;
}

void BVHParser::RecursivelyDeepCopyJointDataAndHierarchy(const BVHJoint& sourceJoint, BVHJoint& copyJoint) const
{
	copyJoint.SetName(sourceJoint.GetName());

	float xOffset, yOffset, zOffset;
	sourceJoint.GetOffset(xOffset, yOffset, zOffset);
	copyJoint.SetOffset(xOffset, yOffset, zOffset);

	copyJoint.SetIsRoot(sourceJoint.IsRoot());
	copyJoint.SetIsEndSite(sourceJoint.IsEndSite());

	std::vector<BVHChannel> bvhChannels = sourceJoint.GetBVHChannels();
	for (int i = 0; i < bvhChannels.size(); i++) {
		copyJoint.AddBVHChannel(bvhChannels[i]);
	}

	int numChildJoints = sourceJoint.GetNumChildJoints();
	for (int i = 0; i < numChildJoints;i++) {
		BVHJoint* copiedChildJoint = new BVHJoint;
		RecursivelyDeepCopyJointDataAndHierarchy(*sourceJoint.GetChildJointOfIndex(i), *copiedChildJoint);
		copyJoint.AddChildJoint(*copiedChildJoint);
	}
}

void BVHParser::ResetVariables()
{
	if (m_rootJoint) {
		delete m_rootJoint;
		m_rootJoint = nullptr;
	}
	m_frames.clear();
	m_bvhChannels.clear();
	m_numChannels = 0;
	m_numFrames = 0;
	m_secondsPerFrame = 0.0f;
}

std::vector<BVHPose> BVHParser::GetAllFrames() const
{
	return m_frames;
}

std::vector<BVHPose> BVHParser::GetFrames(int startFrameIndex, int endFrameIndex) const
{
	if (startFrameIndex < 0 || startFrameIndex >= m_frames.size()) {
		ERROR_AND_DIE(Stringf("startFrameIndex: %d in BVHParser::GetFrames() when m_frames.size(): %d", startFrameIndex, m_frames.size()).c_str());
	}

	if (endFrameIndex < startFrameIndex || endFrameIndex >= m_frames.size()) {
		ERROR_AND_DIE(Stringf("endFrameIndex: %d in BVHParser::GetFrames() when startFrameIndex: %d, m_frames.size(): %d", endFrameIndex, startFrameIndex, m_frames.size()).c_str());
	}
	std::vector<BVHPose> subFrames(m_frames.begin() + startFrameIndex, m_frames.begin() + endFrameIndex);
	return subFrames;
}

std::vector<BVHPose> BVHParser::GetProcessedFrames(int startFrameIndex, int endFrameIndex) const
{
	if (startFrameIndex < 0 || startFrameIndex >= m_frames.size()) {
		ERROR_AND_DIE(Stringf("startFrameIndex: %d in BVHParser::GetProcessedFrames() when m_frames.size(): %d", startFrameIndex, m_frames.size()).c_str());
	}

	if (endFrameIndex < startFrameIndex || endFrameIndex >= m_frames.size()) {
		ERROR_AND_DIE(Stringf("endFrameIndex: %d in BVHParser::GetProcessedFrames() when startFrameIndex: %d, m_frames.size(): %d", endFrameIndex, startFrameIndex, m_frames.size()).c_str());
	}

	std::vector<BVHPose> subFrames(m_frames.begin() + startFrameIndex, m_frames.begin() + endFrameIndex);

	return BVHPose::ProcessPoseSequenceToMatchDesiredStartPosAndFwdXY(subFrames, Vec3(0.0f, 0.0f, subFrames[0].m_rootPosGH.z), Vec2(1.0f, 0.0f));
}

float BVHParser::GetSecondsPerFrame() const
{
	return m_secondsPerFrame;
}

std::string BVHParser::GetLatestParsedFileName() const
{
	return m_latestParsedFileNameWithoutFolders;
}

void BVHParser::RecursivelySetCurrentJointDataAndHierarchy(std::istringstream& iss, BVHJoint& currentJoint)
{
	std::string line;
	while (std::getline(iss, line)) {
		if (line.find("OFFSET") != std::string::npos) {
			std::istringstream offsetIss(line);
			std::string offsetToken;
			float xOffset, yOffset, zOffset;
			offsetIss >> offsetToken;
			offsetIss >> xOffset >> yOffset >> zOffset;
			currentJoint.SetOffset(xOffset, yOffset, zOffset);
		}
		else if (line.find("CHANNELS") != std::string::npos) {
			std::istringstream channelsIss(line);
			std::string channelsToken;
			int numChannels;
			channelsIss >> channelsToken >> numChannels;
			m_numChannels += numChannels;
			std::string channelName;
			for (int i = 0; i < numChannels; i++) {
				channelsIss >> channelName;
				BVHChannel bvhChannel = GetBVHChannelFromString(channelName);
				currentJoint.AddBVHChannel(bvhChannel);
				m_bvhChannels.push_back(bvhChannel);
			}
		}
		else if (line.find("JOINT") != std::string::npos || line.find("End") != std::string::npos) {
			std::istringstream jointIss(line);
			std::string jointToken;
			std::string jointName;
			jointIss >> jointToken >> jointName;	//Skip the Joint Keyword
			BVHJoint* childJoint = new BVHJoint;
			childJoint->SetName(jointName);
			if (line.find("End") != std::string::npos) {
				childJoint->SetIsEndSite(true);
			}
			RecursivelySetCurrentJointDataAndHierarchy(iss, *childJoint);
			currentJoint.AddChildJoint(*childJoint);
		}
		else if (line.find("}") != std::string::npos) {
			return;
		}
	}
}


void BVHParser::LoadPose(std::istringstream& iss)
{
	std::string line;
	while (std::getline(iss, line)) {
		if (line.find("Frames:") != std::string::npos) {
			std::istringstream framesIss(line);
			std::string framesToken;
			int numFrames = 0;
			framesIss >> framesToken >> numFrames;
			m_numFrames = numFrames;
		}
		else if (line.find("Frame Time:") != std::string::npos) {
			std::istringstream frameTimeIss(line);
			std::string frameToken, timeToken;
			float secondsPerFrame = 0.0f;
			frameTimeIss >> frameToken >> timeToken >> secondsPerFrame;
			m_secondsPerFrame = secondsPerFrame;
			break;
		}
	}

	float singleChannelData = 0.0f;
	int channelIndex = 0;
	BVHPose frameData;
	Quaternion jointQuat(1.0f, 0.0f, 0.0f, 0.0f);

	while (!iss.eof()) {
		iss >> singleChannelData;

		switch (m_bvhChannels[channelIndex]) {
		case BVHChannel::Xposition:
			frameData.m_rootPosGH.y = singleChannelData;
			break;
		case BVHChannel::Yposition:
			frameData.m_rootPosGH.z = singleChannelData;
			break;
		case BVHChannel::Zposition:
			frameData.m_rootPosGH.x = singleChannelData;
			break;
		case BVHChannel::Zrotation:	//Quaternions are applied from the right
			jointQuat = jointQuat * Quaternion::CreateFromAxisAndDegrees(singleChannelData, Vec3(1.0f, 0.0f, 0.0f));
			break;
		case BVHChannel::Xrotation:
			jointQuat = jointQuat * Quaternion::CreateFromAxisAndDegrees(singleChannelData, Vec3(0.0f, 1.0f, 0.0f));
			break;
		case BVHChannel::Yrotation:
			jointQuat = jointQuat * Quaternion::CreateFromAxisAndDegrees(singleChannelData, Vec3(0.0f, 0.0f, 1.0f));
			break;
		default:
			ERROR_AND_DIE("BVH channel data not set up correctly");
		}

		if (channelIndex != 2 && channelIndex % 3 == 2) {	//2 is where the root bone position ends. (XPosition, YPosition, ZPosition)
			frameData.m_jointQuatsGH.push_back(jointQuat);
			jointQuat = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);	//clear it out
		}

		//If it's full... put everything you have in a frame
		if (channelIndex == m_numChannels - 1) {
			frameData.m_rig = m_rootJoint;
			m_frames.push_back(frameData);
			frameData.Clear();
			channelIndex = 0;
		}
		else {
			channelIndex++;
		}
	}

	if (m_numFrames != m_frames.size()) {
		ERROR_AND_DIE(Stringf("m_numFrames: %d whereas m_frames.size(): %d", m_numFrames, m_frames.size()));
	}

}

void BVHParser::SetJointsVertexData()
{
	m_rootJoint->RecursivelySetVertexData(m_config.m_renderer);
}

BVHChannel BVHParser::GetBVHChannelFromString(const std::string& bvhChannelString)
{
	if (bvhChannelString == "Xposition")
		return BVHChannel::Xposition;
	else if (bvhChannelString == "Yposition")
		return BVHChannel::Yposition;
	else if (bvhChannelString == "Zposition")
		return BVHChannel::Zposition;
	else if (bvhChannelString == "Zrotation")
		return BVHChannel::Zrotation;
	else if (bvhChannelString == "Xrotation")
		return BVHChannel::Xrotation;
	else if (bvhChannelString == "Yrotation")
		return BVHChannel::Yrotation;
	else
		ERROR_AND_DIE(Stringf("Cannot convert %s to a BVHChannel type!", bvhChannelString.c_str()));
}
