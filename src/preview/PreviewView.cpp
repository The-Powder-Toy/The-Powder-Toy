/*
 * PreviewView.cpp
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#include "PreviewView.h"
#include "interface/Point.h"
#include "interface/Window.h"
#include "search/Thumbnail.h"

PreviewView::PreviewView():
	ui::Window(ui::Point(-1, -1), ui::Point((XRES/2)+200, (YRES/2)+150)),
	savePreview(NULL)
{
	class OpenAction: public ui::ButtonAction
	{
		PreviewView * v;
	public:
		OpenAction(PreviewView * v_){ v = v_; }
		virtual void ActionCallback(ui::Button * sender)
		{
			v->c->DoOpen();
			v->c->Exit();
		}
	};
	openButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(100, 16), "Open");
	openButton->SetActionCallback(new OpenAction(this));
	AddComponent(openButton);

	saveNameLabel = new ui::Label(ui::Point(5, (YRES/2)+5), ui::Point(100, 16), "");
	saveNameLabel->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(saveNameLabel);
}

void PreviewView::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;

	//Window Background+Outline
	g->clearrect(Position.X-2, Position.Y-2, Size.X+4, Size.Y+4);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);

	//Save preview (top-left)
	if(savePreview && savePreview->Data)
	{
		g->draw_image(savePreview->Data, (Position.X+1)+(((XRES/2)-savePreview->Size.X)/2), (Position.Y+1)+(((YRES/2)-savePreview->Size.Y)/2), savePreview->Size.X, savePreview->Size.Y, 255);
	}
	g->drawrect(Position.X, Position.Y, XRES/2, YRES/2, 255, 255, 255, 100);
}

void PreviewView::OnMouseDown(int x, int y, unsigned button)
{
	if(!(x > Position.X && y > Position.Y && y < Position.Y+Size.Y && x < Position.X+Size.X)) //Clicked outside window
		c->Exit();
}

void PreviewView::NotifySaveChanged(PreviewModel * sender)
{
	Save * save = sender->GetSave();
	if(save)
	{
		saveNameLabel->SetText(save->name);
	}
	else
	{
		saveNameLabel->SetText("");
	}
}

void PreviewView::NotifyPreviewChanged(PreviewModel * sender)
{
	savePreview = sender->GetPreview();
	if(savePreview && savePreview->Data && !(savePreview->Size.X == XRES/2 && savePreview->Size.Y == YRES/2))
	{
		int newSizeX, newSizeY;
		float factorX = ((float)XRES/2)/((float)savePreview->Size.X);
		float factorY = ((float)YRES/2)/((float)savePreview->Size.Y);
		float scaleFactor = factorY < factorX ? factorY : factorX;
		savePreview->Data = Graphics::resample_img(savePreview->Data, savePreview->Size.X, savePreview->Size.Y, savePreview->Size.X*scaleFactor, savePreview->Size.Y*scaleFactor);
		savePreview->Size.X *= scaleFactor;
		savePreview->Size.Y *= scaleFactor;
	}
}

PreviewView::~PreviewView() {
	delete openButton;
	delete saveNameLabel;
}

