#include <algorithm>
#include <iomanip>
#include "ColourPickerActivity.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Label.h"
#include "gui/interface/Keys.h"
#include "gui/game/Tool.h"
#include "gui/Style.h"
#include "Format.h"
#include "gui/game/GameModel.h"

ColourPickerActivity::ColourPickerActivity(ui::Colour initialColour, ColourPickedCallback * callback) :
	WindowActivity(ui::Point(-1, -1), ui::Point(266, 175)),
	currentHue(0),
	currentSaturation(0),
	currentValue(0),
	mouseDown(false),
	valueMouseDown(false),
	callback(callback)
{

	class ColourChange : public ui::TextboxAction
	{
		ColourPickerActivity * a;
	public:
		ColourChange(ColourPickerActivity * a) : a(a) {}

		void TextChangedCallback(ui::Textbox * sender)
		{
			int r, g, b, alpha;
			r = format::StringToNumber<int>(a->rValue->GetText());
			g = format::StringToNumber<int>(a->gValue->GetText());
			b = format::StringToNumber<int>(a->bValue->GetText());
			alpha = format::StringToNumber<int>(a->aValue->GetText());
			if (r > 255)
				r = 255;
			if (g > 255)
				g = 255;
			if (b > 255)
				b = 255;
			if (alpha > 255)
				alpha = 255;

			RGB_to_HSV(r, g, b, &a->currentHue, &a->currentSaturation, &a->currentValue);
			a->currentAlpha = alpha;
			a->UpdateTextboxes(r, g, b, alpha);
		}
	};

	rValue = new ui::Textbox(ui::Point(5, Size.Y-23), ui::Point(30, 17), "255");
	rValue->SetActionCallback(new ColourChange(this));
	rValue->SetLimit(3);
	rValue->SetInputType(ui::Textbox::Number);
	AddComponent(rValue);

	gValue = new ui::Textbox(ui::Point(40, Size.Y-23), ui::Point(30, 17), "255");
	gValue->SetActionCallback(new ColourChange(this));
	gValue->SetLimit(3);
	gValue->SetInputType(ui::Textbox::Number);
	AddComponent(gValue);

	bValue = new ui::Textbox(ui::Point(75, Size.Y-23), ui::Point(30, 17), "255");
	bValue->SetActionCallback(new ColourChange(this));
	bValue->SetLimit(3);
	bValue->SetInputType(ui::Textbox::Number);
	AddComponent(bValue);

	aValue = new ui::Textbox(ui::Point(110, Size.Y-23), ui::Point(30, 17), "255");
	aValue->SetActionCallback(new ColourChange(this));
	aValue->SetLimit(3);
	aValue->SetInputType(ui::Textbox::Number);
	AddComponent(aValue);

	hexValue = new::ui::Label(ui::Point(150, Size.Y-23), ui::Point(53, 17), "0xFFFFFFFF");
	AddComponent(hexValue);

	class OkayAction: public ui::ButtonAction
	{
		ColourPickerActivity * a;
	public:
		OkayAction(ColourPickerActivity * a) : a(a) { }
		void ActionCallback(ui::Button * sender)
		{
			int Red, Green, Blue;
			Red = format::StringToNumber<int>(a->rValue->GetText());
			Green = format::StringToNumber<int>(a->gValue->GetText());
			Blue = format::StringToNumber<int>(a->bValue->GetText());
			ui::Colour col(Red, Green, Blue, a->currentAlpha);
			if(a->callback)
				a->callback->ColourPicked(col);
			a->Exit();
		}
	};

	ui::Button * doneButton = new ui::Button(ui::Point(Size.X-45, Size.Y-23), ui::Point(40, 17), "Done");
	doneButton->SetActionCallback(new OkayAction(this));
	AddComponent(doneButton);
	SetOkayButton(doneButton);

	RGB_to_HSV(initialColour.Red, initialColour.Green, initialColour.Blue, &currentHue, &currentSaturation, &currentValue);
	currentAlpha = initialColour.Alpha;
	UpdateTextboxes(initialColour.Red, initialColour.Green, initialColour.Blue, initialColour.Alpha);
}

void ColourPickerActivity::UpdateTextboxes(int r, int g, int b, int a)
{
	rValue->SetText(format::NumberToString<int>(r));
	gValue->SetText(format::NumberToString<int>(g));
	bValue->SetText(format::NumberToString<int>(b));
	aValue->SetText(format::NumberToString<int>(a));
	std::stringstream hex;
	hex << std::hex << "0x" << std::setfill('0') << std::setw(2) << std::uppercase << a << std::setw(2) << r << std::setw(2) << g << std::setw(2) << b;
	hexValue->SetText(hex.str());
}
void ColourPickerActivity::OnTryExit(ExitMethod method)
{
	Exit();
}

void ColourPickerActivity::OnMouseMove(int x, int y, int dx, int dy)
{
	if(mouseDown)
	{
		x -= Position.X+5;
		y -= Position.Y+5;

		currentHue = (float(x)/float(255))*359.0f;
		currentSaturation = 255-(y*2);
	
		if(currentSaturation > 255)
			currentSaturation = 255;
		if(currentSaturation < 0)
			currentSaturation = 0;
		if(currentHue > 359)
			currentHue = 359;
		if(currentHue < 0)
			currentHue = 0;
	}

	if(valueMouseDown)
	{
		x -= Position.X+5;
		//y -= Position.Y+5;

		currentValue = x;

		if(currentValue > 255)
			currentValue = 255;
		if(currentValue < 0)
			currentValue = 0;
	}

	if(mouseDown || valueMouseDown)
	{
		int cr, cg, cb;
		HSV_to_RGB(currentHue, currentSaturation, currentValue, &cr, &cg, &cb);
		UpdateTextboxes(cr, cg, cb, currentAlpha);
	}
}

