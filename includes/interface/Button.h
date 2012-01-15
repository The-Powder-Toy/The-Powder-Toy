/*
 * Button.h
 *
 *  Created on: Jan 8, 2012
 *      Author: Simon
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include <string>

#include "Component.h"

namespace ui
{
    class Button : public Component
    {
    public:
		Button(State* parent_state, std::string buttonText);

		Button(Point position, Point size, std::string buttonText);

		Button(std::string buttonText);
		virtual ~Button();

        bool Toggleable;

        std::string ButtonText;

        virtual void OnMouseClick(int x, int y, unsigned int button);
        virtual void OnMouseUnclick(int x, int y, unsigned int button);
		//virtual void OnMouseUp(int x, int y, unsigned int button);

		virtual void OnMouseEnter(int x, int y);
		virtual void OnMouseLeave(int x, int y);

		virtual void Draw(const Point& screenPos);

        inline bool GetState() { return state; }
        virtual void DoAction(); //action of button what ever it may be
		void SetTogglable(bool isTogglable);
		bool GetTogglable();
		inline bool GetToggleState();
		inline void SetToggleState(bool state);

    protected:
		bool isButtonDown, state, isMouseInside, isTogglable, toggle;
    };
}
#endif /* BUTTON_H_ */
