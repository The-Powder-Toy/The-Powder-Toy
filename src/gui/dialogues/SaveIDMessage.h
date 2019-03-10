#ifndef SAVEIDMESSAGE_H
#define SAVEIDMESSAGE_H

#include "gui/interface/Window.h"

class SaveIDMessage: public ui::Window {
public:
	SaveIDMessage(int id);
	void OnDraw() override;
	void OnTryExit(ExitMethod method) override;
	virtual ~SaveIDMessage();
};

#endif /* SAVEIDMESSAGE_H */
