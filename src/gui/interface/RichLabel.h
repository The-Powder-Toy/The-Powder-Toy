#pragma once

#include "common/String.h"

#include "Component.h"
#include "Colour.h"

namespace ui
{
	class RichLabel : public Component
	{
	public:
		struct RichTextRegion
		{
			int start;
			int finish;
			int action;
			String actionData;
		};

		RichLabel(Point position, Point size, String richText);

		virtual ~RichLabel();

		virtual void SetText(String text);
		virtual String GetDisplayText();
		virtual String GetText();

		virtual void Draw(const Point& screenPos);
		virtual void OnMouseClick(int x, int y, unsigned button);
	protected:
		String textSource;
		String displayText;

		std::vector<RichTextRegion> regions;

		void updateRichText();
	};
}
