#pragma once
#include <thread>

class Job;
class JobSystem;

class JobWorkerThread {
public:
	JobWorkerThread(int threadID, JobSystem& jobSystem);
	~JobWorkerThread();

	void Join() const;
	static void ThreadMain(JobSystem& jobSystem);

private:
	JobSystem& m_jobSystem;
	std::thread* m_thread = nullptr;
	int m_threadID = -1;
};