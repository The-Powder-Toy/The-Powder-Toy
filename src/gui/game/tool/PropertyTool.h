#pragma once
#include "simulation/AccessProperty.h"
#include "Tool.h"
#include <optional>

class GameModel;

class PropertyTool: public Tool
{
public:
	struct Configuration
	{
		AccessProperty changeProperty;
		String propertyValueStr;
	};

private:
	void SetProperty(Simulation *sim, ui::Point position);
	void SetConfiguration(std::optional<Configuration> newConfiguration);

	std::optional<Configuration> configuration;

	friend class PropertyWindow;

public:
	PropertyTool():
		Tool(0, "PROP", "Property Drawing Tool. Use to alter the properties of elements in the field.",
			0xFEA900_rgb, "DEFAULT_UI_PROPERTY", NULL
		)
	{}

	void OpenWindow(Simulation *sim, std::optional<int> takePropertyFrom);
	void Click(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void Draw(Simulation *sim, Brush const &brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging) override;
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override;

	std::optional<Configuration> GetConfiguration() const
	{
		return configuration;
	}
};
