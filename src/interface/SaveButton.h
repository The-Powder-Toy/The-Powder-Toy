#ifndef SAVEBUTTON_H_
#define SAVEBUTTON_H_

#include <string>

#include "Component.h"
#include "client/SaveFile.h"
#include "client/SaveInfo.h"
#include "client/ThumbnailListener.h"
#include "graphics/Graphics.h"
#include "search/Thumbnail.h"
#include "interface/Colour.h"

namespace ui
{
class SaveButton;
class SaveButtonAction
{
public:
	virtual void ActionCallback(ui::SaveButton * sender) {}
	virtual void AuthorActionCallback(ui::SaveButton * sender) {}
	virtual void SelectedCallback(ui::SaveButton * sender) {}
	virtual ~SaveButtonAction() {}
};

class SaveButton : public Component, public ThumbnailListener
{
	SaveFile * file;
	SaveInfo * save;
	Thumbnail * thumbnail;
	std::string name;
	bool wantsDraw;
	bool waitingForThumb;
	bool isMouseInsideAuthor;
public:
	SaveButton(Point position, Point size, SaveInfo * save);
	SaveButton(Point position, Point size, SaveFile * file);
	virtual ~SaveButton();

	virtual void OnMouseClick(int x, int y, unsigned int button);
	virtual void OnMouseUnclick(int x, int y, unsigned int button);

	virtual void OnMouseEnter(int x, int y);
	virtual void OnMouseLeave(int x, int y);

	virtual void OnMouseMovedInside(int x, int y, int dx, int dy);

	virtual void Draw(const Point& screenPos);
	virtual void Tick(float dt);

	virtual void OnThumbnailReady(Thumbnail * thumb);

	void SetSelected(bool selected_) { selected = selected_; }
	bool GetSelected() { return selected; }
	void SetSelectable(bool selectable_) { selectable = selectable_; }
	bool GetSelectable() { return selectable; }

	SaveInfo * GetSave() { return save; }
	SaveFile * GetSaveFile() { return file; }
	inline bool GetState() { return state; }
	virtual void DoAction();
	virtual void DoAuthorAction();
	virtual void DoSelection();
	void SetActionCallback(SaveButtonAction * action);
protected:
	bool isButtonDown, state, isMouseInside, selected, selectable;
	float voteRatio;
	Colour voteColour;
	SaveButtonAction * actionCallback;
};
}
#endif /* BUTTON_H_ */

