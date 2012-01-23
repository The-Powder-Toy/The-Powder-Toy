/*
 * RenderModel.cpp
 *
 *  Created on: Jan 23, 2012
 *      Author: Simon
 */

#include "RenderModel.h"

RenderModel::RenderModel() {
	// TODO Auto-generated constructor stub

}

void RenderModel::AddObserver(RenderView * observer)
{
	observers.push_back(observer);
	observer->NotifyRendererChanged(this);
}

void RenderModel::SetRenderer(Renderer * ren)
{
	renderer = ren;
	notifyRendererChanged();
}

Renderer * RenderModel::GetRenderer()
{
	return renderer;
}

void RenderModel::notifyRendererChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyRendererChanged(this);
	}
}

RenderModel::~RenderModel() {
	// TODO Auto-generated destructor stub
}
