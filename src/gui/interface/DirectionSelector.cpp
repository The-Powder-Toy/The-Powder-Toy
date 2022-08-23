#include "DirectionSelector.h"

namespace ui {

DirectionSelector::DirectionSelector(ui::Point position, float scale, int radius, int handleRadius, int snapPointRadius, int snapPointEffectRadius):
	ui::Component(position, ui::Point(radius * 5 / 2, radius * 5 / 2)),
	scale(scale),
	radius(radius),
	handleRadius(handleRadius),
	useSnapPoints(false),
	snapPointRadius(snapPointRadius),
	snapPointEffectRadius(snapPointEffectRadius),
	autoReturn(false),
	backgroundColor(ui::Colour(0, 0, 0, 63)),
	foregroundColor(ui::Colour(63, 63, 63, 127)),
	borderColor(ui::Colour(255, 255, 255)),
	snapPointColor(ui::Colour(63, 63, 63, 127)),
	updateCallback(nullptr),
	changeCallback(nullptr),
	mouseDown(false),
	mouseHover(false),
	altDown(false),
	value({ { 0, 0 }, 0, 0 })
{
}

void DirectionSelector::CheckHovering(int x, int y)
{
	mouseHover = std::hypot((value.offset.X + radius) - x, (value.offset.Y + radius) - y) < handleRadius;
}

DirectionSelector::Value DirectionSelector::GravityValueToValue(float x, float y)
{
	return { { int(x / scale), int(y / scale) }, x, y };
}

DirectionSelector::Value DirectionSelector::PositionToValue(ui::Point position)
{
	auto length = std::hypot(float(position.X), float(position.Y));
	if (length > radius)
	{
		position.X = int(position.X / length * radius);
		position.Y = int(position.Y / length * radius);
	}
	return { position, position.X * scale, position.Y * scale };
}

void DirectionSelector::SetSnapPoints(int newSnapPointEffectRadius, int points, float maxMagnitude)
{
	snapPointEffectRadius = newSnapPointEffectRadius;
	snapPoints.clear();
	snapPoints.push_back(GravityValueToValue(0, 0));
	for (int i = 1; i < points; i++)
	{
		auto dist = i / float(points - 1) * maxMagnitude;
		snapPoints.push_back(GravityValueToValue( dist,     0));
		snapPoints.push_back(GravityValueToValue(    0,  dist));
		snapPoints.push_back(GravityValueToValue(-dist,     0));
		snapPoints.push_back(GravityValueToValue(    0, -dist));
	}
	useSnapPoints = true;
}

void DirectionSelector::ClearSnapPoints()
{
	useSnapPoints = false;
	snapPoints.clear();
}

float DirectionSelector::GetXValue()
{
	return value.xValue;
}

float DirectionSelector::GetYValue()
{
	return value.yValue;
}

void DirectionSelector::SetPositionAbs(ui::Point position)
{
	SetPosition(position - ui::Point{ radius + handleRadius, radius + handleRadius });
}

void DirectionSelector::SetPosition(ui::Point position)
{
	value = PositionToValue(position);

	if (useSnapPoints && !altDown)
	{
		for (auto &point : snapPoints)
		{
			if (std::hypot(point.offset.X - position.X, point.offset.Y - position.Y) <= snapPointEffectRadius)
			{
				value = point;
			}
		}
	}
	if (updateCallback)
	{
		updateCallback(GetXValue(), GetYValue());
	}
}

void DirectionSelector::SetValues(float x, float y)
{
	value.xValue = x;
	value.yValue = y;
	SetPosition(GravityValueToValue(x, y).offset);
}

void DirectionSelector::Draw(const ui::Point& screenPos)
{
	Graphics * g = GetGraphics();
	auto handleTrackRadius = radius + handleRadius;
	ui::Point center = screenPos + handleTrackRadius;

	g->fillcircle(center.X, center.Y, handleTrackRadius, handleTrackRadius, backgroundColor.Red, backgroundColor.Green, backgroundColor.Blue, backgroundColor.Alpha);
	g->drawcircle(center.X, center.Y, handleTrackRadius, handleTrackRadius, borderColor.Red, borderColor.Green, borderColor.Blue, borderColor.Alpha);

	for (auto &point : snapPoints)
	{
		g->fillrect(
			(center.X + point.offset.X) - snapPointRadius,
			(center.Y + point.offset.Y) - snapPointRadius,
			snapPointRadius * 2 + 1, snapPointRadius * 2 + 1,
			snapPointColor.Red, snapPointColor.Green, snapPointColor.Blue, altDown ? (int)(snapPointColor.Alpha / 2) : snapPointColor.Alpha
		);
	}

	g->fillcircle(center.X + value.offset.X, center.Y + value.offset.Y, handleRadius, handleRadius, foregroundColor.Red, foregroundColor.Green, foregroundColor.Blue, (mouseHover || mouseDown) ? std::min((int)(foregroundColor.Alpha * 1.5f), 255) : foregroundColor.Alpha);
	g->drawcircle(center.X + value.offset.X, center.Y + value.offset.Y, handleRadius, handleRadius, borderColor.Red, borderColor.Green, borderColor.Blue, borderColor.Alpha);
}

void DirectionSelector::OnMouseMoved(int x, int y, int dx, int dy)
{
	if (mouseDown)
	{
		SetPositionAbs({ x, y });
	}
	CheckHovering(x, y);
}

void DirectionSelector::OnMouseClick(int x, int y, unsigned button)
{
	mouseDown = true;
	SetPositionAbs({ x, y });
	CheckHovering(x, y);
}

void DirectionSelector::OnMouseUp(int x, int y, unsigned button)
{
	mouseDown = false;
	if (autoReturn)
	{
		SetPosition({ 0, 0 });
	}
	CheckHovering(x - Position.X, y - Position.Y);

	if (changeCallback)
	{
		changeCallback(GetXValue(), GetYValue());
	}
}

}
