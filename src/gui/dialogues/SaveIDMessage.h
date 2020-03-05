#ifndef SAVEIDMESSAGE_H
#define SAVEIDMESSAGE_H

#include "gui/interface/Window.h"

class SaveIDMessage : public ui::Window
{
public:
	SaveIDMessage(int id);
	virtual ~SaveIDMessage() = default;

	void OnDraw() override;
	void OnTryExit(ExitMethod method) override;
};

#endif /* SAVEIDMESSAGE_H */
