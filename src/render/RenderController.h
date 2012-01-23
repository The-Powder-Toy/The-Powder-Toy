/*
 * RenderController.h
 *
 *  Created on: Jan 23, 2012
 *      Author: Simon
 */

#ifndef RENDERCONTROLLER_H_
#define RENDERCONTROLLER_H_

#include "RenderView.h"
#include "RenderModel.h"
#include "Renderer.h"

class RenderView;
class RenderModel;
class RenderController {
	RenderView * renderView;
	RenderModel * renderModel;
public:
	RenderController(Renderer * ren);
	RenderView * GetView() { return renderView; }
	virtual ~RenderController();
};

#endif /* RENDERCONTROLLER_H_ */
