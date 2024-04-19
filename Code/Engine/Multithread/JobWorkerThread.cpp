#include "Engine/Multithread/JobWorkerThread.hpp"
#include "Engine/Multithread/Job.hpp"
#include "Engine/Multithread/Jobsystem.hpp"

JobWorkerThread::JobWorkerThread(int threadID, JobSystem& jobSystem) : m_threadID(threadID), m_jobSystem(jobSystem)
{
	m_thread = new std::thread(JobWorkerThread::ThreadMain, std::ref(jobSystem));
}

JobWorkerThread::~JobWorkerThread()
{
	delete m_thread;
}

void JobWorkerThread::Join() const
{
	if(m_thread)
		m_thread->join();
}

void JobWorkerThread::ThreadMain(JobSystem& jobSystem)
{
	while (!jobSystem.IsQuitting())
	{
		Job* jobToDo = jobSystem.GetUnclaimedJobFromQueue();
		if (jobToDo) {
			jobSystem.MarkJobAsClaimed(jobToDo);
			jobToDo->Execute();
			jobSystem.MarkJobAsCompleted(jobToDo);
			jobToDo->OnComplete();
		}
		else {
			std::unique_lock<std::mutex> lock(jobSystem.m_unclaimedJobsMutex);
			//Should keep waiting if jobSystem is NOT quitting and there are no queued jobs
			jobSystem.m_areThereUnclaimedJobsCV.wait(lock, [&jobSystem]() {return (jobSystem.IsQuitting() || jobSystem.m_unclaimedJobs.size() > 0); });
		}
	}
}