#pragma once

#include <string>

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
			std::string actionData;
		};

		RichLabel(Point position, Point size, std::string richText);

		~RichLabel() override;

		virtual void SetText(std::string text);
		virtual std::string GetDisplayText();
		virtual std::string GetText();

		void Draw(const Point& screenPos) override;
		void OnMouseClick(int x, int y, unsigned button) override;
	protected:
		std::string textSource;
		std::string displayText;

		std::vector<RichTextRegion> regions;

		void updateRichText();
	};
}
