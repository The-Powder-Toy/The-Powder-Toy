#pragma once

#include "common/String.h"
#include "Activity.h"
#include "gui/interface/Colour.h"

#include <functional>

namespace ui
{
	class Textbox;
	class Label;
}

class ColourPickerActivity : public WindowActivity
{
	using OnPicked = std::function<void (ui::Colour)>;

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

	OnPicked onPicked;

	void UpdateTextboxes(int r, int g, int b, int a);
public:
	ColourPickerActivity(ui::Colour initialColour, OnPicked onPicked = nullptr);
	virtual ~ColourPickerActivity() = default;

	void OnMouseMove(int x, int y, int dx, int dy) override;
	void OnMouseDown(int x, int y, unsigned button) override;
	void OnMouseUp(int x, int y, unsigned button) override;
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnTryExit(ExitMethod method) override;
	void OnDraw() override;
};
