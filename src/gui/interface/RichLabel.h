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

		void SetText(String text);
		String GetDisplayText();
		String GetText();

		void Draw(const Point& screenPos) override;
		void OnMouseClick(int x, int y, unsigned button) override;
	protected:
		String textSource;
		String displayText;

		std::vector<RichTextRegion> regions;

		void updateRichText();
	};
}
