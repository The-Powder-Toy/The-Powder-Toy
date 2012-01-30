/*
 * Textarea.h
 *
 *  Created on: Jan 29, 2012
 *      Author: Simon
 */

#ifndef TEXTAREA_H_
#define TEXTAREA_H_

#include <vector>
#include <string>
#include <sstream>
#include "Textbox.h"

namespace ui
{

class Textarea: public ui::Textbox
{
	void updateMultiline();
	std::string textLines;
public:
	Textarea(Point position, Point size, std::string textboxText);
	virtual void TextPosition() {}
	virtual void SetText(std::string text);
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void Draw(const Point& screenPos);
	virtual ~Textarea();
};
}

#endif /* TEXTAREA_H_ */
