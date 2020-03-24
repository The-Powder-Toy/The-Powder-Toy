#ifndef CONTROLMENUMODEL_H_
#define CONTROLMENUMODEL_H_

#include <vector>
#include "common/String.h"
#include "client/User.h"

class ControlMenuView;
class ControlMenuModel
{
	std::vector<ControlMenuView*> observers;
	String statusText;
	bool loginStatus;
	void notifyStatusChanged();
	User currentUser;
public:
	ControlMenuModel();
	void Login(ByteString username, ByteString password);
	void AddObserver(ControlMenuView* observer);
	String GetStatusText();
	bool GetStatus();
	User GetUser();
	virtual ~ControlMenuModel();
};

#endif /* CONTROLMENUMODEL_H_ */
