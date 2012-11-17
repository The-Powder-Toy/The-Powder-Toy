/*
 * PreviewView.h
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#ifndef PREVIEWVIEW_H_
#define PREVIEWVIEW_H_

#include <vector>
#include "Comment.h"
#include "interface/Window.h"
#include "preview/PreviewController.h"
#include "preview/PreviewModel.h"
#include "interface/Button.h"
#include "search/Thumbnail.h"
#include "interface/Label.h"
#include "interface/Textbox.h"

namespace ui
{
	class ScrollPanel;
}

class PreviewModel;
class PreviewController;
class PreviewView: public ui::Window {
	class SubmitCommentAction;
	class LoginAction;
	class AutoCommentSizeAction;
	PreviewController * c;
	Thumbnail * savePreview;
	ui::Button * openButton;
	ui::Button * browserOpenButton;
	ui::Button * favButton;
	ui::Button * reportButton;
	ui::Button * submitCommentButton;
	ui::Textbox * addCommentBox;
	ui::Label * saveNameLabel;
	ui::Label * authorDateLabel;
	ui::Label * pageInfo;
	ui::Label * saveDescriptionLabel;
	ui::Label * viewsLabel;
	ui::Textbox * saveIDTextbox;
	ui::ScrollPanel * commentsPanel;
	std::vector<SaveComment> comments;
	std::vector<ui::Component*> commentComponents;
	std::vector<ui::Component*> commentTextComponents;
	int votesUp;
	int votesDown;
	bool doOpen;

	int commentBoxHeight;
	float commentBoxPositionX;
	float commentBoxPositionY;
	float commentBoxSizeX;
	float commentBoxSizeY;

	void displayComments();
	void commentBoxAutoHeight();
	void submitComment();
public:
	void AttachController(PreviewController * controller);
	PreviewView();
	void NotifySaveChanged(PreviewModel * sender);
	void NotifyCommentsChanged(PreviewModel * sender);
	void NotifyCommentsPageChanged(PreviewModel * sender);
	void NotifyCommentBoxEnabledChanged(PreviewModel * sender);
	virtual void OnDraw();
	virtual void DoDraw();
	virtual void OnTick(float dt);
	virtual void OnTryExit(ExitMethod method);
	virtual void OnMouseWheel(int x, int y, int d);
	virtual ~PreviewView();
};

#endif /* PREVIEWVIEW_H_ */
