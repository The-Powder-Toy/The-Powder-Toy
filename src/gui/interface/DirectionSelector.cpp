#include "DirectionSelector.h"

namespace ui {

DirectionSelector::DirectionSelector(ui::Point position, float radius):
	ui::Component(position, ui::Point(radius * 2, radius * 2)),
	radius(radius),
	maxRadius(radius - (radius / 4)),
	useSnapPoints(false),
	autoReturn(false),
	backgroundColor(ui::Colour(0, 0, 0, 63)),
	foregroundColor(ui::Colour(31, 31, 31, 127)),
	borderColor(ui::Colour(255, 255, 255)),
	snapPointColor(ui::Colour(63, 63, 63, 127)),
	updateCallback(nullptr),
	changeCallback(nullptr),
	mouseDown(false),
	offset(ui::Point(0, 0))
	{
		
	}

void DirectionSelector::SetSnapPoints(int newRadius, int points)
{
	snapPointRadius = newRadius;
	snapPoints.clear();
	snapPoints.push_back(ui::Point(0, 0));
	for (int i = 1; i < points; i++)
	{
		snapPoints.push_back(ui::Point(0, (radius / points) * i));
		snapPoints.push_back(ui::Point(0, -1 * (radius / points) * i));
		snapPoints.push_back(ui::Point(-1 * (radius / points) * i, 0));
		snapPoints.push_back(ui::Point((radius / points) * i, 0));
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
	return offset.X / maxRadius;
}

float DirectionSelector::GetYValue()
{
	return offset.Y / maxRadius;
}

float DirectionSelector::GetTotalValue()
{
	return std::hypot(offset.X, offset.Y) / maxRadius;
}

void DirectionSelector::SetPositionAbs(float absx, float absy)
{
	SetPosition(absx - radius, absy - radius);
}

void DirectionSelector::SetPosition(float x, float y)
{
	if (std::hypot(x, y) > maxRadius)
	{
		offset.X = (maxRadius * x) / std::hypot(x, y);
		offset.Y = (maxRadius * y) / std::hypot(x, y);
	}
	else
	{
		offset.X = x;
		offset.Y = y;
	}

	if (useSnapPoints)
	{
		std::vector<ui::Point>::iterator i;
		for (i = snapPoints.begin(); i < snapPoints.end(); i++)
			if (std::hypot(i->X - offset.X, i->Y - offset.Y) <= snapPointRadius)
			{
				offset.X = i->X;
				offset.Y = i->Y;
			}
	}
	if (updateCallback)
		updateCallback(GetXValue(), GetYValue());
}

void DirectionSelector::SetValues(float x, float y)
{
	SetPosition(x * maxRadius, y * maxRadius);
}

void DirectionSelector::Draw(const ui::Point& screenPos)
{
	Graphics * g = GetGraphics();
	ui::Point center = screenPos + radius;

	g->fillcircle(center.X, center.Y, radius, radius, backgroundColor.Red, backgroundColor.Green, backgroundColor.Blue, backgroundColor.Alpha);
	g->drawcircle(center.X, center.Y, radius, radius, borderColor.Red, borderColor.Green, borderColor.Blue, borderColor.Alpha);

	if (useSnapPoints)
	{
		std::vector<ui::Point>::iterator i;
		for (i = snapPoints.begin(); i < snapPoints.end(); i++)
			g->fillrect(
				(center.X + i->X) - (radius / 30),
				(center.Y + i->Y) - (radius / 30),
				radius / 10, radius / 10,
				snapPointColor.Red, snapPointColor.Green, snapPointColor.Blue, snapPointColor.Green
			);
	}

	g->fillcircle(center.X + offset.X, center.Y + offset.Y, radius / 4, radius / 4, foregroundColor.Red, foregroundColor.Green, foregroundColor.Blue, foregroundColor.Alpha);
	g->drawcircle(center.X + offset.X, center.Y + offset.Y, radius / 4, radius / 4, borderColor.Red, borderColor.Green, borderColor.Blue, borderColor.Alpha);
}

void DirectionSelector::OnMouseMoved(int x, int y, int dx, int dy)
{
	if (mouseDown)
		SetPositionAbs(x, y);
}

void DirectionSelector::OnMouseClick(int x, int y, unsigned button)
{
	mouseDown = true;
	SetPositionAbs(x, y);
}

void DirectionSelector::OnMouseUp(int x, int y, unsigned button)
{
	mouseDown = false;
	if (autoReturn)
		SetPosition(0, 0);

	if (changeCallback)
		changeCallback(GetXValue(), GetYValue());
}

}
