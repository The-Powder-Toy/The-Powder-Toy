#pragma once

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
	virtual void Exit()
	{
		Hide();
		SelfDestruct();
	}
	virtual void Show()
	{
		if(ui::Engine::Ref().GetWindow() != this)
		{
			ui::Engine::Ref().ShowWindow(this);
		}
	}
	virtual void Hide()
	{
		if(ui::Engine::Ref().GetWindow() == this)
		{
			ui::Engine::Ref().CloseWindow();
		}
	}
	virtual ~WindowActivity() {}
};