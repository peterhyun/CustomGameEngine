#pragma once
#include <vector>
#include <deque>
#include <mutex>
#include <set>
//#include <condition_variable>
#include "Engine/Multithread/JobWorkerThread.hpp"

class Job;

struct JobSystemConfig
{
public:
	JobSystemConfig(int numJobWorkerThreads):m_numJobWorkerThreads(numJobWorkerThreads){};
	int m_numJobWorkerThreads = 0;
};

class JobSystem {
public:
	friend class JobWorkerThread;
	JobSystem(const JobSystemConfig& config);
	~JobSystem();

	void Startup();
	void Shutdown();

	Job* GetCompletedJob();

	void PostNewJob(Job* job);	//Called by main thread to add Job to ToDo list

	void WaitUntilAllJobsCompleted();

	int GetNumQueuedJobs();
	int GetNumClaimedJobs();
	int GetNumCompletedJobs();

	int GetNumCpuCores() const;

private:
	bool IsQuitting() const;
	Job* GetUnclaimedJobFromQueue();
	void MarkJobAsClaimed(Job* job);
	void MarkJobAsCompleted(Job* job);

private:
	JobSystemConfig m_config;
	std::vector<JobWorkerThread*> m_jobWorkerThreads;

	int m_numCpuCores = 1;

	std::deque<Job*> m_unclaimedJobs;
	std::mutex m_unclaimedJobsMutex;

	std::set<Job*> m_claimedJobs;
	std::mutex m_claimedJobsMutex;

	std::deque<Job*> m_completedJobs;	//List of jobs finished, ready to be retrieved (Each thread worker pushes it here)
	std::mutex m_completedJobsMutex;

	std::atomic<bool> m_isQuitting = false;	//Main thread will set this variable through Shutdown() and other threads read from it. Therefore it must be atomic

	std::condition_variable m_areThereUnclaimedJobsCV;
};