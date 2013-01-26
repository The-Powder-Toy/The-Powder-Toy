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
#include "TagsModelException.h"

#include "interface/Button.h"
#include "interface/Textbox.h"
#include "interface/Label.h"
#include "interface/Keys.h"

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
	closeButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	closeButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	closeButton->SetActionCallback(new CloseAction(this));
	AddComponent(closeButton);


	tagInput = new ui::Textbox(ui::Point(8, Size.Y-40), ui::Point(Size.X-60, 16), "", "[new tag]");
	tagInput->Appearance.icon = IconTag;
	tagInput->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tagInput->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tagInput);

	class AddTagAction : public ui::ButtonAction
	{
		TagsView * v;
	public:
		AddTagAction(TagsView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->addTag();
		}
	};
	addButton = new ui::Button(ui::Point(tagInput->Position.X+tagInput->Size.X+4, tagInput->Position.Y), ui::Point(40, 16), "Add");
	addButton->Appearance.icon = IconAdd;
	addButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	addButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	addButton->SetActionCallback(new AddTagAction(this));
	AddComponent(addButton);

	title = new ui::Label(ui::Point(5, 5), ui::Point(185, 16), "Manage tags:");
	title->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	title->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	AddComponent(title);
}

void TagsView::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
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
		std::string tag;
	public:
		DeleteTagAction(TagsView * _v, std::string tag) { v = _v; this->tag = tag; }
		void ActionCallback(ui::Button * sender)
		{
			try
			{
				v->c->RemoveTag(tag);
			}
			catch(TagsModelException & ex)
			{
				new ErrorMessage("Could not remove tag", ex.what());
			}
		}
	};

	if(sender->GetSave())
	{
		for(int i = 0; i < sender->GetSave()->GetTags().size(); i++)
		{
			ui::Label * tempLabel = new ui::Label(ui::Point(35, 35+(16*i)), ui::Point(120, 16), sender->GetSave()->GetTags()[i]);
			tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;			tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
			tags.push_back(tempLabel);
			AddComponent(tempLabel);

			if(sender->GetSave()->GetUserName() == Client::Ref().GetAuthUser().Username || Client::Ref().GetAuthUser().UserElevation == User::ElevationAdmin || Client::Ref().GetAuthUser().UserElevation == User::ElevationModerator)
			{
				ui::Button * tempButton = new ui::Button(ui::Point(15, 37+(16*i)), ui::Point(11, 12));
				tempButton->Appearance.icon = IconDelete;
				tempButton->Appearance.Border = ui::Border(0);
				tempButton->Appearance.Margin.Top += 2;
				tempButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;	
				tempButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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
			addTag();
		}
		break;
	}
}

void TagsView::addTag()
{
	if (tagInput->GetText().length() < 4)
	{
		new ErrorMessage("Tag not long enough", "Must be at least 4 letters");
		return;
	}
	try
	{
		c->AddTag(tagInput->GetText());
	}
	catch(TagsModelException & ex)
	{
		new ErrorMessage("Could not add tag", ex.what());
	}
	tagInput->SetText("");
}

TagsView::~TagsView() {
	// TODO Auto-generated destructor stub
}

