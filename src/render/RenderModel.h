/*
 * RenderModel.h
 *
 *  Created on: Jan 23, 2012
 *      Author: Simon
 */

#ifndef RENDERMODEL_H_
#define RENDERMODEL_H_

#include <vector>
#include "RenderView.h"
#include "Renderer.h"

using namespace std;

class RenderView;
class RenderModel {
	vector<RenderView*> observers;
	Renderer * renderer;
	void notifyRendererChanged();
public:
	RenderModel();
	Renderer * GetRenderer();
	void AddObserver(RenderView * observer);
	void SetRenderer(Renderer * ren);
	void SetRenderMode(unsigned int renderMode);
	void UnsetRenderMode(unsigned int renderMode);
	virtual ~RenderModel();
};

#endif /* RENDERMODEL_H_ */
