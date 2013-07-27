#ifndef SAVEBUTTON_H_
#define SAVEBUTTON_H_

#include <string>

#include "Component.h"
#include "client/SaveFile.h"
#include "client/SaveInfo.h"
#include "client/requestbroker/RequestListener.h"
#include "graphics/Graphics.h"
#include "gui/interface/Colour.h"

namespace ui
{
class SaveButton;
class SaveButtonAction
{
public:
	virtual void ActionCallback(ui::SaveButton * sender) {}
	virtual void AltActionCallback(ui::SaveButton * sender) {}
	virtual void AltActionCallback2(ui::SaveButton * sender) {}
	virtual void SelectedCallback(ui::SaveButton * sender) {}
	virtual ~SaveButtonAction() {}
};

class SaveButton : public Component, public RequestListener
{
	SaveFile * file;
	SaveInfo * save;
	VideoBuffer * thumbnail;
	std::string name;
	std::string votesString;
	std::string votesBackground;
	std::string votesBackground2;
	int voteBarHeightUp;
	int voteBarHeightDown;
	bool wantsDraw;
	bool waitingForThumb;
	bool isMouseInsideAuthor;
	bool isMouseInsideHistory;
	bool showVotes;
public:
	SaveButton(Point position, Point size, SaveInfo * save);
	SaveButton(Point position, Point size, SaveFile * file);
	virtual ~SaveButton();

	virtual void OnMouseClick(int x, int y, unsigned int button);
	virtual void OnMouseUnclick(int x, int y, unsigned int button);

	virtual void OnMouseEnter(int x, int y);
	virtual void OnMouseLeave(int x, int y);

	virtual void OnMouseMovedInside(int x, int y, int dx, int dy);

	void AddContextMenu(int menuType);
	virtual void OnContextMenuAction(int item);

	virtual void Draw(const Point& screenPos);
	virtual void Tick(float dt);

	virtual void OnResponseReady(void * imagePtr, int identifier);

	void SetSelected(bool selected_) { selected = selected_; }
	bool GetSelected() { return selected; }
	void SetSelectable(bool selectable_) { selectable = selectable_; }
	bool GetSelectable() { return selectable; }
	void SetShowVotes(bool showVotes_) { showVotes = showVotes_; }

	SaveInfo * GetSave() { return save; }
	SaveFile * GetSaveFile() { return file; }
	inline bool GetState() { return state; }
	virtual void DoAction();
	virtual void DoAltAction();
	virtual void DoAltAction2();
	virtual void DoSelection();
	void SetActionCallback(SaveButtonAction * action);
protected:
	bool isButtonDown, state, isMouseInside, selected, selectable;
	SaveButtonAction * actionCallback;
};
}
#endif /* BUTTON_H_ */

