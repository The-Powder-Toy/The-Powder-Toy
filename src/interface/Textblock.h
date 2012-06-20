/*
 * Textblock.h
 *
 *  Created on: Jan 29, 2012
 *      Author: Simon
 */

#ifndef TEXTBLOCK_H_
#define TEXTBLOCK_H_

#include <vector>
#include <string>
#include <sstream>
#include "Label.h"

namespace ui
{

class Textblock: public ui::Label
{
	bool autoHeight;
	void updateMultiline();
	std::string textLines;
public:
	Textblock(Point position, Point size, std::string textboxText);
	virtual void TextPosition() {}
	virtual void SetText(std::string text);
	virtual std::string GetDisplayText() { return textLines; }
	virtual void Draw(const Point& screenPos);
	virtual ~Textblock();
};
}

#endif /* TEXTBLOCK_H_ */
