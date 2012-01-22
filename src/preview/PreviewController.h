/*
 * PreviewController.h
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#ifndef PREVIEWCONTROLLER_H_
#define PREVIEWCONTROLLER_H_

#include "preview/PreviewModel.h"
#include "preview/PreviewView.h"

class PreviewModel;
class PreviewView;
class PreviewController {
	PreviewModel * previewModel;
	PreviewView * previewView;
public:
	PreviewController(int saveID);
	PreviewView * GetView() { return previewView; }
	virtual ~PreviewController();
};

#endif /* PREVIEWCONTROLLER_H_ */
