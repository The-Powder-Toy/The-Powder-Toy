#ifndef KEYCONFIGSVIEW_H
#define KEYCONFIGSVIEW_H

#include "gui/interface/Window.h"

namespace ui
{
	class ScrollPanel;
	class Button;
	class Label;
}

class KeyconfigController;
class KeyconfigTextbox;

class KeyconfigView: public ui::Window
{
	ui::ScrollPanel* scrollPanel;
	KeyconfigController* c;
public:
	KeyconfigView();
	void OnDraw() override;
	void OnTryExit(ExitMethod method) override;
	void AttachController(KeyconfigController* controller);
	virtual ~KeyconfigView();
	void OnKeyCombinationChanged(bool hasConflict);
	void BuildKeyBindingsListView();
	void OnKeyReleased();
	void ClearScrollPanel();

protected:
	ui::Button* okayButton;
	ui::Label* conflictLabel;
	std::vector<KeyconfigTextbox*> textboxes;
};

#endif /* KEYCONFIGSVIEW_H */
