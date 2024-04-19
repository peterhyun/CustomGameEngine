#pragma once
#include "Engine/Multithread/Job.hpp"
#include <vector>
#include <string>
#include <Eigen/Dense>

class FBXDDMBakingJob : public Job {
public:
	FBXDDMBakingJob(class FBXModel& model, class FBXParser& parser, const std::string& exportFileName, int numPoses, int numMaxBones, float twistLimit, float pruneThreshold);
	void Execute() override;
	void OnComplete() override;

public:
	class FBXModel& m_model;
	class FBXParser& m_parser;
	const std::string m_exportFileName;
	int m_numPoses = 0;
	int m_numMaxBones = 0;
	float m_twistLimit = 0.0f;
	float m_pruneThreshold = 0.0f;
};