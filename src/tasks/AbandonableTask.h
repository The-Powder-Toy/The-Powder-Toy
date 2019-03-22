#ifndef ABANDONABLETASK_H_
#define ABANDONABLETASK_H_

#include "Task.h"

class AbandonableTask : public Task
{
	pthread_cond_t done_cv;
	
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
