#include <cmath>
#include <iostream>
#include <typeinfo>
#include "ThumbRenderRequest.h"
#include "client/GameSave.h"
#include "graphics/Graphics.h"
#include "simulation/SaveRenderer.h"

ThumbRenderRequest::ThumbRenderRequest(GameSave * save, bool decorations, bool fire, int width, int height, bool autoRescale, ListenerHandle listener, int identifier):
	RequestBroker::Request(ThumbnailRender, listener, identifier),
	Save(save),
	Width(width),
	Height(height),
	Decorations(decorations),
	Fire(fire),
	autoRescale(autoRescale)
{

}

RequestBroker::ProcessResponse ThumbRenderRequest::Process(RequestBroker & rb)
{
	VideoBuffer * thumbnail = SaveRenderer::Ref().Render(Save, Decorations, Fire);

	delete Save;
	Save = NULL;

	if (thumbnail)
	{
		if (!autoRescale)
			thumbnail->Resize(Width, Height, true);
		else
		{
			int scaleX = (int)std::ceil((float)thumbnail->Width / Width);
			int scaleY = (int)std::ceil((float)thumbnail->Height / Height);
			int scale = scaleX > scaleY ? scaleX : scaleY;
			int newWidth = thumbnail->Width / scale, newHeight = thumbnail->Height / scale;
			thumbnail->Resize(newWidth, newHeight, true);
			Width = newWidth;
			Height = newHeight;
		}
		ResultObject = (void*)thumbnail;
		rb.requestComplete((Request*)this);
		return RequestBroker::Finished;
	}
	else
	{
		return RequestBroker::Failed;
	}
	return RequestBroker::Failed;
}

ThumbRenderRequest::~ThumbRenderRequest()
{
	delete Save;
}

void ThumbRenderRequest::Cleanup()
{
	Request::Cleanup();
	if(ResultObject)
	{
		delete ((VideoBuffer*)ResultObject);
		ResultObject = NULL;
	}
}
