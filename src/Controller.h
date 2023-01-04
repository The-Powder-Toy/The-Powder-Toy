#pragma once
class Controller
{
private:
	virtual void Exit();
	virtual void Show();
	virtual void Hide();
	virtual ~Controller() = default;
};
