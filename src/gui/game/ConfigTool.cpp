#include "GameModel.h"
#include "Tool.h"

class FiltConfigWindow: public ui::Window
{
public:
	ConfigTool *tool;
	Simulation *sim;
	FiltConfigWindow(ConfigTool *tool_, Simulation *sim);
	void OnSelect(int result);
	virtual void OnTryExit(ExitMethod method);
	virtual ~FiltConfigWindow() {}
	class SelectAction: public ui::ButtonAction
	{
	public:
		FiltConfigWindow *prompt;
		int result;
		SelectAction(FiltConfigWindow *prompt_, int result_):
		prompt(prompt_),
		result(result_)
		{ }
		void ActionCallback(ui::Button *sender)
		{
			prompt->OnSelect(result);
			return;
		}
	};
};

FiltConfigWindow::FiltConfigWindow(ConfigTool * tool_, Simulation *sim_):
ui::Window(ui::Point(-1, -1), ui::Point(150, 200)),
tool(tool_),
sim(sim_)
{
	int maxTextWidth = 0;
	for(int i = 0; i <= Element_FILT::NUM_MODES; i++)
	{
		String buttonText = (i == Element_FILT::NUM_MODES) ?
			String::Build("Cancel") : Element_FILT::MODES[i];
		int textWidth = Graphics::textwidth(buttonText);
		if(textWidth > maxTextWidth)
			maxTextWidth = textWidth;
	}
	int buttonWidth = maxTextWidth + 15;
	int buttonHeight = 17;
	int buttonLeft = Size.X/2 - buttonWidth/2;
	int buttonTop = Size.Y/2 - ((buttonHeight-1) * (Element_FILT::NUM_MODES+1))/2;
	for(int i = 0; i <= Element_FILT::NUM_MODES; i++)
	{
		String buttonText = (i == Element_FILT::NUM_MODES) ?
			String::Build("Cancel") : Element_FILT::MODES[i];
		ui::Button * b = new ui::Button(ui::Point(buttonLeft, i * (buttonHeight-1) + buttonTop), ui::Point(buttonWidth, buttonHeight), buttonText);
		b->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
		b->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
		b->SetActionCallback(new SelectAction(this, i));
		AddComponent(b);
	}
	MakeActiveWindow();
}

void FiltConfigWindow::OnSelect(int result)
{
	if(result != Element_FILT::NUM_MODES)
		tool->OnSelectFiltTmp(sim, result);
	CloseActiveWindow();
	SelfDestruct();
}

void FiltConfigWindow::OnTryExit(ExitMethod method)
{
	CloseActiveWindow();
	SelfDestruct();
}

int ConfigTool::getIdAt(Simulation *sim, ui::Point position)
{
	if(position.X<0 || position.X>=XRES || position.Y<0 || position.Y>=YRES)
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

ui::Point ConfigTool::projectPoint(Particle part, int sampleX, int sampleY, bool allowDiag)
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
	int relProjX = relX - projX, relProjY = relY - projY;
	int relDiagProjX = relX - diagProjX, relDiagProjY = relY - diagProjY;
	int distProj = relProjX*relProjX + relProjY*relProjY;
	int distDiagProj = relDiagProjX*relDiagProjX + relDiagProjY*relDiagProjY;
	if(distProj < distDiagProj || !allowDiag)
		return ui::Point(projX, projY);
	else
		return ui::Point(diagProjX, diagProjY);
}

int ConfigTool::getDist(ui::Point relPos, int offset)
{
	int signedDist = relPos.X ? relPos.X : relPos.Y;
	int dist = ((signedDist > 0) ? signedDist : -signedDist) - offset;
	return (dist < 0) ? 0 : dist;
}

int ConfigTool::getDist(Particle part, int sampleX, int sampleY, int offset, bool allowDiag)
{
	ui::Point proj = projectPoint(part, sampleX, sampleY, allowDiag);
	return getDist(proj, offset);
}

void ConfigTool::OnSelectFiltTmp(Simulation *sim, int tmp)
{
	if(!isSamePart(sim->parts[currId], configPart))
	{
		Reset();
		return;
	}
	sim->parts[currId].tmp = tmp;
}

void ConfigTool::CalculatePreview(int x, int y, Simulation *sim)
{
	cursorPos = ui::Point(x, y);
	bool allowDiag = !(configState == ConfigState::dtecTmp2);
	ui::Point proj = projectPoint(configPart, x, y, allowDiag);
	dirx = (proj.X == 0) ? 0 : ((proj.X > 0) ? 1 : -1);
	diry = (proj.Y == 0) ? 0 : ((proj.Y > 0) ? 1 : -1);
	switch(configState)
	{
	case ConfigState::drayTmp:
		configPart.tmp = getDist(proj);
		break;
	case ConfigState::drayTmp2:
		configPart.tmp2 = getDist(proj, configPart.tmp);
		break;
	case ConfigState::crayTmp2:
		configPart.tmp2 = getDist(proj);
		break;
	case ConfigState::crayTmp:
		configPart.tmp = getDist(proj, configPart.tmp2);
		break;
	case ConfigState::ldtcLife:
		configPart.life = getDist(proj);
		break;
	case ConfigState::ldtcTmp:
		configPart.tmp = getDist(proj, configPart.life);
		break;
	case ConfigState::dtecTmp2:
		configPart.tmp2 = getDist(proj);
		break;
	case ConfigState::convTmp:
		configPart.tmp = getPartAt(sim, ui::Point(x, y)).type;
		break;
	default:
		break;
	}
}

