#pragma once
#include "common/String.h"

#include "Component.h"
#include "client/http/ThumbnailRequest.h"

#include <memory>
#include <functional>

class VideoBuffer;
class SaveFile;
class SaveInfo;
class ThumbnailRendererTask;
namespace ui
{
class SaveButton : public Component
{
	SaveFile *file = nullptr; // non-owning
	SaveInfo *save = nullptr; // non-owning
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

	std::unique_ptr<http::ThumbnailRequest> thumbnailRequest;

	struct SaveButtonAction
	{
		std::function<void ()> action, altAction, altAltAction, selected;
	};
	SaveButtonAction actionCallback;

	SaveButton(Point position, Point size);

public:
	SaveButton(Point position, Point size, SaveInfo *newSave /* non-owning */);
	SaveButton(Point position, Point size, SaveFile *newFile /* non-owning */);
	virtual ~SaveButton();

	void OnMouseClick(int x, int y, unsigned int button) override;
	void OnMouseDown(int x, int y, unsigned int button) override;

	void OnMouseEnter(int x, int y) override;
	void OnMouseLeave(int x, int y) override;

	void OnMouseMoved(int x, int y) override;

	void AddContextMenu(int menuType);
	void OnContextMenuAction(int item) override;

	void Draw(const Point& screenPos) override;
	void Tick() override;

	void SetSelected(bool selected_) { selected = selected_; }
	bool GetSelected() { return selected; }
	void SetSelectable(bool selectable_) { selectable = selectable_; }
	bool GetSelectable() { return selectable; }
	void SetShowVotes(bool showVotes_) { showVotes = showVotes_; }

	const SaveInfo *GetSave() const { return save; }
	const SaveFile *GetSaveFile() const { return file; }
	inline bool GetState() { return state; }
	void DoAction();
	void DoAltAction();
	void DoAltAction2();
	void DoSelection();
	inline void SetActionCallback(SaveButtonAction action) { actionCallback = action; }

	// TODO: clone the request instead because sometimes the user of CloneThumbnail might end up
	// with a nullptr even though the thumbnail for the SaveButton will eventually arrive.
	std::unique_ptr<VideoBuffer> CloneThumbnail() const;

protected:
	bool isButtonDown, state, isMouseInside, selected, selectable;
};
}
