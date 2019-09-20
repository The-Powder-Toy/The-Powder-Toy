#include "KeyconfigView.h"

#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/DropDown.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Checkbox.h"
#include "gui/interface/ScrollPanel.h"
#include "gui/Style.h"
#include "graphics/Graphics.h"
#include "KeyconfigMap.h"
#include "KeyconfigTextbox.h"
#include "KeyconfigModel.h"
#include "KeyconfigController.h"
#include "client/Client.h"
#include <vector>
#include <algorithm>

KeyconfigView::KeyconfigView() :
	ui::Window(ui::Point(-1, -1), ui::Point(320, 340)) {
	
	ui::Label * tempLabel = new ui::Label(ui::Point(4, 1), ui::Point(Size.X / 2, 22), "Keyboard Bindings");
	tempLabel->SetTextColour(style::Colour::InformationTitle);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class ResetDefaultsAction: public ui::ButtonAction
	{
	public:
		KeyconfigView * v;
		ResetDefaultsAction(KeyconfigView * v_) { v = v_; }
		void ActionCallback(ui::Button * sender) override
		{
			v->c->ResetToDefaults();
			v->BuildKeyBindingsListView();
			v->c->NotifyBindingsChanged();
		}
	};

	ui::Button* resetDefaults = new ui::Button(ui::Point(Size.X - 150, 5), ui::Point(140, 18), "Reset to Defaults");
	resetDefaults->SetActionCallback(new ResetDefaultsAction(this));
	AddComponent(resetDefaults);

	conflictLabel = new ui::Label(ui::Point(4, resetDefaults->Size.Y + 10), ui::Point(Size.X / 2, 18), "Please resolve conflicts or empty bindings");
	conflictLabel->SetTextColour(style::Colour::ErrorTitle);
	conflictLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	conflictLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	conflictLabel->Visible = false;
	AddComponent(conflictLabel);

	class CloseAction: public ui::ButtonAction
	{
	public:
		KeyconfigView * v;
		CloseAction(KeyconfigView * v_) { v = v_; }
		void ActionCallback(ui::Button * sender) override
		{
			v->c->Save();
			v->c->Exit();
		}
	};

	okayButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "OK");
	okayButton->SetActionCallback(new CloseAction(this));
	AddComponent(okayButton);
	SetCancelButton(okayButton);
	SetOkayButton(okayButton);
	scrollPanel = new ui::ScrollPanel(ui::Point(1, 50), ui::Point(Size.X-2, Size.Y-70));
	AddComponent(scrollPanel);
}

void KeyconfigView::ClearScrollPanel()
{
	int count = scrollPanel->GetChildCount();

	for (int i = 0; i < count; i++)
	{
		auto com = scrollPanel->GetChild(i);
		scrollPanel->RemoveChild(com);
		RemoveComponent(com);
	}

	RemoveComponent(scrollPanel);
	textboxes.clear();
}

