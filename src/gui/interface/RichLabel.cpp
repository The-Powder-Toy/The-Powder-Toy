#include "RichLabel.h"
#include "common/platform/Platform.h"
#include "Format.h"

using namespace ui;

RichLabel::RichLabel(Point position, Point size, String text) : Label(position, size, "")
{
	SetText(text);
}

void RichLabel::SetText(String newText)
{
	Label::SetText(newText);
	std::vector<RichTextRegion> newRegions;
	StringBuilder sb;
	auto it = newText.begin();
	while (it != newText.end())
	{
		auto find = [&newText](auto it, String::value_type ch) {
			while (it != newText.end())
			{
				if (*it == ch)
				{
					break;
				}
				++it;
			}
			return it;
		};
		auto beginRegionIt = find(it, '{');
		auto beginDataIt = find(beginRegionIt, ':');
		auto beginTextIt = find(beginDataIt, '|');
		auto endRegionIt = find(beginTextIt, '}');
		if (endRegionIt == newText.end())
		{
			break;
		}
		auto action = String(beginRegionIt + 1, beginDataIt);
		auto data = String(beginDataIt + 1, beginTextIt);
		auto text = String(beginTextIt + 1, endRegionIt);
		sb << String(it, beginRegionIt);
		auto good = false;
		if (action == "a" && data.size() && text.size())
		{
			RichTextRegion region;
			region.begin = sb.Size();
			sb << text;
			region.end = sb.Size();
			region.action = RichTextRegion::LinkAction{ data.ToUtf8() };
			newRegions.push_back(region);
			good = true;
		}
		if (!good)
		{
			sb << String(beginRegionIt, endRegionIt + 1);
		}
		it = endRegionIt + 1;
	}
	sb << String(it, newText.end());
	auto newDisplayText = sb.Build();
	Label::SetText(format::CleanString(newDisplayText, false, true, false));
	Label::SetDisplayText(newDisplayText);
	regions = newRegions;
}

void RichLabel::OnMouseClick(int x, int y, unsigned button)
{
	int cursorPosition = displayTextWrapper.Point2Index(x - textPosition.X, y - textPosition.Y).raw_index;
	for (auto const &region : regions)
	{
		if (region.begin <= cursorPosition && region.end > cursorPosition)
		{
			if (auto *linkAction = std::get_if<RichTextRegion::LinkAction>(&region.action))
			{
				Platform::OpenURI(linkAction->uri);
				return;
			}
		}
	}
	Label::OnMouseClick(x, y, button);
}
