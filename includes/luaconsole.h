/**
 * Powder Toy - Lua console (header)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

int *lua_el_func, *lua_el_mode, *lua_gr_func;

void luacon_open();
int luacon_step(int mx, int my, int selectl, int selectr);
int luacon_mouseevent(int mx, int my, int mb, int event);
int luacon_keyevent(int key, int modifier, int event);
int luacon_eval(char *command);
int luacon_part_update(int t, int i, int x, int y, int surround_space, int nt);
int luacon_graphics_update(int t, int i, int *pixel_mode, int *cola, int *colr, int *colg, int *colb, int *firea, int *firer, int *fireg, int *fireb);
char *luacon_geterror();
void luacon_close();
int luacon_partsread(lua_State* l);
int luacon_partswrite(lua_State* l);
int luacon_partread(lua_State* l);
int luacon_partwrite(lua_State* l);
int luacon_elementread(lua_State* l);
int luacon_elementwrite(lua_State* l);
int luacon_transitionread(lua_State* l);
int luacon_transitionwrite(lua_State* l);
int luacon_particle_getproperty(char * key, int * format);
int luacon_transition_getproperty(char * key, int * format);
int luacon_element_getproperty(char * key, int * format, unsigned int * modified_stuff);
int process_command_lua(pixel *vid_buf, char *console, char *console_error);
void lua_hook(lua_State *L, lua_Debug *ar);

int getPartIndex_curIdx;

//TPT Interface
int luatpt_test(lua_State* l);
int luatpt_getelement(lua_State *l);
int luatpt_element_func(lua_State *l);
int luatpt_graphics_func(lua_State *l);
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
int luatpt_set_wallmap(lua_State* l);
int luatpt_get_wallmap(lua_State* l);
int luatpt_set_elecmap(lua_State* l);
int luatpt_get_elecmap(lua_State* l);
int luatpt_drawpixel(lua_State* l);
int luatpt_drawrect(lua_State* l);
int luatpt_fillrect(lua_State* l);
int luatpt_drawline(lua_State* l);
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
int luatpt_screenshot(lua_State* l);
#endif
