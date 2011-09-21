#ifndef LUACONSOLEH
#define LUACONSOLEH
#ifdef LUA_R_INCL
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#else
#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
#endif
#include <defines.h>

#define LUACON_MDOWN 1
#define LUACON_MUP 2
#define LUACON_MPRESS 3
#define LUACON_KDOWN 1
#define LUACON_KUP 2

void luacon_open();
int luacon_step(int mx, int my);
int luacon_mouseevent(int mx, int my, int mb, int event);
int luacon_keyevent(int key, int modifier, int event);
int luacon_eval(char *command);
char *luacon_geterror();
void luacon_close();
int process_command_lua(pixel *vid_buf, char *console, char *console_error);

int getPartIndex_curIdx;

//TPT Interface
int luatpt_test(lua_State* l);
int luatpt_drawtext(lua_State* l);
int luatpt_create(lua_State* l);
int luatpt_setpause(lua_State* l);
int luatpt_togglepause(lua_State* l);
int luatpt_togglewater(lua_State* l);
int luatpt_setconsole(lua_State* l);
int luatpt_log(lua_State* l);
int luatpt_set_pressure(lua_State* l);
int luatpt_set_gravity(lua_State* l);
int luatpt_reset_gravity_field(lua_State* l);
int luatpt_reset_velocity(lua_State* l);
int luatpt_reset_spark(lua_State* l);
int luatpt_set_property(lua_State* l);
int luatpt_get_property(lua_State* l);
int luatpt_drawpixel(lua_State* l);
int luatpt_drawrect(lua_State* l);
int luatpt_fillrect(lua_State* l);
int luatpt_textwidth(lua_State* l);
int luatpt_get_name(lua_State* l);
int luatpt_set_shortcuts(lua_State* l);
int luatpt_delete(lua_State* l);
int luatpt_register_step(lua_State* l);
int luatpt_unregister_step(lua_State* l);
int luatpt_register_mouseclick(lua_State* l);
int luatpt_unregister_mouseclick(lua_State* l);
int luatpt_register_keypress(lua_State* l);
int luatpt_unregister_keypress(lua_State* l);
int luatpt_input(lua_State* l);
int luatpt_message_box(lua_State* l);
int luatpt_get_numOfParts(lua_State* l);
int luatpt_start_getPartIndex(lua_State* l);
int luatpt_getPartIndex(lua_State* l);
int luatpt_next_getPartIndex(lua_State* l);
int luatpt_hud(lua_State* l);
int luatpt_gravity(lua_State* l);
int luatpt_airheat(lua_State* l);
int luatpt_active_menu(lua_State* l);
int luatpt_decorations_enable(lua_State* l);
int luatpt_cmode_set(lua_State* l);
int luatpt_error(lua_State* l);
int luatpt_heat(lua_State* l);
int luatpt_setfire(lua_State* l);
int luatpt_setdebug(lua_State* l);
int luatpt_setfpscap(lua_State* l);
int luatpt_getscript(lua_State* l);
int luatpt_setwindowsize(lua_State* l);
#endif
