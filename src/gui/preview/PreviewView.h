#ifndef PREVIEWVIEW_H_
#define PREVIEWVIEW_H_

#include <vector>
#include <set>
#include <string>
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
	std::string doErrorMessage;
	bool showAvatars;
	bool prevPage;

	int commentBoxHeight;
	float commentBoxPositionX;
	float commentBoxPositionY;
	float commentBoxSizeX;
	float commentBoxSizeY;
	bool commentHelpText;
	
	std::set<std::string> swearWords;

	void displayComments();
	void commentBoxAutoHeight();
	void submitComment();
	bool CheckSwearing(std::string text);
	void CheckComment();
public:
	void AttachController(PreviewController * controller);
	PreviewView();
	void NotifySaveChanged(PreviewModel * sender);
	void NotifyCommentsChanged(PreviewModel * sender);
	void NotifyCommentsPageChanged(PreviewModel * sender);
	void NotifyCommentBoxEnabledChanged(PreviewModel * sender);
	void SaveLoadingError(std::string errorMessage);
	virtual void OnDraw();
	virtual void DoDraw();
	virtual void OnTick(float dt);
	virtual void OnTryExit(ExitMethod method);
	virtual void OnMouseWheel(int x, int y, int d);
	virtual void OnMouseUp(int x, int y, unsigned int button);
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual ~PreviewView();
};

#endif /* PREVIEWVIEW_H_ */
