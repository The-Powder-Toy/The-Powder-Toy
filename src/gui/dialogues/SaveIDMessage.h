#ifndef SAVEIDMESSAGE_H
#define SAVEIDMESSAGE_H

#include "gui/interface/Window.h"

class SaveIDMessage: public ui::Window {
public:
	SaveIDMessage(int id);
	virtual void OnDraw();
	virtual void OnTryExit(ExitMethod method);
	virtual ~SaveIDMessage();
};

#endif /* SAVEIDMESSAGE_H */
