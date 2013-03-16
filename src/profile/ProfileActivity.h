#ifndef PROFILEACTIVITY_H_
#define PROFILEACTIVITY_H_

#include <string>
#include "Activity.h"
#include "client/requestbroker/RequestListener.h"
#include "interface/Window.h"

class ProfileActivity: public WindowActivity, public RequestListener {
public:
	ProfileActivity(std::string username);
	virtual ~ProfileActivity();
	virtual void OnResponseReady(void * userDataPtr);
	virtual void OnDraw();
};

#endif /* PROFILEACTIVITY_H_ */
