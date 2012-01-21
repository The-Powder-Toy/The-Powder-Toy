/*
 * PreviewController.cpp
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#include "PreviewController.h"
#include "PreviewView.h"
#include "PreviewModel.h"

PreviewController::PreviewController(int saveID) {
	// TODO Auto-generated constructor stub
	previewModel = new PreviewModel();
	previewView = new PreviewView();
	previewModel->AddObserver(previewView);
	previewView->AttachController(this);

	previewModel->UpdateSave(saveID);
}

PreviewController::~PreviewController() {
	delete previewView;
	delete previewModel;
}

