#ifndef SAVEBUTTON_H_
#define SAVEBUTTON_H_

#include <string>

#include "Component.h"
#include "search/Save.h"
#include "Graphics.h"
#include "search/Thumbnail.h"

namespace ui
{
class SaveButton;
class SaveButtonAction
{
public:
	virtual void ActionCallback(ui::SaveButton * sender) {}
	virtual ~SaveButtonAction() {}
};

class SaveButton : public Component
{
	Save * save;
	Thumbnail * thumbnail;
public:
	SaveButton(Window* parent_state, Save * save);

	SaveButton(Point position, Point size, Save * save);

	SaveButton(Save * save);
	virtual ~SaveButton();

	virtual void OnMouseClick(int x, int y, unsigned int button);
	virtual void OnMouseUnclick(int x, int y, unsigned int button);

	virtual void OnMouseEnter(int x, int y);
	virtual void OnMouseLeave(int x, int y);

	virtual void Draw(const Point& screenPos);
	virtual void Tick(float dt);

	inline bool GetState() { return state; }
	virtual void DoAction();
	void SetActionCallback(SaveButtonAction * action);
protected:
	bool isButtonDown, state, isMouseInside;
	SaveButtonAction * actionCallback;
};
}
#endif /* BUTTON_H_ */

