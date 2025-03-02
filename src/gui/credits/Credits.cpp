#include "Credits.h"

#include <json/json.h>

#include "credits_json.h"
#include "gui/Style.h"

#include "common/platform/Platform.h"
#include "gui/interface/AvatarButton.h"
#include "gui/interface/Button.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Label.h"
#include "gui/interface/RichLabel.h"
#include "gui/interface/ScrollPanel.h"
#include "gui/interface/Separator.h"

Credits::Credits():
	ui::Window(ui::Point(-1, -1), ui::Point(WINDOWW, WINDOWH))
{
	Json::Value root;
	Json::Reader reader;
	auto credits = credits_json.AsCharSpan();
	if (bool parsed = reader.parse(credits.data(), credits.data() + credits.size(), root, false); !parsed) {
		// Failure. Shouldn't ever happen.
		return;
	}

	auto *scrollPanel = new ui::ScrollPanel(ui::Point(0, 0), ui::Point(Size.X, Size.Y - 12));
	AddComponent(scrollPanel);

	int xPos = 0, yPos = 0, row = 0;
	int nextY = 0;

	// Organize blocks of components of equal width into rows, and add them to the scroll panel
	auto organizeComponents = [&xPos, &yPos, &nextY, &row, &scrollPanel](const auto& components, const int panelWidth) {
		ui::Point blockSize = { 0, 0 };
		for (const auto &component : components)
		{
			blockSize.X = std::max(blockSize.X, component->Position.X + component->Size.X);
			blockSize.Y = std::max(blockSize.Y, component->Position.Y + component->Size.Y);
		}

		// New row, offset x position to ensure entire row is centered
		if (xPos == 0)
			xPos = (panelWidth % blockSize.X) / 2;
		for (const auto &component : components)
			component->Position += ui::Point({ xPos, yPos });

		xPos += blockSize.X;
		nextY =  std::max(nextY, yPos + blockSize.Y);
		if (xPos + blockSize.X > panelWidth)
		{
			xPos = 0;
			yPos = nextY + 8;
			row++;
		}

		for (const auto &component : components)
			scrollPanel->AddChild(component);
	};

	// Add header and separator for each section of credits
	auto addHeader = [&xPos, &yPos, &nextY, &row, &scrollPanel](const String &text, const bool addSeparator = true) {
		xPos = 0;
		yPos = nextY + 10;
		row = 0;

		if (addSeparator)
		{
			auto *separator = new ui::Separator(ui::Point(0, yPos), ui::Point(scrollPanel->Size.X, 1));
			scrollPanel->AddChild(separator);
			yPos += 6;
		}

		auto *label = new ui::RichLabel(ui::Point(4, yPos), ui::Point(scrollPanel->Size.X, 24), text);
		label->SetTextColour(style::Colour::InformationTitle);
		label->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
		label->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
		scrollPanel->AddChild(label);
		yPos += label->Size.Y + 8;
	};


	addHeader("TPT is an open source project, developed by members of the community.\n"
			"We'd like to thank everyone who contributed to our \bt{a:https://github.com/The-Powder-Toy/The-Powder-Toy|GitHub repo}\x0E:", false);

	auto GitHub = root["GitHub"];
	int grayscale = 255;
	for (auto &item : GitHub)
	{
		ByteString username = item.asString();
		auto components = AddCredit(username.FromUtf8(), "", Small, "", false, grayscale);
		organizeComponents(components, scrollPanel->Size.X);
		if (grayscale > 180)
			grayscale--;
	}


	addHeader("Staff - volunteers that run the community and keep the site running");

	auto Moderators = root["Moderators"];
	for (auto &item : Moderators)
	{
		ByteString username = item["username"].asString();
		ByteString role = item["role"].asString();

		if (role == "Moderator" || role == "HalfMod")
		{
			auto components = AddCredit(username.FromUtf8(), "", Large, GetProfileUri(username), true);
			organizeComponents(components, scrollPanel->Size.X);
		}
	}


	addHeader("Former Staff", false);

	for (auto &item : Moderators)
	{
		ByteString username = item["username"].asString();
		ByteString role = item["role"].asString();

		if (role == "Former Staff")
		{
			auto components = AddCredit(username.FromUtf8(), "", Small, "", true);
			organizeComponents(components, scrollPanel->Size.X);
		}
	}


	addHeader("The following users have been credited in the intro text from the start.\n"
			"Their contributions to the early beginnings of TPT were invaluable in shaping TPT into what it is today.");

	auto OrigCredits = root["OrigCredits"];
	for (auto &item : OrigCredits)
	{
		ByteString realname = item["realname"].asString();
		ByteString message = item["message"].asString();

		auto components = AddCredit(realname.FromUtf8(), message.FromUtf8(), row == 0 ? Half : Small, "");
		organizeComponents(components, scrollPanel->Size.X);
	}


	scrollPanel->InnerSize = ui::Point(scrollPanel->Size.X, nextY);

	auto *closeButton = new ui::Button({ 0, Size.Y - 12 }, { Size.X, 12 }, "Close");
	closeButton->SetActionCallback({
	[this] {
		CloseActiveWindow();
	}  });
	AddComponent(closeButton);
}

std::vector<ui::Component *> Credits::AddCredit(const String &name, const String &subheader, const CreditSize size,
	const ByteString &uri, const bool includeAvatar, const int grayscale)
{
	std::vector<ui::Component *> components;
	int creditBlockWidth = size == Small ? 100 : (size == Large ? 155 : 310);
	int y = 0;

	if (includeAvatar)
	{
		int avatarWidth = size == Small ? 40 : 64;
		int avatarSize = size == Small ? 40 : 256;
		auto *avatarButton = new ui::AvatarButton(ui::Point((creditBlockWidth - avatarWidth) / 2, 0), ui::Point(avatarWidth, avatarWidth), name.ToUtf8(), avatarSize);
		if (!uri.empty())
		{
			avatarButton->SetActionCallback({[uri] {
				Platform::OpenURI(uri);
			} });
		}
		components.push_back(avatarButton);

		y += avatarButton->Size.Y + 2;
	}

	if (!name.empty())
	{
		auto labelText = !uri.empty() ? GetRichLabelText(uri, name) : name;
		auto *nameLabel = new ui::RichLabel(ui::Point(0, y), ui::Point(creditBlockWidth, 14), labelText);
		nameLabel->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
		nameLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
		nameLabel->SetTextColour(ui::Colour(grayscale, grayscale, grayscale));
		components.push_back(nameLabel);

		y += nameLabel->Size.Y + 2;
	}

	if (!subheader.empty())
	{
		auto *subheaderLabel = new ui::Label(ui::Point(0, y), ui::Point(creditBlockWidth, 14), subheader);
		subheaderLabel->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
		subheaderLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
		int col = (int)((float)grayscale * .67f);
		subheaderLabel->SetTextColour(ui::Colour(col, col, col));
		components.push_back(subheaderLabel);
	}

	return components;
}

ByteString Credits::GetProfileUri(const ByteString &username)
{
	return "https://powdertoy.co.uk/User.html?Name=" + username;
}

String Credits::GetRichLabelText(const ByteString &uri, const String &message)
{
	StringBuilder builder;
	builder << "{a:" << uri.FromUtf8() << "|" << message << "}";
	return builder.Build();
}

void Credits::OnTryExit(ExitMethod method)
{
	ui::Engine::Ref().CloseWindow();
}
