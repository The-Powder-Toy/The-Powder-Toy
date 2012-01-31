/*
 * RenderModel.cpp
 *
 *  Created on: Jan 23, 2012
 *      Author: Simon
 */

#include "RenderModel.h"

RenderModel::RenderModel():
	renderer(NULL)
{
	try
	{
		json::Number tempNumber = Client::Ref().configDocument["Renderer"]["ColourMode"];
		if(tempNumber.Value())
			renderer->SetColourMode(tempNumber.Value());

		json::Array tempArray = Client::Ref().configDocument["Renderer"]["DisplayModes"];
		if(tempArray.Size())
		{
			std::vector<unsigned int> displayModes;
			json::Array::const_iterator itDisplayModes(tempArray.Begin()), itDisplayModesEnd(tempArray.End());
			for (; itDisplayModes != itDisplayModesEnd; ++itDisplayModes)
			{
				json::Number tempNumberI = *itDisplayModes;
				displayModes.push_back(tempNumberI.Value());
			}
		}

		tempArray = Client::Ref().configDocument["Renderer"]["RenderModes"];
		if(tempArray.Size())
		{
			std::vector<unsigned int> renderModes;
			json::Array::const_iterator itRenderModes(tempArray.Begin()), itRenderModesEnd(tempArray.End());
			for (; itRenderModes != itRenderModesEnd; ++itRenderModes)
			{
				json::Number tempNumberI = *itRenderModes;
				renderModes.push_back(tempNumberI.Value());
			}
		}
	}
	catch(json::Exception & e)
	{

	}
}

void RenderModel::AddObserver(RenderView * observer)
{
	observers.push_back(observer);
	observer->NotifyRendererChanged(this);
	observer->NotifyRenderChanged(this);
	observer->NotifyDisplayChanged(this);
	observer->NotifyColourChanged(this);
}

void RenderModel::SetRenderMode(unsigned int renderMode)
{
	if(renderer)
		renderer->AddRenderMode(renderMode);
	notifyRenderChanged();
}

void RenderModel::UnsetRenderMode(unsigned int renderMode)
{
	if(renderer)
		renderer->RemoveRenderMode(renderMode);
	notifyRenderChanged();
}

unsigned int RenderModel::GetRenderMode()
{
	if(renderer)
		return renderer->render_mode;
	else
		return 0;
}

void RenderModel::SetDisplayMode(unsigned int displayMode)
{
	if(renderer)
		renderer->AddDisplayMode(displayMode);
	notifyDisplayChanged();
}

void RenderModel::UnsetDisplayMode(unsigned int displayMode)
{
	if(renderer)
		renderer->RemoveDisplayMode(displayMode);
	notifyDisplayChanged();
}

unsigned int RenderModel::GetDisplayMode()
{
	if(renderer)
		return renderer->display_mode;
	else
		return 0;
}

void RenderModel::SetColourMode(unsigned int colourMode)
{
	if(renderer)
		renderer->SetColourMode(colourMode);
	notifyColourChanged();
}

unsigned int RenderModel::GetColourMode()
{
	if(renderer)
		return renderer->colour_mode;
	else
		return 0;
}

void RenderModel::SetRenderer(Renderer * ren)
{
	renderer = ren;
	notifyRendererChanged();
	notifyRenderChanged();
	notifyDisplayChanged();
	notifyColourChanged();
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

void RenderModel::notifyRenderChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyRenderChanged(this);
	}
}

void RenderModel::notifyDisplayChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyDisplayChanged(this);
	}
}

void RenderModel::notifyColourChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyColourChanged(this);
	}
}

RenderModel::~RenderModel() {
	// TODO Auto-generated destructor stub
}
