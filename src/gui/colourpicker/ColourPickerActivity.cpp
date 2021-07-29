#include "ColourPickerActivity.h"

#include "gui/interface/Textbox.h"
#include "gui/interface/Slider.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/Keys.h"
#include "gui/Style.h"

#include "graphics/Graphics.h"

#include "Misc.h"

ColourPickerActivity::ColourPickerActivity(ui::Colour initialColour, OnPicked onPicked_) :
	WindowActivity(ui::Point(-1, -1), ui::Point(266, 215)),
	currentHue(0),
	currentSaturation(0),
	currentValue(0),
	mouseDown(false),
	onPicked(onPicked_)
{
	auto colourChange = [this] {
		int r, g, b, alpha;
		r = rValue->GetText().ToNumber<int>(true);
		g = gValue->GetText().ToNumber<int>(true);
		b = bValue->GetText().ToNumber<int>(true);
		alpha = aValue->GetText().ToNumber<int>(true);
		if (r > 255)
			r = 255;
		if (g > 255)
			g = 255;
		if (b > 255)
			b = 255;
		if (alpha > 255)
			alpha = 255;

		RGB_to_HSV(r, g, b, &currentHue, &currentSaturation, &currentValue);
		currentAlpha = alpha;
		UpdateTextboxes(r, g, b, alpha);
		UpdateSliders();
	};

	auto colourChangeSlider = [this] {
		int r, g, b;
		currentHue = hSlider->GetValue();
		currentSaturation = sSlider->GetValue();
		currentValue = vSlider->GetValue();

		HSV_to_RGB(currentHue, currentSaturation, currentValue, &r, &g, &b);
		UpdateTextboxes(r, g, b, currentAlpha);
		UpdateSliders();
	};


	hSlider = new ui::Slider(ui::Point(0,134), ui::Point(Size.X,17), 359);
	hSlider->SetActionCallback({ colourChangeSlider });
	AddComponent(hSlider);

	sSlider = new ui::Slider(ui::Point(0, 134 + 17), ui::Point(Size.X, 17), 255);
	sSlider->SetActionCallback({ colourChangeSlider });
	AddComponent(sSlider);

	vSlider = new ui::Slider(ui::Point(0, 134 + 34), ui::Point(Size.X, 17), 255);
	vSlider->SetActionCallback({ colourChangeSlider });
	AddComponent(vSlider);


	rValue = new ui::Textbox(ui::Point(5, Size.Y-23), ui::Point(30, 17), "255");
	rValue->SetActionCallback({ colourChange });
	rValue->SetLimit(3);
	rValue->SetInputType(ui::Textbox::Number);
	AddComponent(rValue);

	gValue = new ui::Textbox(ui::Point(40, Size.Y-23), ui::Point(30, 17), "255");
	gValue->SetActionCallback({ colourChange });
	gValue->SetLimit(3);
	gValue->SetInputType(ui::Textbox::Number);
	AddComponent(gValue);

	bValue = new ui::Textbox(ui::Point(75, Size.Y-23), ui::Point(30, 17), "255");
	bValue->SetActionCallback({ colourChange });
	bValue->SetLimit(3);
	bValue->SetInputType(ui::Textbox::Number);
	AddComponent(bValue);

	aValue = new ui::Textbox(ui::Point(110, Size.Y-23), ui::Point(30, 17), "255");
	aValue->SetActionCallback({ colourChange });
	aValue->SetLimit(3);
	aValue->SetInputType(ui::Textbox::Number);
	AddComponent(aValue);

	hexValue = new::ui::Label(ui::Point(150, Size.Y-23), ui::Point(53, 17), "0xFFFFFFFF");
	AddComponent(hexValue);

	ui::Button * doneButton = new ui::Button(ui::Point(Size.X-45, Size.Y-23), ui::Point(40, 17), "Done");
	doneButton->SetActionCallback({ [this] {
		int Red, Green, Blue;
		Red = rValue->GetText().ToNumber<int>(true);
		Green = gValue->GetText().ToNumber<int>(true);
		Blue = bValue->GetText().ToNumber<int>(true);
		ui::Colour col(Red, Green, Blue, currentAlpha);
		if (onPicked)
			onPicked(col);
		Exit();
	} });
	AddComponent(doneButton);
	SetOkayButton(doneButton);

	RGB_to_HSV(initialColour.Red, initialColour.Green, initialColour.Blue, &currentHue, &currentSaturation, &currentValue);
	currentAlpha = initialColour.Alpha;
	UpdateTextboxes(initialColour.Red, initialColour.Green, initialColour.Blue, initialColour.Alpha);
	UpdateSliders();
}

