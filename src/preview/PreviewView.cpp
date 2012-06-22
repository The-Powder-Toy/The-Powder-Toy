/*
 * PreviewView.cpp
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#include <vector>
#include <cmath>
#include "PreviewView.h"
#include "dialogues/TextPrompt.h"
#include "simulation/SaveRenderer.h"
#include "interface/Point.h"
#include "interface/Window.h"
#include "Style.h"
#include "search/Thumbnail.h"

PreviewView::PreviewView():
	ui::Window(ui::Point(-1, -1), ui::Point((XRES/2)+200, (YRES/2)+150)),
	savePreview(NULL),
	doOpen(false),
	commentsOffset(0),
	commentsVel(0),
	maxOffset(0)
{
	class OpenAction: public ui::ButtonAction
	{
		PreviewView * v;
	public:
		OpenAction(PreviewView * v_){ v = v_; }
		virtual void ActionCallback(ui::Button * sender)
		{
			v->c->DoOpen();
		}
	};
	openButton = new ui::Button(ui::Point(0, Size.Y-19), ui::Point(51, 19), "Open");
	openButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	openButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	openButton->SetIcon(IconOpen);
	openButton->SetActionCallback(new OpenAction(this));
	AddComponent(openButton);

	class FavAction: public ui::ButtonAction
	{
		PreviewView * v;
	public:
		FavAction(PreviewView * v_){ v = v_; }
		virtual void ActionCallback(ui::Button * sender)
		{
			v->c->FavouriteSave();
		}
	};

	favButton = new ui::Button(ui::Point(50, Size.Y-19), ui::Point(51, 19), "Fav.");
	favButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	favButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	favButton->SetIcon(IconFavourite);
	favButton->SetActionCallback(new FavAction(this));
	AddComponent(favButton);

	class ReportPromptCallback: public TextDialogueCallback {
	public:
		PreviewView * v;
		ReportPromptCallback(PreviewView * v_) { v = v_;	}
		virtual void TextCallback(TextPrompt::DialogueResult result, std::string resultText) {
			if (result == TextPrompt::ResultOkay)
				v->c->Report(resultText);
		}
		virtual ~ReportPromptCallback() { }
	};

	class ReportAction: public ui::ButtonAction
	{
		PreviewView * v;
	public:
		ReportAction(PreviewView * v_){ v = v_; }
		virtual void ActionCallback(ui::Button * sender)
		{
			new TextPrompt("Report Save", "Reason for reporting", true, new ReportPromptCallback(v));
		}
	};
	reportButton = new ui::Button(ui::Point(100, Size.Y-19), ui::Point(51, 19), "Report");
	reportButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	reportButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	reportButton->SetIcon(IconReport);
	reportButton->SetActionCallback(new ReportAction(this));
	AddComponent(reportButton);

	class BrowserOpenAction: public ui::ButtonAction
	{
		PreviewView * v;
	public:
		BrowserOpenAction(PreviewView * v_){ v = v_; }
		virtual void ActionCallback(ui::Button * sender)
		{
			v->c->OpenInBrowser();
		}
	};

	browserOpenButton = new ui::Button(ui::Point((XRES/2)-107, Size.Y-19), ui::Point(108, 19), "Open in browser");
	browserOpenButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	browserOpenButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	browserOpenButton->SetIcon(IconOpen);
	browserOpenButton->SetActionCallback(new BrowserOpenAction(this));
	AddComponent(browserOpenButton);

	saveNameLabel = new ui::Label(ui::Point(5, (YRES/2)+15), ui::Point(100, 16), "");
	saveNameLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	saveNameLabel->Appearance.VerticalAlign = ui::Appearance::AlignBottom;
	AddComponent(saveNameLabel);

	saveDescriptionTextblock = new ui::Textblock(ui::Point(5, (YRES/2)+15+14+17), ui::Point((XRES/2)-10, Size.Y-((YRES/2)+15+14+17)-21), "");
	saveDescriptionTextblock->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	saveDescriptionTextblock->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	saveDescriptionTextblock->SetTextColour(ui::Colour(180, 180, 180));
	AddComponent(saveDescriptionTextblock);

	authorDateLabel = new ui::Label(ui::Point(5, (YRES/2)+15+14), ui::Point(100, 16), "");
	authorDateLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	authorDateLabel->Appearance.VerticalAlign = ui::Appearance::AlignBottom;
	AddComponent(authorDateLabel);
}

void PreviewView::DoDraw()
{
	Window::DoDraw();
	Graphics * g = ui::Engine::Ref().g;
	if(c->GetDoOpen())
	{
		g->fillrect(Position.X+(Size.X/2)-101, Position.Y+(Size.Y/2)-26, 202, 52, 0, 0, 0, 210);
		g->drawrect(Position.X+(Size.X/2)-100, Position.Y+(Size.Y/2)-25, 200, 50, 255, 255, 255, 180);
		g->drawtext(Position.X+(Size.X/2)-(Graphics::textwidth("Loading save...")/2), Position.Y+(Size.Y/2)-5, "Loading save...", style::Colour::InformationTitle.Red, style::Colour::InformationTitle.Green, style::Colour::InformationTitle.Blue, 255);
	}
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
	g->drawrect(Position.X, Position.Y, (XRES/2)+1, (YRES/2)+1, 255, 255, 255, 100);
	g->draw_line(Position.X+XRES/2, Position.Y+1, Position.X+XRES/2, Position.Y+Size.Y-2, 200, 200, 200, 255);


	g->draw_line(Position.X+1, Position.Y+12+YRES/2, Position.X-1+XRES/2, Position.Y+12+YRES/2, 100, 100, 100,255);
	float factor;
	if(!(!votesUp && !votesDown))
	{
		factor = (float)(((float)(XRES/2)-2)/((float)(votesUp+votesDown)));
		g->fillrect(1+Position.X, 2+Position.Y+YRES/2, (XRES/2)-2, 9, 200, 50, 50, 255);
		g->fillrect(1+Position.X, 2+Position.Y+YRES/2, (int)(((float)votesUp)*factor), 9, 50, 200, 50, 255);
		g->fillrect(1+Position.X, 2+Position.Y+(YRES/2), 14, 9, 0, 0, 0, 100);
		g->fillrect(Position.X+(XRES/2)-15, 2+Position.Y+(YRES/2), 14, 9, 0, 0, 0, 100);
		g->draw_icon(1+Position.X+2, Position.Y+(YRES/2)+4, IconVoteUp);
		g->draw_icon(Position.X+(XRES/2)-12, Position.Y+(YRES/2)+1, IconVoteDown);
	}

	for(int i = 0; i < commentTextComponents.size(); i++)
	{
		g->draw_line(
				Position.X+XRES/2,
				Position.Y+commentTextComponents[i]->Position.Y+commentTextComponents[i]->Size.Y+4,
				Position.X+Size.X-1,
				Position.Y+commentTextComponents[i]->Position.Y+commentTextComponents[i]->Size.Y+4,
				100, 100, 100, 255);
	}
}

void PreviewView::OnTick(float dt)
{
	if(commentsVel > 5.0f) commentsVel = 5.0f;
	if(commentsVel < -5.0f) commentsVel = -5.0f;
	if(commentsVel > -0.5f && commentsVel < 0.5)
		commentsVel = 0;

	int oldOffset = commentsOffset;
	commentsOffset += commentsVel;

	commentsVel*=0.99f;

	if(oldOffset!=int(commentsOffset))
	{
		if(commentsOffset<0)
		{
			commentsOffset = 0;
			commentsVel = 0;
		}
		if(commentsOffset>maxOffset)
		{
			commentsOffset = maxOffset;
			commentsVel = 0;
		}
		displayComments(commentsOffset);
	}

	c->Update();
}

void PreviewView::OnMouseDown(int x, int y, unsigned button)
{
	if(!(x > Position.X && y > Position.Y && y < Position.Y+Size.Y && x < Position.X+Size.X)) //Clicked outside window
		c->Exit();
}

void PreviewView::NotifySaveChanged(PreviewModel * sender)
{
	SaveInfo * save = sender->GetSave();
	if(savePreview)
		delete savePreview;
	savePreview = NULL;
	if(save)
	{
		votesUp = save->votesUp;
		votesDown = save->votesDown;
		saveNameLabel->SetText(save->name);
		authorDateLabel->SetText("\bgAuthor:\bw " + save->userName + " \bgDate:\bw ");
		saveDescriptionTextblock->SetText(save->Description);
		if(save->Favourite)
			favButton->Enabled = false;
		else
			favButton->Enabled = true;

		if(save->GetGameSave())
		{
			savePreview = SaveRenderer::Ref().Render(save->GetGameSave());

			if(savePreview && savePreview->Data && !(savePreview->Size.X == XRES/2 && savePreview->Size.Y == YRES/2))
			{
				int newSizeX, newSizeY;
				pixel * oldData = savePreview->Data;
				float factorX = ((float)XRES/2)/((float)savePreview->Size.X);
				float factorY = ((float)YRES/2)/((float)savePreview->Size.Y);
				float scaleFactor = factorY < factorX ? factorY : factorX;
				savePreview->Data = Graphics::resample_img(oldData, savePreview->Size.X, savePreview->Size.Y, savePreview->Size.X*scaleFactor, savePreview->Size.Y*scaleFactor);
				free(oldData);
				savePreview->Size.X *= scaleFactor;
				savePreview->Size.Y *= scaleFactor;
			}
		}
	}
	else
	{
		votesUp = 0;
		votesDown = 0;
		saveNameLabel->SetText("");
		authorDateLabel->SetText("");
		saveDescriptionTextblock->SetText("");
		favButton->Enabled = false;
	}
}

void PreviewView::displayComments(int yOffset)
{
	for(int i = 0; i < commentComponents.size(); i++)
	{
		RemoveComponent(commentComponents[i]);
		delete commentComponents[i];
	}
	commentComponents.clear();
	commentTextComponents.clear();

	int currentY = -yOffset;
	ui::Label * tempUsername;
	ui::Textblock * tempComment;
	for(int i = 0; i < comments.size(); i++)
	{
		int usernameY = currentY+5, commentY;
		tempUsername = new ui::Label(ui::Point((XRES/2) + 5, currentY+5), ui::Point(Size.X-((XRES/2) + 10), 16), comments[i].authorName);
		tempUsername->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;			tempUsername->Appearance.VerticalAlign = ui::Appearance::AlignBottom;
		currentY += 16;

		if(currentY > Size.Y || usernameY < 0)
		{
			delete tempUsername;
			if(currentY > Size.Y)
				break;
		}
		else
		{
			commentComponents.push_back(tempUsername);
			AddComponent(tempUsername);
		}

		commentY = currentY+5;
		tempComment = new ui::Textblock(ui::Point((XRES/2) + 5, currentY+5), ui::Point(Size.X-((XRES/2) + 10), -1), comments[i].comment);
		tempComment->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;			tempComment->Appearance.VerticalAlign = ui::Appearance::AlignTop;
		tempComment->SetTextColour(ui::Colour(180, 180, 180));
		currentY += tempComment->Size.Y+4;

		if(currentY > Size.Y || commentY < 0)
		{
			delete tempComment;
			if(currentY > Size.Y)
				break;
		}
		else
		{
			commentComponents.push_back(tempComment);
			AddComponent(tempComment);
			commentTextComponents.push_back(tempComment);
		}
	}
}

void PreviewView::NotifyCommentsChanged(PreviewModel * sender)
{
	if(sender->GetComments())
	{
		comments = std::vector<SaveComment>(sender->GetComments()->begin(), sender->GetComments()->end());
	}
	else
	{
		comments.clear();
	}

	ui::Label * tempUsername;
	ui::Textblock * tempComment;
	int maxY = 0;
	for(int i = 0; i < comments.size(); i++)
	{
		tempUsername = new ui::Label(ui::Point(0, 0), ui::Point(Size.X-((XRES/2) + 10), 16), comments[i].authorName);
		tempUsername->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		tempUsername->Appearance.VerticalAlign = ui::Appearance::AlignBottom;
		maxY += 16;
		tempComment = new ui::Textblock(ui::Point(0, 0), ui::Point(Size.X-((XRES/2) + 10), -1), comments[i].comment);
		tempComment->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		tempComment->Appearance.VerticalAlign = ui::Appearance::AlignTop;
		tempComment->SetTextColour(ui::Colour(180, 180, 180));
		maxY += tempComment->Size.Y+4;

		delete tempUsername;
		delete tempComment;
	}


	maxOffset = (maxY-Size.Y)+16;
	commentsOffset = 0;
	commentsVel = 0;
	displayComments(commentsOffset);
}

void PreviewView::OnMouseWheel(int x, int y, int d)
{
	commentsVel-=d;
	/*if(!d)
		return;
	if(d<0)
		c->NextPage();
	else
		c->PrevPage();*/
}

/*void PreviewView::NotifyPreviewChanged(PreviewModel * sender)
{
	savePreview = sender->GetGameSave();
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
}*/

PreviewView::~PreviewView() {
}

