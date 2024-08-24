#pragma once
#include "Tool.h"

class GameModel;

class PropertyTool: public Tool
{
public:
	struct Configuration
	{
		StructProperty prop;
		PropertyValue propValue;
		bool changeType;
		int propertyIndex;
		String propertyValueStr;
	};

private:
	void SetProperty(Simulation *sim, ui::Point position);
	void SetConfiguration(std::optional<Configuration> newConfiguration);

	GameModel &gameModel;
	std::optional<Configuration> configuration;

	friend class PropertyWindow;

public:
	PropertyTool(GameModel &model):
		Tool(0, "PROP", "Property Drawing Tool. Use to alter the properties of elements in the field.",
			0xFEA900_rgb, "DEFAULT_UI_PROPERTY", NULL
		),
		gameModel(model)
	{}

	void OpenWindow(Simulation *sim, const Particle *takePropertyFrom);
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
