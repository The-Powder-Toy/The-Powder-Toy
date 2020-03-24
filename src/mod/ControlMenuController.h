#ifndef CONTROLMENUCONTROLLER_
#define CONTROLMENUCONTROLLER_

#include "common/String.h"
#include "client/User.h"

#include <functional>

class ControlMenuView;
class ControlMenuModel;

class ControlMenuController
{

public:
	ControlMenuController() {}
	virtual ~ControlMenuController() {}
};

#endif /* CONTROLMENUCONTROLLER_H_ */
