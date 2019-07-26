#ifndef KEYBOARDBINDINGSVIEW_H
#define KEYBOARDBINDINGSVIEW_H

#include "gui/interface/Window.h"

namespace ui
{
	class ScrollPanel;
	class Button;
	class Label;
}

class KeyboardBindingsController;
class KeyboardBindingsTextbox;

class KeyboardBindingsView: public ui::Window
{
	ui::ScrollPanel* scrollPanel;
	KeyboardBindingsController* c;
public:
	KeyboardBindingsView();
	void OnDraw() override;
	void OnTryExit(ExitMethod method) override;
	void AttachController(KeyboardBindingsController* controller);
	virtual ~KeyboardBindingsView();
	void OnKeyCombinationChanged(bool hasConflict);
	void BuildKeyBindingsListView();
	void OnKeyReleased();
	void ClearScrollPanel();

protected:
	ui::Button* okayButton;
	ui::Label* conflictLabel;
	std::vector<KeyboardBindingsTextbox*> textboxes;
};

#endif /* KEYBOARDBINDINGSVIEW_H */
