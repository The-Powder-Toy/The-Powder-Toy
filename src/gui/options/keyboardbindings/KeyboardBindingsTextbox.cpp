#include "KeyboardBindingsTextbox.h"

#include "SDLCompat.h"
#include "gui/interface/Window.h"
#include "client/Client.h"
#include "KeyboardBindingsModel.h"
#include "KeyboardBindingsController.h"

KeyboardBindingsTextbox::KeyboardBindingsTextbox(ui::Point position, ui::Point size) :
	ui::Textbox(position, size)
{
	// reasonable defaults
	SetTextColour(ui::Colour(255, 255, 255));
	Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
}

void KeyboardBindingsTextbox::OnMouseClick(int x, int y, unsigned button)
{
	prevKey = GetText();
	SetText("");
	c->ForceHasConflict();
}

void KeyboardBindingsTextbox::AttachController(KeyboardBindingsController* _c)
{
	c = _c;
}

void KeyboardBindingsTextbox::SetModel(BindingModel _model)
{
	model = _model;
}

void KeyboardBindingsTextbox::SetTextToPrevious()
{
	SetText(prevKey);	
}

void KeyboardBindingsTextbox::SetTextFromModifierAndScan(uint32_t modifier, uint32_t scan)
{
	ByteString modDisplay;

	if (modifier & BINDING_CTRL)
	{
		modDisplay += "CTRL+";
	}
	
	if (modifier & BINDING_ALT)
	{
		modDisplay += "ALT+";
	}

	if (modifier & BINDING_SHIFT)
	{
		modDisplay += "SHIFT+";
	}

	const char* scanDisplay = SDL_GetScancodeName((SDL_Scancode) scan);
	ByteString keyNameDisplay(scanDisplay);
	keyNameDisplay = modDisplay + keyNameDisplay.ToUpper();

	SetText(keyNameDisplay.FromUtf8());
}

void KeyboardBindingsTextbox::OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	ui::Textbox::OnKeyRelease(key, scan, repeat, shift, ctrl, alt);

	uint32_t mod = 0x00;
	ByteString modDisplay = "";

	if (ctrl)
	{
		mod |= BINDING_CTRL;
		modDisplay += "CTRL+";
	}
	
	if (alt)
	{
		mod |= BINDING_ALT;
		modDisplay += "ALT+";
	}

	if (shift)
	{
		mod |= BINDING_SHIFT;
		modDisplay += "SHIFT+";
	}

	const char* scanDisplay = SDL_GetScancodeName((SDL_Scancode) scan);
	ByteString keyNameDisplay(scanDisplay);
	keyNameDisplay = modDisplay + keyNameDisplay.ToUpper();

	if (!scan)
	{
		SetText(prevKey);
		return;
	}

	SetText(keyNameDisplay.FromUtf8());
	GetParentWindow()->FocusComponent(NULL);

	BindingModel newModel;
	newModel.modifier = mod;
	newModel.scan = (uint32_t) scan;
	newModel.functionId = model.functionId;
	newModel.description = model.description;
	newModel.index = model.index;
	newModel.isNew = false;
	newModel.noShortcut = false;

	c->ChangeModel(newModel);

	model = newModel;

	// we notify the controller so the view can recover all empty textboxes
	// should the user carelessly click about
	c->NotifyKeyReleased();
}
