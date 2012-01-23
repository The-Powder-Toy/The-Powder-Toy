/*
 * RenderController.cpp
 *
 *  Created on: Jan 23, 2012
 *      Author: Simon
 */

#include "RenderController.h"

RenderController::RenderController(Renderer * ren) {
	renderView = new RenderView();
	renderModel = new RenderModel();

	renderView->AttachController(this);
	renderModel->AddObserver(renderView);

	renderModel->SetRenderer(ren);
}

RenderController::~RenderController() {
	delete renderView;
	delete renderModel;
}

