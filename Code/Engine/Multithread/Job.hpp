#pragma once

class Job {
public:
	virtual ~Job() = default;
	virtual void Execute() = 0;	//Client code has to implement this function
	virtual void OnComplete() = 0;	//Client code has to implement this function
};