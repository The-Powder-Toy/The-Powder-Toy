#ifndef LUASCRIPTINTERFACE_H_
#define LUASCRIPTINTERFACE_H_

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "CommandInterface.h"
#include "simulation/Simulation.h"

namespace ui
{
	class Window;
}

namespace pim
{
	class VirtualMachine;
}
class Tool;

//Because lua only has bindings for C, we're going to have to go outside "outside" the LuaScriptInterface, this means we can only have one instance :(

#define LOCAL_LUA_DIR "Lua"

#define LUACON_MDOWN 1
#define LUACON_MUP 2
#define LUACON_MPRESS 3
#define LUACON_KDOWN 1
#define LUACON_KUP 2

//Bitmasks for things that might need recalculating after changes to tpt.el
#define LUACON_EL_MODIFIED_CANMOVE 0x1
#define LUACON_EL_MODIFIED_GRAPHICS 0x2
#define LUACON_EL_MODIFIED_MENUS 0x4

class TPTScriptInterface;
class LuaScriptInterface: public CommandInterface
{
	int luacon_mousex, luacon_mousey, luacon_mousebutton, luacon_brushx, luacon_brushy;
	std::string luacon_selectedl, luacon_selectedr, luacon_selectedalt;
	bool luacon_mousedown;
	bool currentCommand;
	TPTScriptInterface * legacy;

	//Simulation
	static StructProperty * particleProperties;
	static int particlePropertiesCount;
	//
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

	//Renderer
	void initRendererAPI();
	static int renderer_renderModes(lua_State * l);
	static int renderer_displayModes(lua_State * l);
	static int renderer_colourMode(lua_State * l);
	static int renderer_decorations(lua_State * l);

	//Elements
	static pim::VirtualMachine * updateVirtualMachines[PT_NUM];
	static int updateVM(UPDATE_FUNC_ARGS);
	//
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

	//VM
	void initVirtualMachineAPI();
	static int virtualMachine_loadProgram(lua_State * l);

	void initGraphicsAPI();
	static int graphics_textSize(lua_State * l);
	static int graphics_drawText(lua_State * l);
	static int graphics_drawLine(lua_State * l);
	static int graphics_drawRect(lua_State * l);
	static int graphics_fillRect(lua_State * l);

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

public:
	ui::Window * Window;
	lua_State *l;
	LuaScriptInterface(GameController * c, GameModel * m);
	virtual bool OnBrushChanged(int brushType, int rx, int ry);
	virtual bool OnActiveToolChanged(int toolSelection, Tool * tool);
	virtual bool OnMouseMove(int x, int y, int dx, int dy);
	virtual bool OnMouseDown(int x, int y, unsigned button);
	virtual bool OnMouseUp(int x, int y, unsigned button);
	virtual bool OnMouseWheel(int x, int y, int d);
	virtual bool OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual bool OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void OnTick();
	virtual void Init();
	virtual void SetWindow(ui::Window * window);
	virtual int Command(std::string command);
	virtual std::string FormatCommand(std::string command);
	virtual ~LuaScriptInterface();
};


#endif /* LUASCRIPTINTERFACE_H_ */
