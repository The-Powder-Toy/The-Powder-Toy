/*
 * TagsView.cpp
 *
 *  Created on: Mar 5, 2012
 *      Author: Simon
 */

#include "client/Client.h"
#include "TagsView.h"

#include "dialogues/ErrorMessage.h"
#include "TagsController.h"
#include "TagsModel.h"

TagsView::TagsView():
	ui::Window(ui::Point(-1, -1), ui::Point(195, 250))
{

	class CloseAction : public ui::ButtonAction
	{
		TagsView * v;
	public:
		CloseAction(TagsView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->Exit();
		}
	};
	closeButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(195, 16), "Close");
	closeButton->SetAlignment(AlignLeft, AlignTop);
	closeButton->SetActionCallback(new CloseAction(this));
	AddComponent(closeButton);

	tagInput = new ui::Textbox(ui::Point(8, Size.Y-40), ui::Point(Size.X-16, 16), "");
	AddComponent(tagInput);

	title = new ui::Label(ui::Point(5, 5), ui::Point(185, 16), "Manage tags:");
	title->SetAlignment(AlignLeft, AlignTop);
	AddComponent(title);
}

void TagsView::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
}

void TagsView::NotifyError(TagsModel * sender)
{
	new ErrorMessage("Error", sender->GetLastError());
}

void TagsView::NotifyTagsChanged(TagsModel * sender)
{
	for(int i = 0; i < tags.size(); i++)
	{
		RemoveComponent(tags[i]);
		delete tags[i];
	}
	tags.clear();


	class DeleteTagAction : public ui::ButtonAction
	{
		TagsView * v;
		string tag;
	public:
		DeleteTagAction(TagsView * _v, string tag) { v = _v; this->tag = tag; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->RemoveTag(tag);
		}
	};

	if(sender->GetSave())
	{
		for(int i = 0; i < sender->GetSave()->GetTags().size(); i++)
		{
			ui::Label * tempLabel = new ui::Label(ui::Point(35, 35+(16*i)), ui::Point(120, 16), sender->GetSave()->GetTags()[i]);
			tempLabel->SetAlignment(AlignLeft, AlignMiddle);
			tags.push_back(tempLabel);
			AddComponent(tempLabel);

			if(sender->GetSave()->GetUserName()==Client::Ref().GetAuthUser().Username)
			{
				ui::Button * tempButton = new ui::Button(ui::Point(15, 35+(16*i)), ui::Point(14, 14), "x");
				tempButton->SetAlignment(AlignCentre, AlignMiddle);
				tempButton->SetActionCallback(new DeleteTagAction(this, sender->GetSave()->GetTags()[i]));
				tags.push_back(tempButton);
				AddComponent(tempButton);
			}
		}
	}
}

void TagsView::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	switch(key)
	{
	/*case KEY_TAB:
		if(IsFocused(usernameField))
			FocusComponent(passwordField);
		else
			FocusComponent(usernameField);
		break;*/
	case KEY_ENTER:
	case KEY_RETURN:
		if(IsFocused(tagInput))
		{
			c->AddTag(tagInput->GetText());
			tagInput->SetText("");
		}
		break;
	}
}

TagsView::~TagsView() {
	// TODO Auto-generated destructor stub
}

