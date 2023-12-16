#pragma once
#include "Label.h"
#include <vector>
#include <variant>

namespace ui
{
	class RichLabel : public Label
	{
		struct RichTextRegion
		{
			int begin;
			int end;
			struct LinkAction
			{
				ByteString uri;
			};
			using Action = std::variant<LinkAction>;
			Action action;
		};
		std::vector<RichTextRegion> regions;

	public:
		RichLabel(Point position, Point size, String text);

		void SetText(String newText) override;
		void OnMouseClick(int x, int y, unsigned button) override;
	};
}
