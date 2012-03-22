/*
 * TagsView.cpp
 *
 *  Created on: Mar 5, 2012
 *      Author: Simon
 */

#include "TagsView.h"

#include "TagsController.h"
#include "TagsModel.h"

TagsView::TagsView():
	ui::Window(ui::Point(-1, -1), ui::Point(200, 300))
{
	submitButton = new ui::Button(ui::Point(Size.X-56, Size.Y-24), ui::Point(50, 16));
	AddComponent(submitButton);
	tagInput = new ui::Textbox(ui::Point(6, Size.Y-24), ui::Point(Size.X-80, 16), "");
	AddComponent(tagInput);
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

	if(sender->GetSave())
	{
		for(int i = 0; i < sender->GetSave()->GetTags().size(); i++)
		{
			ui::Label * tempLabel = new ui::Label(ui::Point(5, 10*i), ui::Point(50, 16), sender->GetSave()->GetTags()[i]);
			tags.push_back(tempLabel);
			AddComponent(tempLabel);
		}
	}
}

TagsView::~TagsView() {
	// TODO Auto-generated destructor stub
}