void ColourPickerActivity::OnMouseDown(int x, int y, unsigned button)
{
	x -= Position.X+5;
	y -= Position.Y+5;
	if(x >= 0 && x < 256 && y >= 0 && y <= 128)
	{
		mouseDown = true;
		currentHue = (float(x)/float(255))*359.0f;
		currentSaturation = 255-(y*2);

		if(currentSaturation > 255)
			currentSaturation = 255;
		if(currentSaturation < 0)
			currentSaturation = 0;
		if(currentHue > 359)
			currentHue = 359;
		if(currentHue < 0)
			currentHue = 0;
	}

	if(x >= 0 && x < 256 && y >= 132 && y <= 142)
	{
		valueMouseDown = true;
		currentValue = x;

		if(currentValue > 255)
			currentValue = 255;
		if(currentValue < 0)
			currentValue = 0;
	}

	if(mouseDown || valueMouseDown)
	{
		int cr, cg, cb;
		HSV_to_RGB(currentHue, currentSaturation, currentValue, &cr, &cg, &cb);
		UpdateTextboxes(cr, cg, cb, currentAlpha);
	}
}

void ColourPickerActivity::OnMouseUp(int x, int y, unsigned button)
{
	if(mouseDown || valueMouseDown)
	{
		int cr, cg, cb;
		HSV_to_RGB(currentHue, currentSaturation, currentValue, &cr, &cg, &cb);
		UpdateTextboxes(cr, cg, cb, currentAlpha);
	}

	if(mouseDown)
	{
		mouseDown = false;
		x -= Position.X+5;
		y -= Position.Y+5;

		currentHue = (float(x)/float(255))*359.0f;
		currentSaturation = 255-(y*2);

		if(currentSaturation > 255)
			currentSaturation = 255;
		if(currentSaturation < 0)
			currentSaturation = 0;
		if(currentHue > 359)
			currentHue = 359;
		if(currentHue < 0)
			currentHue = 0;
	}

	if(valueMouseDown)
	{
		valueMouseDown = false;

		x -= Position.X+5;
		//y -= Position.Y+5;

		currentValue = x;

		if(currentValue > 255)
			currentValue = 255;
		if(currentValue < 0)
			currentValue = 0;
	}
}

void ColourPickerActivity::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if (key == SDLK_TAB)
	{
		if (rValue->IsFocused())
			gValue->TabFocus();
		else if (gValue->IsFocused())
			bValue->TabFocus();
		else if (bValue->IsFocused())
			aValue->TabFocus();
		else if (aValue->IsFocused())
			rValue->TabFocus();
	}
}

void ColourPickerActivity::OnDraw()
{
	Graphics * g = GetGraphics();
	//g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->fillrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3, 0, 0, 0, currentAlpha);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);

	g->drawrect(Position.X+4, Position.Y+4, 258, 130, 180, 180, 180, 255);

	g->drawrect(Position.X+4, Position.Y+4+4+128, 258, 12, 180, 180, 180, 255);


	int offsetX = Position.X+5;
	int offsetY = Position.Y+5;


	//draw color square
	int lastx = -1, currx = 0;
	for(int saturation = 0; saturation <= 255; saturation+=2)
	{
		for(int hue = 0; hue <= 359; hue++)
		{
			currx = clamp_flt(hue, 0, 359)+offsetX;
			if (currx == lastx)
				continue;
			lastx = currx;
			int cr = 0;
			int cg = 0;
			int cb = 0;
			HSV_to_RGB(hue, 255-saturation, currentValue, &cr, &cg, &cb);
			g->blendpixel(currx, (saturation/2)+offsetY, cr, cg, cb, currentAlpha);
		}
	}

	//draw brightness bar
	for(int value = 0; value <= 255; value++)
		for(int i = 0;  i < 10; i++)
		{
			int cr = 0;
			int cg = 0;
			int cb = 0;
			HSV_to_RGB(currentHue, currentSaturation, value, &cr, &cg, &cb);

			g->blendpixel(value+offsetX, i+offsetY+127+5, cr, cg, cb, currentAlpha);
		}

	//draw color square pointer
	int currentHueX = clamp_flt(currentHue, 0, 359);
	int currentSaturationY = ((255-currentSaturation)/2);
	g->xor_line(offsetX+currentHueX, offsetY+currentSaturationY-5, offsetX+currentHueX, offsetY+currentSaturationY-1);
	g->xor_line(offsetX+currentHueX, offsetY+currentSaturationY+1, offsetX+currentHueX, offsetY+currentSaturationY+5);
	g->xor_line(offsetX+currentHueX-5, offsetY+currentSaturationY, offsetX+currentHueX-1, offsetY+currentSaturationY);
	g->xor_line(offsetX+currentHueX+1, offsetY+currentSaturationY, offsetX+currentHueX+5, offsetY+currentSaturationY);

	//draw brightness bar pointer
	int currentValueX = restrict_flt(currentValue, 0, 254);
	g->xor_line(offsetX+currentValueX, offsetY+4+128, offsetX+currentValueX, offsetY+13+128);
	g->xor_line(offsetX+currentValueX+1, offsetY+4+128, offsetX+currentValueX+1, offsetY+13+128);
}

ColourPickerActivity::~ColourPickerActivity() {
	delete callback;
}

