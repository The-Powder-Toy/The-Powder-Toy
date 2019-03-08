#ifndef SAVEBUTTON_H_
#define SAVEBUTTON_H_

#include "common/String.h"

#include "Component.h"
#include "client/SaveFile.h"
#include "client/SaveInfo.h"
#include "graphics/Graphics.h"
#include "gui/interface/Colour.h"
#include "client/http/ThumbnailRequest.h"
#include "client/http/RequestMonitor.h"
#include "graphics/Graphics.h"

#include <memory>

class ThumbnailRendererTask;
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

class SaveButton : public Component, public http::RequestMonitor<http::ThumbnailRequest>
{
	SaveFile * file;
	SaveInfo * save;
	std::unique_ptr<VideoBuffer> thumbnail;
	ui::Point thumbSize = ui::Point(0, 0);
	String name;
	String votesString;
	String votesBackground;
	String votesBackground2;
	int voteBarHeightUp;
	int voteBarHeightDown;
	bool wantsDraw;
	bool triedThumbnail;
	bool isMouseInsideAuthor;
	bool isMouseInsideHistory;
	bool showVotes;
	ThumbnailRendererTask *thumbnailRenderer;
public:
	SaveButton(Point position, Point size, SaveInfo * save);
	SaveButton(Point position, Point size, SaveFile * file);
	virtual ~SaveButton();

	void OnMouseClick(int x, int y, unsigned int button) override;
	void OnMouseUnclick(int x, int y, unsigned int button) override;

	void OnMouseEnter(int x, int y) override;
	void OnMouseLeave(int x, int y) override;

	void OnMouseMovedInside(int x, int y, int dx, int dy) override;

	void AddContextMenu(int menuType);
	void OnContextMenuAction(int item) override;

	void Draw(const Point& screenPos) override;
	void Tick(float dt) override;

	void OnResponse(std::unique_ptr<VideoBuffer> thumbnail) override;

	void SetSelected(bool selected_) { selected = selected_; }
	bool GetSelected() { return selected; }
	void SetSelectable(bool selectable_) { selectable = selectable_; }
	bool GetSelectable() { return selectable; }
	void SetShowVotes(bool showVotes_) { showVotes = showVotes_; }

	SaveInfo * GetSave() { return save; }
	SaveFile * GetSaveFile() { return file; }
	inline bool GetState() { return state; }
	void DoAction();
	void DoAltAction();
	void DoAltAction2();
	void DoSelection();
	void SetActionCallback(SaveButtonAction * action);
protected:
	bool isButtonDown, state, isMouseInside, selected, selectable;
	SaveButtonAction * actionCallback;
};
}
#endif /* BUTTON_H_ */

