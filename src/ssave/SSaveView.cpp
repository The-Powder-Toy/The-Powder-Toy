/*
 * SSaveView.cpp
 *
 *  Created on: Jan 29, 2012
 *      Author: Simon
 */

#include "SSaveView.h"

SSaveView::SSaveView():
	ui::Window(ui::Point(-1, -1), ui::Point(200, 200)),
	publishCheckbox(NULL),
	saveButton(NULL),
	closeButton(NULL),
	nameField(NULL),
	titleLabel(NULL),
	descriptionField(NULL)
{
	titleLabel = new ui::Label(ui::Point(2, 1), ui::Point(Size.X-4, 16), "Save to Server");
	titleLabel->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(titleLabel);

	nameField = new ui::Textbox(ui::Point(4, 18), ui::Point(Size.X-8, 16), "");
	nameField->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(nameField);

	descriptionField = new ui::Textarea(ui::Point(4, 54), ui::Point(Size.X-8, Size.Y-26-54), "");
	AddComponent(descriptionField);

	publishCheckbox = new ui::Checkbox(ui::Point(4, 36), ui::Point(Size.X-8, 16), "Publish");
	AddComponent(publishCheckbox);

	class CloseAction: public ui::ButtonAction
	{
		SSaveView * v;
	public:
		CloseAction(SSaveView * v_) { v = v_; };
		void ActionCallback(ui::Button * sender)
		{
			v->c->Exit();
		}
	};
	closeButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(50, 16), "Cancel");
	closeButton->SetActionCallback(new CloseAction(this));
	AddComponent(closeButton);

	class SaveAction: public ui::ButtonAction
	{
		SSaveView * v;
	public:
		SaveAction(SSaveView * v_) { v = v_; };
		void ActionCallback(ui::Button * sender)
		{
			v->c->UploadSave(v->nameField->GetText(), "", v->publishCheckbox->GetChecked());
		}
	};
	saveButton = new ui::Button(ui::Point(Size.X-50, Size.Y-16), ui::Point(50, 16), "Save");
	saveButton->SetActionCallback(new SaveAction(this));
	AddComponent(saveButton);
}

void SSaveView::NotifySaveChanged(SSaveModel * sender)
{
	if(sender->GetSave())
	{
		nameField->SetText(sender->GetSave()->GetName());
		publishCheckbox->SetChecked(sender->GetSave()->Published);
	}
	else
	{
		nameField->SetText("");
		//publishCheckbox->SetChecked(sender->GetSave()->GetPublished());
	}
}

void SSaveView::NotifySaveUploadChanged(SSaveModel * sender)
{
	if(sender->GetSaveUploaded())
		c->Exit();
}

void SSaveView::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;

	g->clearrect(Position.X, Position.Y, Size.X, Size.Y);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

SSaveView::~SSaveView() {
}