void KeyconfigView::BuildKeyBindingsListView()
{
	int currentY = 0;
	float scrollPos = scrollPanel->GetScrollPositionY();
	ClearScrollPanel();

	scrollPanel = new ui::ScrollPanel(ui::Point(1, 50), ui::Point(Size.X-2, Size.Y-70));
	AddComponent(scrollPanel);

	std::vector<BindingModel> bindingModel = c->GetBindingPrefs();
	std::sort(bindingModel.begin(), bindingModel.end());

	for (int i = 0; i < (int)bindingModel.size(); i++)
	{
		BindingModel& binding = bindingModel[i];

		ui::Label * functionLabel = new ui::Label(ui::Point(4, currentY), ui::Point(Size.X / 2, 16), binding.description);
		functionLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		scrollPanel->AddChild(functionLabel);

		KeyconfigTextbox* textbox;
		ui::Label* noShortCutLabel;

		bool hasBinding = true;
		int removeButtonPosX = 0;

		if (!c->FunctionHasShortcut(binding.functionId))
		{
			hasBinding = false;
			noShortCutLabel = new ui::Label(ui::Point(functionLabel->Position.X + functionLabel->Size.X + 20, currentY), ui::Point(95, 16), "(No shortcut)");
			noShortCutLabel->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
			scrollPanel->AddChild(noShortCutLabel);
		}
		else
		{
			textbox = new KeyconfigTextbox(ui::Point(functionLabel->Position.X + functionLabel->Size.X + 20, currentY), ui::Point(95, 16));
			textbox->SetTextFromModifierAndScan(binding.modifier, binding.scan);
			textbox->SetModel(binding);
			textbox->AttachController(c);
			textboxes.push_back(textbox);
			scrollPanel->AddChild(textbox);
			removeButtonPosX = textbox->Position.X + textbox->Size.X + 5;
		}
		
		int addButtonPosX = functionLabel->Position.X + functionLabel->Size.X - 5;
		int addRemoveButtonsPosY = currentY;
		currentY += 20;

		// add in all the bindings associated with the current functionId
		if (hasBinding)
		{
			auto it = bindingModel.begin() + i + 1;
			while (it != bindingModel.end())
			{
				BindingModel nextBinding = *it;
				if (nextBinding.functionId == binding.functionId)
				{
					KeyconfigTextbox* tb = new KeyconfigTextbox(ui::Point(functionLabel->Position.X + functionLabel->Size.X + 20, currentY), ui::Point(95, 16));
					if (!nextBinding.isNew)
						tb->SetTextFromModifierAndScan(nextBinding.modifier, nextBinding.scan);
					else
						tb->SetText("");
					
					tb->SetModel(nextBinding);
					tb->AttachController(c);
					textboxes.push_back(tb);
					scrollPanel->AddChild(tb);
					currentY += 20;
					i++;
				}
				else
				{
					// the vector is sorted so once we hit unequality
					// in function id then it means we are onto the next function
					break;
				}
				
				it++;
			}
		}

		ui::Button* addButton = new ui::Button(ui::Point(addButtonPosX, addRemoveButtonsPosY), ui::Point(20, 16), "+", "Add a binding to this action");

		scrollPanel->AddChild(addButton);

		class AddBindingAction: public ui::ButtonAction
		{
		public:
			KeyconfigView * v;
			int functionId;
			String desc;

			AddBindingAction(KeyconfigView * v_, int _functionId, String _desc) 
			{ 
				v = v_; 
				functionId = _functionId;
				desc = _desc;
			}

			void ActionCallback(ui::Button * sender) override
			{
				auto modelArr = v->c->GetBindingPrefs();
				auto it = std::find_if(modelArr.begin(), modelArr.end(), [this](BindingModel b)
				{
					return b.functionId == functionId && b.isNew;
				});

				// do not add more KBT's if we have an empty one on the 
				// current function
				if (it != modelArr.end())
					return;

				BindingModel model;
				model.isNew = true;
				model.functionId = functionId;
				model.description = desc; // for sorting
				v->c->CreateModel(model);
				v->BuildKeyBindingsListView();
				v->c->NotifyBindingsChanged();
			}
		};

		class RemoveBindingAction: public ui::ButtonAction
		{
		public:
			KeyconfigView * v;
			int functionId;

			RemoveBindingAction(KeyconfigView * v_, int _functionId) 
			{ 
				v = v_; 
				functionId = _functionId;
			}

			void ActionCallback(ui::Button * sender) override
			{
				v->c->PopBindingByFunctionId(functionId);
				v->BuildKeyBindingsListView();
				v->c->NotifyBindingsChanged();
			}
		};

		addButton->SetActionCallback(new AddBindingAction(this, binding.functionId, binding.description));

		// only add in a remove button if we have a binding attached to the current function
		if (hasBinding)
		{
			ui::Button* removeButton = new ui::Button(ui::Point(removeButtonPosX, addRemoveButtonsPosY), ui::Point(20, 16), "-", "Remove a binding from this action");
			scrollPanel->AddChild(removeButton);
			removeButton->SetActionCallback(new RemoveBindingAction(this, binding.functionId));
		}
	}

	scrollPanel->InnerSize = ui::Point(Size.X, currentY);
	scrollPanel->SetScrollPosition(scrollPos);
}

void KeyconfigView::OnKeyReleased()
{
	for (auto textbox : textboxes)
	{
		if (textbox->GetText().length() == 0)
		{
			textbox->SetTextToPrevious();
		}
	}
}

void KeyconfigView::AttachController(KeyconfigController* c_)
{
	c = c_;
}

void KeyconfigView::OnDraw()
{
	Graphics * g = GetGraphics();
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
}

void KeyconfigView::OnTryExit(ExitMethod method)
{
	c->Exit();
}

void KeyconfigView::OnKeyCombinationChanged(bool hasConflict)
{
	// disable OK button if there's a conflict
	if (hasConflict)
	{
		okayButton->Enabled = false;
		conflictLabel->Visible = true;
	}
	else
	{
		okayButton->Enabled = true;
		conflictLabel->Visible = false;
	}
}

KeyconfigView::~KeyconfigView()
{

}
