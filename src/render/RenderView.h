/*
 * RenderView.h
 *
 *  Created on: Jan 23, 2012
 *      Author: Simon
 */

#ifndef RENDERVIEW_H_
#define RENDERVIEW_H_


#include <vector>
#include "interface/Window.h"
#include "RenderController.h"
#include "RenderModel.h"
#include "Renderer.h"
#include "interface/Checkbox.h"

class RenderController;
class RenderModel;
class RenderView: public ui::Window {
	RenderController * c;
	Renderer * ren;
	std::vector<ui::Checkbox*> renderModes;
public:
	class RenderModeAction;
	RenderView();
	void NotifyRendererChanged(RenderModel * sender);
	void AttachController(RenderController * c_) { c = c_; }
	virtual void OnDraw();
	virtual ~RenderView();
};

#endif /* RENDERVIEW_H_ */
