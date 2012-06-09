/*
 * PreviewView.h
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#ifndef PREVIEWVIEW_H_
#define PREVIEWVIEW_H_

#include <vector>
#include "interface/Window.h"
#include "preview/PreviewController.h"
#include "preview/PreviewModel.h"
#include "interface/Button.h"
#include "search/Thumbnail.h"
#include "interface/Label.h"
#include "interface/Textblock.h"

class PreviewModel;
class PreviewController;
class PreviewView: public ui::Window {
	PreviewController * c;
	Thumbnail * savePreview;
	ui::Button * openButton;
	ui::Button * browserOpenButton;
	ui::Button * favButton;
	ui::Button * reportButton;
	ui::Label * saveNameLabel;
	ui::Label * authorDateLabel;
	ui::Textblock * saveDescriptionTextblock;
	std::vector<ui::Component*> commentComponents;
	std::vector<ui::Component*> commentTextComponents;
	int votesUp;
	int votesDown;
	bool doOpen;
public:
	void AttachController(PreviewController * controller) { c = controller;}
	PreviewView();
	void NotifySaveChanged(PreviewModel * sender);
	void NotifyCommentsChanged(PreviewModel * sender);
	virtual void OnDraw();
	virtual void DoDraw();
	virtual void OnTick(float dt);
	virtual void OnMouseDown(int x, int y, unsigned button);
	virtual ~PreviewView();
};

#endif /* PREVIEWVIEW_H_ */
