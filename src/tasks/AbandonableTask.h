#ifndef ABANDONABLETASK_H_
#define ABANDONABLETASK_H_

#include "Task.h"

class AbandonableTask : public Task
{
public:
	void Start();
	void Finish();
	void Abandon();
	void Poll();
	AbandonableTask();
	virtual ~AbandonableTask();

protected:
	bool thAbandoned;
	TH_ENTRY_POINT static void * doWork_helper(void * ref);
};

#endif /* ABANDONABLETASK_H_ */
