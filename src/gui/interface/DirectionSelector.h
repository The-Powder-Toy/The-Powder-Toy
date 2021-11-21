#ifndef DIRECTIONSELECTOR_H_
#define DIRECTIONSELECTOR_H_

#include "Component.h"
#include "Colour.h"
#include "graphics/Graphics.h"

#include <iostream>

#include <cmath>
#include <vector>
#include <functional>

namespace ui {

class DirectionSelector : public ui::Component
{
	const float radius;
	const float maxRadius;

	bool useSnapPoints;
	int snapPointRadius;

	std::vector<ui::Point> snapPoints;

	bool autoReturn;
	
	ui::Colour backgroundColor;
	ui::Colour foregroundColor;
	ui::Colour borderColor;
	ui::Colour snapPointColor;

	std::function<void(float x, float y)> updateCallback;
	std::function<void(float x, float y)> changeCallback;

	bool mouseDown;

	ui::Point offset;

public:
	DirectionSelector(ui::Point position, float radius);
	virtual ~DirectionSelector() = default;

	void SetSnapPoints(int newRadius, int points);
	void ClearSnapPoints();

	inline void EnableAutoReturn() { autoReturn = true; }
	inline void DisableAutoReturn() { autoReturn = false; }
	
	inline void SetBackgroundColor(ui::Colour color) { backgroundColor = color; }
	inline void SetForegroundColor(ui::Colour color) { foregroundColor = color; }
	inline void SetBorderColor(ui::Colour color) { borderColor = color; }
	inline void SetSnapPointColor(ui::Colour color) { snapPointColor = color; }

	float GetXValue();
	float GetYValue();
	float GetTotalValue();

	void SetPositionAbs(float absx, float absy);
	void SetPosition(float x, float y);
	void SetValues(float x, float y);

	void Draw(const ui::Point& screenPos) override;
	void OnMouseMoved(int x, int y, int dx, int dy) override;
	void OnMouseClick(int x, int y, unsigned int button) override;
	void OnMouseUp(int x, int y, unsigned button) override;

	inline void SetUpdateCallback(std::function<void(float x, float y)> callback) { updateCallback = callback; }
	inline void SetChangeCallback(std::function<void(float x, float y)> callback) { changeCallback = callback; }
};

} /* namespace ui */
#endif /* DIRECTIONSELECTOR_H_ */
