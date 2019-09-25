static int Aa;
#if 0 // temporarily disabled so it doesn't interfere with what I'm doing -- LBPHacker
#include "KeyconfigTextbox.h"
#include "SDLCompat.h"
#include "gui/interface/Window.h"
#include "client/Client.h"
#include "KeyconfigModel.h"
#include "KeyconfigController.h"

KeyconfigTextbox::KeyconfigTextbox(ui::Point position, ui::Point size) :
	ui::Textbox(position, size)
{
	// reasonable defaults
	SetTextColour(ui::Colour(255, 255, 255));
	Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
}

void KeyconfigTextbox::OnMouseClick(int x, int y, unsigned button)
{
	prevKey = GetText();
	SetText("");
	c->ForceHasConflict();
}

void KeyconfigTextbox::AttachController(KeyconfigController* _c)
{
	c = _c;
}

void KeyconfigTextbox::SetModel(BindingModel _model)
{
	model = _model;
}

void KeyconfigTextbox::SetTextToPrevious()
{
	SetText(prevKey);	
}

void KeyconfigTextbox::SetTextFromModifierAndScan(int modifier, int scan)
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

void KeyconfigTextbox::OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	ui::Textbox::OnKeyRelease(key, scan, repeat, shift, ctrl, alt);

	int mod = 0x00;
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
	newModel.scan = (int) scan;
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
#endif
