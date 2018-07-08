#include "GameModel.h"
#include "Tool.h"

int ConfigTool::getIdAt(Simulation *sim, ui::Point position)
{
	if(position.X<0 || position.X>XRES || position.Y<0 || position.Y>YRES)
		return -1;
	int p = sim->pmap[position.Y][position.X];
	if(!p)
		return -1;
	return ID(p);
}

Particle ConfigTool::getPartAt(Simulation *sim, ui::Point position)
{
	int id = getIdAt(sim, position);
	Particle nullPart;
	nullPart.type = PT_NONE;
	return (id == -1) ? nullPart : sim->parts[id];
}

bool ConfigTool::isSamePart(Particle p1, Particle p2)
{
	return p1.type == p2.type &&
		int(p1.x+0.5f) == int(p2.x+0.5f) &&
		int(p1.y+0.5f) == int(p2.y+0.5f);
}

ui::Point ConfigTool::projectPoint(Particle part, int sampleX, int sampleY)
{
	int partX = int(part.x + 0.5f), partY = int(part.y + 0.5f);
	int relX = sampleX - partX, relY = sampleY - partY;
	int absX = (relX > 0) ? relX : -relX, absY = (relY > 0) ? relY : -relY;
	int projX = 0, projY = 0, diagProjX, diagProjY;
	if(absX > absY)
		projX = relX;
	else
		projY = relY;
	if((relX > 0) == (relY > 0))
	{
		diagProjX = (relX + relY) / 2;
		diagProjY = diagProjX;
	}
	else
	{
		diagProjX = (relX - relY) / 2;
		diagProjY = -diagProjX;
	}
	if(projX*projX+projY*projY > diagProjX*diagProjX+diagProjY*diagProjY)
		return ui::Point(projX, projY);
	else
		return ui::Point(diagProjX, diagProjY);
}

int ConfigTool::getDist(Particle part, int sampleX, int sampleY)
{
	ui::Point proj = projectPoint(part, sampleX, sampleY);
	int signedDist = proj.X ? proj.X : proj.Y;
	return (signedDist > 0) ? signedDist : -signedDist;
}

void ConfigTool::Click(Simulation *sim, Brush *brush, ui::Point position)
{
	if(configState != ConfigState::ready &&
		!isSamePart(sim->parts[currId], configPart))
	{
		Reset();
	}
	switch(configState)
	{
	case ConfigState::ready:
		currId = getIdAt(sim, position);
		if(currId == -1)
			break;
		configPart = sim->parts[currId];
		switch(configPart.type)
		{
		case PT_DRAY:
			configState = ConfigState::drayTmp;
			break;
		}
		break;
	case ConfigState::drayTmp:
		sim->parts[currId].tmp = getDist(configPart, position.X, position.Y);
		configPart = sim->parts[currId];
		configState = ConfigState::drayTmp2;
		break;
	case ConfigState::drayTmp2:
		sim->parts[currId].tmp2 = getDist(configPart, position.X, position.Y) - configPart.tmp;
		configState = ConfigState::ready;
		break;
	}
}

void ConfigTool::Reset()
{
	configState = ConfigState::ready;
}

String ConfigTool::GetInfo(GameController *c, SimulationSample sample)
{
	StringBuilder infoStream;
	infoStream << "Configurator: ";
	switch(configState)
	{
	case ConfigState::ready:
		if(sample.particle.type == PT_DRAY)
			infoStream << c->ElementResolve(sample.particle.type, -1).FromAscii();
		else
			infoStream << "Ready";
		break;
	case ConfigState::drayTmp:
		infoStream << "DRAY, tmp: " << getDist(configPart, sample.PositionX, sample.PositionY);
		break;
	case ConfigState::drayTmp2:
		infoStream << "DRAY, tmp: " << configPart.tmp << ", tmp2: " << getDist(configPart, sample.PositionX, sample.PositionY) - configPart.tmp;
		break;
	}
	return infoStream.Build();
}

void ConfigTool::lineToProj(Renderer *ren, SimulationSample sample)
{
	ui::Point proj = projectPoint(configPart, sample.PositionX, sample.PositionY);
	int dirx = (proj.X == 0) ? 0 : ((proj.X > 0) ? 1 : -1);
	int diry = (proj.Y == 0) ? 0 : ((proj.Y > 0) ? 1 : -1);
	ren->draw_line(configPart.x + dirx, configPart.y + diry, configPart.x + proj.X, configPart.y + proj.Y, 255, 0, 0, 255);
}

void ConfigTool::tripleLine(Renderer *ren, SimulationSample sample, int offset)
{
	ui::Point proj = projectPoint(configPart, sample.PositionX, sample.PositionY);
	int dirx = (proj.X == 0) ? 0 : ((proj.X > 0) ? 1 : -1);
	int diry = (proj.Y == 0) ? 0 : ((proj.Y > 0) ? 1 : -1);
	ren->draw_line(configPart.x + dirx, configPart.y + diry, configPart.x + dirx * offset, configPart.y + diry * offset, 255, 0, 0, 255);
	ren->draw_line(configPart.x + dirx * (offset + 1), configPart.y + diry * (offset + 1), configPart.x + proj.X, configPart.y + proj.Y, 200, 200, 200, 255);
	ren->draw_line(configPart.x + proj.X + dirx, configPart.y + proj.Y + diry, configPart.x + proj.X + dirx * offset, configPart.y + proj.Y + diry * offset, 255, 0, 0, 255);
}

void ConfigTool::DrawHUD(Renderer *ren, SimulationSample sample)
{
	switch(configState)
	{
	case ConfigState::ready:
		ren->xor_line(sample.PositionX, sample.PositionY, sample.PositionX, sample.PositionY);
		break;
	case ConfigState::drayTmp:
		lineToProj(ren, sample);
		break;
	case ConfigState::drayTmp2:
		tripleLine(ren, sample, configPart.tmp);
		break;
	}
}