void ConfigTool::Click(Simulation *sim, Brush *brush, ui::Point position)
{
	if(configState != ConfigState::ready &&
		!isSamePart(sim->parts[currId], configPart))
	{
		Reset();
	}
	CalculatePreview(position.X, position.Y, sim);
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
		case PT_CRAY:
			configState = ConfigState::crayTmp2;
			break;
		case PT_LDTC:
			configState = ConfigState::ldtcLife;
			break;
		case PT_DTEC:
			configState = ConfigState::dtecTmp2;
			break;
		case PT_CONV:
			configState = ConfigState::convTmp;
			break;
		case PT_FILT:
			new FiltConfigWindow(this, sim);
			break;
		default:
			break;
		}
		break;
	case ConfigState::drayTmp:
		sim->parts[currId].tmp = configPart.tmp;
		configState = ConfigState::drayTmp2;
		break;
	case ConfigState::drayTmp2:
		sim->parts[currId].tmp2 = configPart.tmp2;
		configState = ConfigState::ready;
		break;
	case ConfigState::crayTmp2:
		sim->parts[currId].tmp2 = configPart.tmp2;
		configState = ConfigState::crayTmp;
		break;
	case ConfigState::crayTmp:
		sim->parts[currId].tmp = configPart.tmp;
		configState = ConfigState::ready;
		break;
	case ConfigState::ldtcLife:
		sim->parts[currId].life = configPart.life;
		configState = ConfigState::ldtcTmp;
		break;
	case ConfigState::ldtcTmp:
		sim->parts[currId].tmp = configPart.tmp;
		configState = ConfigState::ready;
		break;
	case ConfigState::dtecTmp2:
		sim->parts[currId].tmp2 = configPart.tmp2;
		configState = ConfigState::ready;
		break;
	case ConfigState::convTmp:
		sim->parts[currId].tmp = configPart.tmp;
		configState = ConfigState::ready;
		break;
	default:
		break;
	}
}

void ConfigTool::Reset()
{
	configState = ConfigState::ready;
}

Particle ConfigTool::GetPart()
{
	return configPart;
}

bool ConfigTool::IsConfigurableType(int type)
{
	return type == PT_DRAY || type == PT_CRAY || type == PT_LDTC ||
		type == PT_DTEC || type == PT_CONV || type == PT_FILT;
}

bool ConfigTool::IsConfiguring()
{
	return configState != ConfigState::ready;
}

bool ConfigTool::IsConfiguringTemp()
{
	return false;
}

bool ConfigTool::IsConfiguringLife()
{
	return configState == ConfigState::ldtcLife;
}

bool ConfigTool::IsConfiguringTmp()
{
	return configState == ConfigState::drayTmp ||
		configState == ConfigState::crayTmp ||
		configState == ConfigState::ldtcTmp ||
		configState == ConfigState::convTmp;
}

bool ConfigTool::IsConfiguringTmp2()
{
	return configState == ConfigState::drayTmp2 ||
		configState == ConfigState::crayTmp2 ||
		configState == ConfigState::dtecTmp2;
}

void ConfigTool::drawRedLine(Renderer *ren, int startx, int starty, int endx, int endy)
{
	ren->draw_line(startx, starty, endx, endy, 255, 0, 0, 255);
}

void ConfigTool::drawWhiteLine(Renderer *ren, int startx, int starty, int endx, int endy)
{
	ren->draw_line(startx, starty, endx, endy, 255, 200, 200, 255);
}

void ConfigTool::tripleLine(Renderer *ren, int firstLineLen, int midLineLen, bool drawFirstLine, bool drawThirdLine)
{
	int mid1x = configPart.x + dirx * firstLineLen,
		mid1y = configPart.y + diry * firstLineLen;
	int mid2x = mid1x + dirx * midLineLen,
		mid2y = mid1y + diry * midLineLen;
	if(drawFirstLine && firstLineLen > 0)
		drawWhiteLine(ren, configPart.x + dirx, configPart.y + diry, mid1x, mid1y);
	if(midLineLen > 0)
		drawRedLine(ren, mid1x + dirx, mid1y + diry, mid2x, mid2y);
	if(drawThirdLine && firstLineLen > 0)
		drawWhiteLine(ren, mid2x + dirx, mid2y + diry, mid2x + dirx * firstLineLen, mid2y + diry * firstLineLen);
}

void ConfigTool::DrawHUD(Renderer *ren)
{
	switch(configState)
	{
	case ConfigState::drayTmp:
		tripleLine(ren, 0, configPart.tmp, false, false);
		break;
	case ConfigState::drayTmp2:
		tripleLine(ren, configPart.tmp, configPart.tmp2);
		break;
	case ConfigState::crayTmp2:
		tripleLine(ren, 0, configPart.tmp2, false, false);
		break;
	case ConfigState::crayTmp:
		tripleLine(ren, configPart.tmp2, configPart.tmp, true, false);
		break;
	case ConfigState::ldtcLife:
		tripleLine(ren, 0, configPart.life, false, false);
		break;
	case ConfigState::ldtcTmp:
		tripleLine(ren, configPart.life, configPart.tmp, true, false);
		break;
	case ConfigState::dtecTmp2:
		ren->drawrect(configPart.x - configPart.tmp2, configPart.y - configPart.tmp2, configPart.tmp2 * 2 + 1, configPart.tmp2 * 2 + 1, 200, 200, 200, 255);
		break;
	case ConfigState::convTmp:
	{
		int partX = int(configPart.x+0.5f), partY = int(configPart.y+0.5f);
		drawRedLine(ren, partX, partY, partX, partY);
		ren->xor_line(cursorPos.X, cursorPos.Y, cursorPos.X, cursorPos.Y);
		break;
	}
	default:
		break;
	}
}
