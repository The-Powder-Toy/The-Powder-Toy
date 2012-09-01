#pragma once

#include <vector>
#include <string>
#include "Activity.h"
#include "interface/Window.h"
#include "interface/Textbox.h"

class ColourPickedCallback
{
public:
	ColourPickedCallback() {}
	virtual  ~ColourPickedCallback() {}
	virtual void ColourPicked(ui::Colour colour) {}
};

class ColourPickerActivity: public WindowActivity {
	int currentHue;
	int currentSaturation;
	int currentValue;

	bool mouseDown;
	bool valueMouseDown;

	ui::Textbox * rValue;
	ui::Textbox * gValue;
	ui::Textbox * bValue;

	ColourPickedCallback * callback;
public:
	virtual void OnMouseMove(int x, int y, int dx, int dy);
	virtual void OnMouseDown(int x, int y, unsigned button);
	virtual void OnMouseUp(int x, int y, unsigned button);
	ColourPickerActivity(ui::Colour initialColour, ColourPickedCallback * callback = NULL);
	virtual ~ColourPickerActivity();
	virtual void OnDraw();
};
