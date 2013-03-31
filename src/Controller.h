#ifndef CONTROLLER_H_
#define CONTROLLER_H_

class ControllerCallback
{
public:
	ControllerCallback() {}
	virtual void ControllerExit() {}
	virtual ~ControllerCallback() {}
};

class Controller
{
private:
	virtual void Exit();
	virtual void Show();
	virtual void Hide();
};

#endif /* CONTROLLER_H_ */
