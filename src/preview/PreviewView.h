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

class PreviewController;
class PreviewView: public ui::Window {
	PreviewController * c;
public:
	void AttachController(PreviewController * controller) { c = controller;}
	PreviewView();
	virtual void OnDraw();
	virtual ~PreviewView();
};

#endif /* PREVIEWVIEW_H_ */