void ColourPickerActivity::UpdateTextboxes(int r, int g, int b, int a)
{
	rValue->SetText(String::Build(r));
	gValue->SetText(String::Build(g));
	bValue->SetText(String::Build(b));
	aValue->SetText(String::Build(a));
	hexValue->SetText(String::Build(Format::Hex(), Format::Uppercase(), Format::Width(2), a, r, g, b));
}

void ColourPickerActivity::UpdateSliders()
{
	hSlider->SetValue(currentHue);
	sSlider->SetValue(currentSaturation);
	vSlider->SetValue(currentValue);

	int r, g, b;

	//Value gradient
	HSV_to_RGB(currentHue, currentSaturation, 255, &r, &g, &b);
	vSlider->SetColour(ui::Colour(0, 0, 0), ui::Colour(r, g, b));

	//Saturation gradient
	if (currentValue != 0)
	{
		HSV_to_RGB(currentHue, 255, currentValue, &r, &g, &b);
		sSlider->SetColour(ui::Colour(currentValue, currentValue, currentValue), ui::Colour(r, g, b));
	}
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

		currentHue = int((float(x)/float(255))*359.0f);
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

	if(mouseDown)
	{
		int cr, cg, cb;
		HSV_to_RGB(currentHue, currentSaturation, currentValue, &cr, &cg, &cb);
		UpdateTextboxes(cr, cg, cb, currentAlpha);
		UpdateSliders();
	}
}

void ColourPickerActivity::OnMouseDown(int x, int y, unsigned button)
{
	x -= Position.X+5;
	y -= Position.Y+5;
	if(x >= 0 && x < 256 && y >= 0 && y <= 128)
	{
		mouseDown = true;
		currentHue = int((float(x)/float(255))*359.0f);
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

	if(mouseDown)
	{
		int cr, cg, cb;
		HSV_to_RGB(currentHue, currentSaturation, currentValue, &cr, &cg, &cb);
		UpdateTextboxes(cr, cg, cb, currentAlpha);
		UpdateSliders();
	}
}

void ColourPickerActivity::OnMouseUp(int x, int y, unsigned button)
{
	if(mouseDown)
	{
		int cr, cg, cb;
		HSV_to_RGB(currentHue, currentSaturation, currentValue, &cr, &cg, &cb);
		UpdateTextboxes(cr, cg, cb, currentAlpha);
		UpdateSliders();
	}

	if(mouseDown)
	{
		mouseDown = false;
		x -= Position.X+5;
		y -= Position.Y+5;

		currentHue = int((float(x)/float(255))*359.0f);
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
}

void ColourPickerActivity::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
		return;
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


	int offsetX = Position.X+5;
	int offsetY = Position.Y+5;


	//draw color square
	int lastx = -1, currx = 0;
	for(int saturation = 0; saturation <= 255; saturation+=2)
	{
		for(int hue = 0; hue <= 359; hue++)
		{
			currx = clamp_flt(float(hue), 0, 359)+offsetX;
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

	//Draw hue bar gradient
	auto gradientWidth = hSlider->Size.X - 10;
	for (int rx = 0; rx < gradientWidth; rx++)
	{
		int red, green, blue;
		int hue = rx * 360 / gradientWidth;
		HSV_to_RGB(hue, currentSaturation, currentValue, &red, &green, &blue);
		for (int ry = 0; ry < (hSlider->Size.Y / 2) - 1; ry++)
		{
			g->blendpixel(
				rx + offsetX + hSlider->Position.X,
				ry + offsetY + hSlider->Position.Y,
				red, green, blue, currentAlpha
			);
		}
	}

	//draw color square pointer
	int currentHueX = clamp_flt(float(currentHue), 0, 359);
	int currentSaturationY = ((255-currentSaturation)/2);
	g->xor_line(offsetX+currentHueX, offsetY+currentSaturationY-5, offsetX+currentHueX, offsetY+currentSaturationY-1);
	g->xor_line(offsetX+currentHueX, offsetY+currentSaturationY+1, offsetX+currentHueX, offsetY+currentSaturationY+5);
	g->xor_line(offsetX+currentHueX-5, offsetY+currentSaturationY, offsetX+currentHueX-1, offsetY+currentSaturationY);
	g->xor_line(offsetX+currentHueX+1, offsetY+currentSaturationY, offsetX+currentHueX+5, offsetY+currentSaturationY);

}
