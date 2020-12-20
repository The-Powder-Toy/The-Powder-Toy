#ifndef CONTROLLER_H_
#define CONTROLLER_H_

class Controller
{
private:
	virtual void Exit();
	virtual void Show();
	virtual void Hide();
	virtual ~Controller() = default;
};

#endif /* CONTROLLER_H_ */
