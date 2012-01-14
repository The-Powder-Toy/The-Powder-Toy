/*
 * State.h
 *
 *  Created on: Jan 8, 2012
 *      Author: Simon
 */

#ifndef STATE_H_
#define STATE_H_

#include <vector>

#include "interface/Component.h"

namespace ui {

class State
{
public:
	State(int w, int h);
	virtual ~State();

	bool AllowExclusiveDrawing; //false will not call draw on objects outside of bounds

	virtual void Tick(float dt);
	virtual void Draw(void* userdata);

	virtual void OnMouseMove(int x, int y);
	virtual void OnMouseDown(int x, int y, unsigned int button);
	virtual void OnMouseUp(int x, int y, unsigned int button);
	virtual void OnMouseWheel(int x, int y, int d);
	virtual void OnKeyPress(int key, bool shift, bool ctrl, bool alt);
	virtual void OnKeyRelease(int key, bool shift, bool ctrl, bool alt);

	virtual void Add(Component *child);
	virtual void Remove(Component *child);

	inline bool IsFocused(Component* c) { return (c == focusedComponent_); }
	inline int GetMouseX() { return mouseX; }
	inline int GetMouseY() { return mouseY; }
	inline int GetWidth() { return width; }
	inline int GetHeight() { return height; }

protected:
	std::vector<Component*> Components;

	int width;
	int height;

	int mouseX;
	int mouseY;
	int mouseXP;
	int mouseYP;

private:
	Component* focusedComponent_;

};

} /* namespace ui */
#endif /* STATE_H_ */
