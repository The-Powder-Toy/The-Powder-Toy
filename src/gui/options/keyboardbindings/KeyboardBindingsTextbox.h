#ifndef KEYBOARDBINDINGSTEXTBOX_H
#define KEYBOARDBINDINGSTEXTBOX_H

#include "gui/interface/Textbox.h"
#include "KeyboardBindingsModel.h"

class KeyboardBindingsController;

class KeyboardBindingsTextbox: public ui::Textbox
{
public:
	KeyboardBindingsTextbox(ui::Point position, ui::Point size);

	void OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
	void OnMouseClick(int x, int y, unsigned button);

	void SetModel(BindingModel _model);
	void SetTextFromModifierAndScan(uint32_t modifier, uint32_t scan);
	void SetTextToPrevious();

	void OnTextInput(String text) {}

	void AttachController(KeyboardBindingsController* _c);

protected:
	String prevKey;
	KeyboardBindingsController* c;
	BindingModel model;
};

#endif /* KEYBOARDBINDINGSTEXTBOX_H */
