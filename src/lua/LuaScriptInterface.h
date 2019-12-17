#ifndef LUASCRIPTINTERFACE_H_
#define LUASCRIPTINTERFACE_H_
#include "Config.h"

#include "LuaCompat.h"
#include "LuaSmartRef.h"

#include "CommandInterface.h"
#include "lua/LuaEvents.h"
#include "simulation/StructProperty.h"

#include <map>

namespace ui
{
	class Window;
}

class Tool;

//Because lua only has bindings for C, we're going to have to go outside "outside" the LuaScriptInterface, this means we can only have one instance :(

#define LUACON_MDOWN 1
#define LUACON_MUP 2
#define LUACON_MPRESS 3
#define LUACON_MUPALT 4
#define LUACON_MUPZOOM 5
#define LUACON_KDOWN 1
#define LUACON_KUP 2

//Bitmasks for things that might need recalculating after changes to tpt.el
#define LUACON_EL_MODIFIED_CANMOVE 0x1
#define LUACON_EL_MODIFIED_GRAPHICS 0x2
#define LUACON_EL_MODIFIED_MENUS 0x4

// idea from mniip, makes things much simpler
#define SETCONST(L, NAME)\
	lua_pushinteger(L, NAME);\
	lua_setfield(L, -2, #NAME)

class Simulation;
class TPTScriptInterface;
class LuaComponent;

class LuaScriptInterface: public CommandInterface
{
	int luacon_mousex, luacon_mousey, luacon_mousebutton;
	ByteString luacon_selectedl, luacon_selectedr, luacon_selectedalt, luacon_selectedreplace;
	bool luacon_mousedown;
	bool currentCommand;
	TPTScriptInterface * legacy;

	// signs
	static int simulation_signIndex(lua_State *l);
	static int simulation_signNewIndex(lua_State *l);
	static int simulation_newsign(lua_State *l);

	//Simulation

	void initSimulationAPI();
	static void set_map(int x, int y, int width, int height, float value, int mapType);
	static int simulation_partNeighbours(lua_State * l);
	static int simulation_partChangeType(lua_State * l);
	static int simulation_partCreate(lua_State * l);
	static int simulation_partProperty(lua_State * l);
	static int simulation_partPosition(lua_State * l);
	static int simulation_partID(lua_State * l);
	static int simulation_partKill(lua_State * l);
	static int simulation_pressure(lua_State * l);
	static int simulation_velocityX(lua_State * l);
	static int simulation_velocityY(lua_State * l);
	static int simulation_gravMap(lua_State * l);
	static int simulation_ambientHeat(lua_State * l);
	static int simulation_createParts(lua_State * l);
	static int simulation_createLine(lua_State * l);
	static int simulation_createBox(lua_State * l);
	static int simulation_floodParts(lua_State * l);
	static int simulation_createWalls(lua_State * l);
	static int simulation_createWallLine(lua_State * l);
	static int simulation_createWallBox(lua_State * l);
	static int simulation_floodWalls(lua_State * l);
	static int simulation_toolBrush(lua_State * l);
	static int simulation_toolLine(lua_State * l);
	static int simulation_toolBox(lua_State * l);
	static int simulation_floodProp(lua_State * l);
	static int simulation_decoBrush(lua_State * l);
	static int simulation_decoLine(lua_State * l);
	static int simulation_decoBox(lua_State * l);
	static int simulation_decoColor(lua_State * l);
	static int simulation_clearSim(lua_State * l);
	static int simulation_clearRect(lua_State * l);
	static int simulation_resetTemp(lua_State * l);
	static int simulation_resetPressure(lua_State * l);
	static int simulation_saveStamp(lua_State * l);
	static int simulation_loadStamp(lua_State * l);
	static int simulation_deleteStamp(lua_State * l);
	static int simulation_loadSave(lua_State * l);
	static int simulation_reloadSave(lua_State * l);
	static int simulation_getSaveID(lua_State * l);
	static int simulation_adjustCoords(lua_State * l);
	static int simulation_prettyPowders(lua_State * l);
	static int simulation_gravityGrid(lua_State * l);
	static int simulation_edgeMode(lua_State * l);
	static int simulation_gravityMode(lua_State * l);
	static int simulation_airMode(lua_State * l);
	static int simulation_waterEqualisation(lua_State * l);
	static int simulation_ambientAirTemp(lua_State * l);
	static int simulation_elementCount(lua_State * l);
	static int simulation_canMove(lua_State * l);
	static int simulation_parts(lua_State * l);
	static int simulation_brush(lua_State * l);
	static int simulation_pmap(lua_State * l);
	static int simulation_photons(lua_State * l);
	static int simulation_neighbours(lua_State * l);
	static int simulation_framerender(lua_State * l);
	static int simulation_gspeed(lua_State * l);
	static int simulation_takeSnapshot(lua_State *l);

	//Renderer
	void initRendererAPI();
	static int renderer_renderModes(lua_State * l);
	static int renderer_displayModes(lua_State * l);
	static int renderer_colourMode(lua_State * l);
	static int renderer_decorations(lua_State * l);
	static int renderer_grid(lua_State * l);
	static int renderer_debugHUD(lua_State * l);
	static int renderer_depth3d(lua_State * l);
	static int renderer_zoomEnabled(lua_State *l);
	static int renderer_zoomWindowInfo(lua_State *l);
	static int renderer_zoomScopeInfo(lua_State *l);

	//Elements
	void initElementsAPI();
	static int elements_allocate(lua_State * l);
	static int elements_element(lua_State * l);
	static int elements_property(lua_State * l);
	static int elements_loadDefault(lua_State * l);
	static int elements_free(lua_State * l);

	//Interface
	void initInterfaceAPI();
	static int interface_showWindow(lua_State * l);
	static int interface_closeWindow(lua_State * l);
	static int interface_addComponent(lua_State * l);
	static int interface_removeComponent(lua_State * l);

	void initGraphicsAPI();
	static int graphics_textSize(lua_State * l);
	static int graphics_drawText(lua_State * l);
	static int graphics_drawLine(lua_State * l);
	static int graphics_drawRect(lua_State * l);
	static int graphics_fillRect(lua_State * l);
	static int graphics_drawCircle(lua_State * l);
	static int graphics_fillCircle(lua_State * l);
	static int graphics_getColors(lua_State * l);
	static int graphics_getHexColor(lua_State * l);

	void initFileSystemAPI();
	static int fileSystem_list(lua_State * l);
	static int fileSystem_exists(lua_State * l);
	static int fileSystem_isFile(lua_State * l);
	static int fileSystem_isDirectory(lua_State * l);
	static int fileSystem_makeDirectory(lua_State * l);
	static int fileSystem_removeDirectory(lua_State * l);
	static int fileSystem_removeFile(lua_State * l);
	static int fileSystem_move(lua_State * l);
	static int fileSystem_copy(lua_State * l);

	void initPlatformAPI();
	static int platform_platform(lua_State * l);
	static int platform_build(lua_State * l);
	static int platform_releaseType(lua_State * l);
	static int platform_exeName(lua_State * l);
	static int platform_restart(lua_State * l);
	static int platform_openLink(lua_State * l);
	static int platform_clipboardCopy(lua_State * l);
	static int platform_clipboardPaste(lua_State * l);

	void initEventAPI();
	static int event_register(lua_State * l);
	static int event_unregister(lua_State * l);
	static int event_getmodifiers(lua_State * l);

	void initHttpAPI();
	static int http_get(lua_State * l);
	static int http_post(lua_State * l);

	std::vector<LuaSmartRef> lua_el_func_v, lua_gr_func_v, lua_cd_func_v;
	std::vector<int> lua_el_mode_v;

public:
	int tpt_index(lua_State *l);
	int tpt_newIndex(lua_State *l);

	static void LuaGetProperty(lua_State* l, StructProperty property, intptr_t propertyAddress);
	static void LuaSetProperty(lua_State* l, StructProperty property, intptr_t propertyAddress, int stackPos);

	ui::Window * Window;
	lua_State *l;
	std::map<LuaComponent *, LuaSmartRef> grabbed_components;
	LuaScriptInterface(GameController * c, GameModel * m);

	void OnTick() override;
	bool HandleEvent(LuaEvents::EventTypes eventType, Event * event) override;

	void Init();
	void SetWindow(ui::Window * window);
	int Command(String command) override;
	String FormatCommand(String command) override;
	virtual ~LuaScriptInterface();
};


#endif /* LUASCRIPTINTERFACE_H_ */
