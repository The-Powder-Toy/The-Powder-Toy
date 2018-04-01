#ifndef PROMOTE_H
#define PROMOTE_H

#include "gui/interface/Window.h"

class Promote : public ui::Window
{
	int saveID;
public:
	Promote(int saveID);
	virtual void OnDraw();
	virtual void OnTryExit(ExitMethod method);

	void Callback(bool confirmed);
};

#endif // PROMOTE_H
