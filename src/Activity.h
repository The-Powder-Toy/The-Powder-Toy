#pragma once
#include "Config.h"

#include "gui/interface/Window.h"

class Activity
{
public:
	virtual void Exit() {}
	virtual void Show() {}
	virtual void Hide() {}
	virtual ~Activity() {}
};

class WindowActivity: public ui::Window, public Activity
{
public:
	WindowActivity(ui::Point position, ui::Point size) :
		ui::Window(position, size)
	{
		Show();
	}
	void Exit() override
	{
		Hide();
		SelfDestruct();
	}
	void Show() override
	{
		MakeActiveWindow();
	}
	void Hide() override
	{
		CloseActiveWindow();
	}
	virtual ~WindowActivity() {}
};
