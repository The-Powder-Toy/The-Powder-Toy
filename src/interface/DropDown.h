#ifndef DROPDOWN_H_
#define DROPDOWN_H_

#include <utility>
#include "Component.h"
#include "Colour.h"

namespace ui {

class DropDown;
class DropDownWindow;
class DropDownAction
{
public:
	virtual void OptionChanged(DropDown * sender, std::pair<std::string, int> newOption) {}
	virtual ~DropDownAction() {}
};
class DropDown: public ui::Component {
	friend class DropDownWindow;
	bool isMouseInside;
	int optionIndex;
	DropDownAction * callback;
	std::vector<std::pair<std::string, int> > options;
public:
	DropDown(Point position, Point size);
	std::pair<std::string, int> GetOption();
	void SetOption(int option);
	void SetOption(std::string option);
	void AddOption(std::pair<std::string, int> option);
	void RemoveOption(std::string option);
	void SetOptions(std::vector<std::pair<std::string, int> > options);
	void SetActionCallback(DropDownAction * action) { callback = action;}
	virtual void Draw(const Point& screenPos);
	virtual void OnMouseClick(int x, int y, unsigned int button);
	virtual void OnMouseEnter(int x, int y);
	virtual void OnMouseLeave(int x, int y);
	virtual ~DropDown();
};

} /* namespace ui */
#endif /* DROPDOWN_H_ */
