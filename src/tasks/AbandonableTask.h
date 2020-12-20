#ifndef ABANDONABLETASK_H_
#define ABANDONABLETASK_H_

#include "Task.h"

#include <condition_variable>

class AbandonableTask : public Task
{
	std::condition_variable done_cv;
	
public:
	void Finish();
	void Abandon();
	AbandonableTask();
	virtual ~AbandonableTask();

protected:
	void doWork_wrapper() override;
	bool thAbandoned;
};

#endif /* ABANDONABLETASK_H_ */
