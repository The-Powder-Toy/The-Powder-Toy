#ifndef DROPDOWN_H_
#define DROPDOWN_H_

#include <utility>
#include "Component.h"

#include <functional>

namespace ui {

class DropDownWindow;

class DropDown : public ui::Component
{
	friend class DropDownWindow;
	bool isMouseInside;
	int optionIndex;

	struct DropDownAction
	{
		std::function<void ()> change;
	};
	DropDownAction actionCallback;

	std::vector<std::pair<String, int> > options;
	
public:
	DropDown(Point position, Point size);
	virtual ~DropDown() = default;

	std::pair<String, int> GetOption();
	void SetOption(int option);
	void SetOption(String option);
	void AddOption(std::pair<String, int> option);
	void RemoveOption(String option);
	void SetOptions(std::vector<std::pair<String, int> > options);
	inline void SetActionCallback(DropDownAction action) { actionCallback = action; }
	void Draw(const Point& screenPos) override;
	void OnMouseClick(int x, int y, unsigned int button) override;
	void OnMouseEnter(int x, int y) override;
	void OnMouseLeave(int x, int y) override;
};

} /* namespace ui */
#endif /* DROPDOWN_H_ */
