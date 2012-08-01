/*
 * SSaveView.h
 *
 *  Created on: Jan 29, 2012
 *      Author: Simon
 */

#ifndef SSAVEVIEW_H_
#define SSAVEVIEW_H_

#include "interface/Window.h"
#include "interface/Label.h"
#include "interface/Button.h"
#include "interface/Textbox.h"
#include "interface/Textarea.h"
#include "interface/Checkbox.h"
#include "SSaveModel.h"
#include "SSaveController.h"

class SSaveController;
class SSaveModel;
class SSaveView: public ui::Window {
	SSaveController * c;
	ui::Checkbox * publishCheckbox;
	ui::Button * saveButton;
	ui::Button * closeButton;
	ui::Textbox * nameField;
	ui::Label * titleLabel;
	ui::Textarea * descriptionField;
public:
	SSaveView();
	void AttachController(SSaveController * c_) { c = c_; };
	void NotifySaveChanged(SSaveModel * sender);
	void NotifySaveUploadChanged(SSaveModel * sender);
	virtual void OnDraw();
	virtual ~SSaveView();
};

#endif /* SSAVEVIEW_H_ */
