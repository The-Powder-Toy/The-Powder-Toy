#pragma once

#include <vector>
#include "common/String.h"
#include "Activity.h"
#include "gui/interface/Window.h"
#include "gui/interface/Textbox.h"

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
	int currentAlpha;

	bool mouseDown;
	bool valueMouseDown;

	ui::Textbox * rValue;
	ui::Textbox * gValue;
	ui::Textbox * bValue;
	ui::Textbox * aValue;
	ui::Label * hexValue;

	ColourPickedCallback * callback;

	void UpdateTextboxes(int r, int g, int b, int a);
public:
	void OnMouseMove(int x, int y, int dx, int dy) override;
	void OnMouseDown(int x, int y, unsigned button) override;
	void OnMouseUp(int x, int y, unsigned button) override;
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnTryExit(ExitMethod method) override;
	ColourPickerActivity(ui::Colour initialColour, ColourPickedCallback * callback = NULL);
	virtual ~ColourPickerActivity();
	void OnDraw() override;
};
