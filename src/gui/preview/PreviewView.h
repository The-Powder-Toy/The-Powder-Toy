#ifndef PREVIEWVIEW_H_
#define PREVIEWVIEW_H_

#include <vector>
#include <set>
#include "common/String.h"
#include "Comment.h"
#include "gui/interface/Window.h"
#include "gui/preview/PreviewController.h"
#include "gui/preview/PreviewModel.h"
#include "gui/interface/Button.h"
#include "gui/interface/CopyTextButton.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"

namespace ui
{
	class ScrollPanel;
	class AvatarButton;
}

class VideoBuffer;
class PreviewModel;
class PreviewController;
class PreviewView: public ui::Window {
	class SubmitCommentAction;
	class LoginAction;
	class AutoCommentSizeAction;
	class AvatarAction;
	PreviewController * c;
	VideoBuffer * savePreview;
	ui::Button * openButton;
	ui::Button * browserOpenButton;
	ui::Button * favButton;
	ui::Button * reportButton;
	ui::Button * submitCommentButton;
	ui::Textbox * addCommentBox;
	ui::Label * commentWarningLabel;
	ui::Label * saveNameLabel;
	ui::Label * authorDateLabel;
	ui::AvatarButton * avatarButton;
	ui::Label * pageInfo;
	ui::Label * saveDescriptionLabel;
	ui::Label * viewsLabel;
	ui::Label * saveIDLabel;
	ui::Label * saveIDLabel2;
	ui::CopyTextButton * saveIDButton;
	ui::ScrollPanel * commentsPanel;
	std::vector<ui::Component*> commentComponents;
	std::vector<ui::Component*> commentTextComponents;
	int votesUp;
	int votesDown;
	bool userIsAuthor;
	bool doOpen;
	bool doError;
	String doErrorMessage;
	bool showAvatars;
	bool prevPage;

	int commentBoxHeight;
	float commentBoxPositionX;
	float commentBoxPositionY;
	float commentBoxSizeX;
	float commentBoxSizeY;
	bool commentHelpText;

	std::set<String> swearWords;

	void displayComments();
	void commentBoxAutoHeight();
	void submitComment();
	bool CheckSwearing(String text);
	void CheckComment();
public:
	void AttachController(PreviewController * controller);
	PreviewView();
	void NotifySaveChanged(PreviewModel * sender);
	void NotifyCommentsChanged(PreviewModel * sender);
	void NotifyCommentsPageChanged(PreviewModel * sender);
	void NotifyCommentBoxEnabledChanged(PreviewModel * sender);
	void SaveLoadingError(String errorMessage);
	void OnDraw() override;
	void DoDraw() override;
	void OnTick(float dt) override;
	void OnTryExit(ExitMethod method) override;
	void OnMouseWheel(int x, int y, int d) override;
	void OnMouseUp(int x, int y, unsigned int button) override;
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	virtual ~PreviewView();
};

#endif /* PREVIEWVIEW_H_ */
