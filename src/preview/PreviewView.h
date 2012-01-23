/*
 * PreviewView.h
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#ifndef PREVIEWVIEW_H_
#define PREVIEWVIEW_H_
#include "interface/Window.h"
#include "preview/PreviewController.h"
#include "preview/PreviewModel.h"
#include "interface/Button.h"
#include "search/Thumbnail.h"
#include "interface/Label.h"

class PreviewModel;
class PreviewController;
class PreviewView: public ui::Window {
	PreviewController * c;
	Thumbnail * savePreview;
	ui::Button * openButton;
	ui::Label * saveNameLabel;
public:
	void AttachController(PreviewController * controller) { c = controller;}
	PreviewView();
	void NotifyPreviewChanged(PreviewModel * sender);
	void NotifySaveChanged(PreviewModel * sender);
	virtual void OnDraw();
	virtual ~PreviewView();
};

#endif /* PREVIEWVIEW_H_ */
