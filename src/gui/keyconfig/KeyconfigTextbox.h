#ifndef KEYCONFIGSTEXTBOX_H
#define KEYCONFIGSTEXTBOX_H

#include "gui/interface/Textbox.h"
#include "KeyconfigModel.h"

class KeyconfigController;

class KeyconfigTextbox: public ui::Textbox
{
public:
	KeyconfigTextbox(ui::Point position, ui::Point size);

	void OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
	void OnMouseClick(int x, int y, unsigned button);

	void SetModel(BindingModel _model);
	void SetTextFromModifierAndScan(int modifier, int scan);
	void SetTextToPrevious();

	void OnTextInput(String text) {}

	void AttachController(KeyconfigController* _c);

protected:
	String prevKey;
	KeyconfigController* c;
	BindingModel model;
};

#endif /* KEYCONFIGSTEXTBOX_H */
