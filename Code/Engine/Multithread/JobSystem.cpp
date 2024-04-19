#include "Engine/Multithread/JobSystem.hpp"
#include "Engine/Multithread/Job.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

JobSystem* g_theJobSystem = nullptr;

JobSystem::JobSystem(const JobSystemConfig& config):m_config(config)
{
}

JobSystem::~JobSystem()
{
}

void JobSystem::Startup()
{
	m_numCpuCores = std::thread::hardware_concurrency();

	int numWorkers = m_config.m_numJobWorkerThreads;
	if (numWorkers < 0 || numWorkers >= m_numCpuCores) {
		numWorkers = m_numCpuCores - 1;	//-1 because you have to exclude the main thread
	}

	for (int i = 0; i < numWorkers; i++) {
		m_jobWorkerThreads.push_back(new JobWorkerThread(i, *this));
	}

	m_isQuitting = false;
}

void JobSystem::Shutdown()
{
	m_isQuitting = true;

	m_areThereUnclaimedJobsCV.notify_all();

	for (JobWorkerThread* workerThread : m_jobWorkerThreads) {
		if (workerThread) {
			workerThread->Join();
			delete workerThread;
		}
	}

	m_jobWorkerThreads.clear();

	m_unclaimedJobsMutex.lock();
	while (!m_unclaimedJobs.empty()) {
		Job* job = m_unclaimedJobs.front();
		m_unclaimedJobs.pop_front();
		delete job;
	}
	m_unclaimedJobsMutex.unlock();

	m_claimedJobsMutex.lock();
	for (Job* job: m_claimedJobs) {
		delete job;
	}
	m_claimedJobsMutex.unlock();

	m_completedJobsMutex.lock();
	while (!m_completedJobs.empty()) {
		Job* job = m_completedJobs.front();
		m_completedJobs.pop_front();
		delete job;
	}
	m_completedJobsMutex.unlock();
}

Job* JobSystem::GetCompletedJob()
{
	Job* completedJob = nullptr;
	m_completedJobsMutex.lock();
	if (!m_completedJobs.empty()) {
		completedJob = m_completedJobs.front();
		m_completedJobs.pop_front();
	}
	m_completedJobsMutex.unlock();
	return completedJob;
}

bool JobSystem::IsQuitting() const
{
	return m_isQuitting;
}

Job* JobSystem::GetUnclaimedJobFromQueue()
{
	Job* jobToDo = nullptr;

	m_unclaimedJobsMutex.lock();
	if (!m_unclaimedJobs.empty()) {
		jobToDo = m_unclaimedJobs.front();
		m_unclaimedJobs.pop_front();
	}
	m_unclaimedJobsMutex.unlock();

	return jobToDo;
}

void JobSystem::PostNewJob(Job* job)
{
	if (job == nullptr)
		ERROR_AND_DIE("You cannot post a nullptr as new job");
	m_unclaimedJobsMutex.lock();
	m_unclaimedJobs.push_back(job);
	m_unclaimedJobsMutex.unlock();
	m_areThereUnclaimedJobsCV.notify_all();
}

void JobSystem::WaitUntilAllJobsCompleted()
{
	while (true) {
		bool unclaimedEmpty = false;
		bool claimedEmpty = false;

		{
			std::lock_guard<std::mutex> unclaimedLock(m_unclaimedJobsMutex);
			std::lock_guard<std::mutex> claimedLock(m_claimedJobsMutex);
			unclaimedEmpty = m_unclaimedJobs.empty();
			claimedEmpty = m_claimedJobs.empty();
		}

		if (unclaimedEmpty && claimedEmpty) {
			break;
		}

		// Wait or yield the current thread to avoid busy waiting
		std::this_thread::yield();
	}
}

int JobSystem::GetNumQueuedJobs()
{
	m_unclaimedJobsMutex.lock();
	int numQueuedJobs = (int)m_unclaimedJobs.size();
	m_unclaimedJobsMutex.unlock();
	return numQueuedJobs;
}

int JobSystem::GetNumClaimedJobs()
{
	m_claimedJobsMutex.lock();
	int numClaimedJobs = (int)m_claimedJobs.size();
	m_claimedJobsMutex.unlock();
	return numClaimedJobs;
}

int JobSystem::GetNumCompletedJobs()
{
	m_completedJobsMutex.lock();
	int numCompletedJobs = (int)m_completedJobs.size();
	m_completedJobsMutex.unlock();
	return numCompletedJobs;
}

int JobSystem::GetNumCpuCores() const
{
	return m_numCpuCores;
}

void JobSystem::MarkJobAsClaimed(Job* job)
{
	if (job == nullptr)
		ERROR_AND_DIE("You cannot mark a nullptr as claimed job");
	m_claimedJobsMutex.lock();
	m_claimedJobs.insert(job);
	m_claimedJobsMutex.unlock();
}

void JobSystem::MarkJobAsCompleted(Job* job)
{
	if (job == nullptr)
		ERROR_AND_DIE("You cannot mark a nullptr as completed job");

	m_claimedJobsMutex.lock();
	auto foundJobItr = m_claimedJobs.find(job);
	if (foundJobItr != m_claimedJobs.end()) {
		m_claimedJobs.erase(foundJobItr);
	}
	else {
		m_claimedJobsMutex.unlock();
		ERROR_AND_DIE("Job to be marked complete was not on the claimedJobs list");
	}
	m_claimedJobsMutex.unlock();

	m_completedJobsMutex.lock();
	m_completedJobs.push_back(job);
	m_completedJobsMutex.unlock();
}