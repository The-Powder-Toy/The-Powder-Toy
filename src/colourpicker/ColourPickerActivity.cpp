/*
 * ElementSearchActivity.cpp
 *
 *  Created on: Jun 24, 2012
 *      Author: Simon
 */

#include <algorithm>
#include "ColourPickerActivity.h"
#include "interface/Textbox.h"
#include "interface/Label.h"
#include "interface/Keys.h"
#include "game/Tool.h"
#include "Style.h"
#include "Format.h"
#include "game/GameModel.h"

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
			int r, g, b;
			r = format::StringToNumber<int>(a->rValue->GetText());
			g = format::StringToNumber<int>(a->gValue->GetText());
			b = format::StringToNumber<int>(a->bValue->GetText());
			RGB_to_HSV(r, g, b, &a->currentHue, &a->currentSaturation, &a->currentValue);
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

	class CancelAction: public ui::ButtonAction
	{
		ColourPickerActivity * a;
	public:
		CancelAction(ColourPickerActivity * a) : a(a) { }
		void ActionCallback(ui::Button * sender)
		{
			a->Exit();
		}
	};

	class OkayAction: public ui::ButtonAction
	{
		ColourPickerActivity * a;
	public:
		OkayAction(ColourPickerActivity * a) : a(a) { }
		void ActionCallback(ui::Button * sender)
		{
			int Red, Green, Blue;
			HSV_to_RGB(a->currentHue, a->currentSaturation, a->currentValue, &Red, &Green, &Blue);
			ui::Colour col(Red, Green, Blue);
			if(a->callback)
				a->callback->ColourPicked(col);
			a->Exit();
		}
	};

	ui::Button * doneButton = new ui::Button(ui::Point(Size.X-45, Size.Y-23), ui::Point(40, 17), "Done");
	doneButton->SetActionCallback(new OkayAction(this));
	AddComponent(doneButton);
	SetOkayButton(doneButton);

	ui::Button * cancelButton = new ui::Button(ui::Point(Size.X-90, Size.Y-23), ui::Point(40, 17), "Cancel");
	cancelButton->SetActionCallback(new CancelAction(this));
	AddComponent(cancelButton);
	SetCancelButton(cancelButton);

	rValue->SetText(format::NumberToString<int>(initialColour.Red));
	gValue->SetText(format::NumberToString<int>(initialColour.Green));
	bValue->SetText(format::NumberToString<int>(initialColour.Blue));
	RGB_to_HSV(initialColour.Red, initialColour.Green, initialColour.Blue, &currentHue, &currentSaturation, &currentValue);
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
		y -= Position.Y+5;

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
		rValue->SetText(format::NumberToString<int>(cr));
		gValue->SetText(format::NumberToString<int>(cg));
		bValue->SetText(format::NumberToString<int>(cb));
	}
}

void ColourPickerActivity::OnMouseDown(int x, int y, unsigned button)
{
	x -= Position.X+5;
	y -= Position.Y+5;
	if(x >= 0 && x <= 256 && y >= 0 && y < 127)
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

	if(x >= 0 && x <= 256 && y >= 131 && y < 142)
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
		rValue->SetText(format::NumberToString<int>(cr));
		gValue->SetText(format::NumberToString<int>(cg));
		bValue->SetText(format::NumberToString<int>(cb));
	}
}

void ColourPickerActivity::OnMouseUp(int x, int y, unsigned button)
{
	if(mouseDown || valueMouseDown)
	{
		int cr, cg, cb;
		HSV_to_RGB(currentHue, currentSaturation, currentValue, &cr, &cg, &cb);
		rValue->SetText(format::NumberToString<int>(cr));
		gValue->SetText(format::NumberToString<int>(cg));
		bValue->SetText(format::NumberToString<int>(cb));
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
		y -= Position.Y+5;

		currentValue = x;

		if(currentValue > 255)
			currentValue = 255;
		if(currentValue < 0)
			currentValue = 0;
	}
}


void ColourPickerActivity::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);

	g->drawrect(Position.X+4, Position.Y+4, 258, 130, 180, 180, 180, 255);

	g->drawrect(Position.X+4, Position.Y+4+4+128, 258, 12, 180, 180, 180, 255);


	int offsetX = Position.X+5;
	int offsetY = Position.Y+5;


	for(int saturation = 0; saturation <= 255; saturation+=2)
		for(int hue = 0; hue <= 359; hue++)
		{
			int cr = 0;
			int cg = 0;
			int cb = 0;
			HSV_to_RGB(hue, 255-saturation, 255-saturation, &cr, &cg, &cb);

			g->blendpixel(clamp_flt(hue, 0, 359)+offsetX, (saturation/2)+offsetY, cr, cg, cb, 255);
		}

	//draw brightness bar
	for(int value = 0; value <= 255; value++)
		for(int i = 0;  i < 10; i++)
		{
			int cr = 0;
			int cg = 0;
			int cb = 0;
			HSV_to_RGB(currentHue, currentSaturation, value, &cr, &cg, &cb);

			g->blendpixel(value+offsetX, i+offsetY+127+5, cr, cg, cb, 255);
		}

	int currentHueX = clamp_flt(currentHue, 0, 359);
	int currentSaturationY = ((255-currentSaturation)/2);
	g->xor_line(offsetX+currentHueX, offsetY+currentSaturationY-5, offsetX+currentHueX, offsetY+currentSaturationY+5);
	g->xor_line(offsetX+currentHueX-5, offsetY+currentSaturationY, offsetX+currentHueX+5, offsetY+currentSaturationY);

	g->xor_line(offsetX+currentValue, offsetY+4+128, offsetX+currentValue, offsetY+13+128);
	g->xor_line(offsetX+currentValue+1, offsetY+4+128, offsetX+currentValue+1, offsetY+13+128);
}

ColourPickerActivity::~ColourPickerActivity() {
	if(callback)
		delete callback;
}

