#ifndef INTERFACE_H
#define INTERFACE_H
#include <SDL/SDL.h>
#include "graphics.h"

struct menu_section
{
    char *icon;
    const char *name;
    int itemcount;
};
typedef struct menu_section menu_section;

struct menu_wall
{
    pixel colour;
    const char *descs;
};
typedef struct menu_wall menu_wall;

static menu_wall mwalls[] =
{
    {PIXPACK(0xC0C0C0), "Wall. Indestructible. Blocks everything. Conductive."},
    {PIXPACK(0x808080), "E-Wall. Becomes transparent when electricity is connected."},
    {PIXPACK(0xFF8080), "Detector. Generates electricity when a particle is inside."},
    {PIXPACK(0x808080), "Streamline. Set start point of a streamline."},
    {PIXPACK(0x808080), "Sign. Click on a sign to edit it or anywhere else to place a new one."},
    {PIXPACK(0x8080FF), "Fan. Accelerates air. Use line tool to set direction and strength."},
    {PIXPACK(0xC0C0C0), "Wall. Blocks most particles but lets liquids through. Conductive."},
    {PIXPACK(0x808080), "Wall. Absorbs particles but lets air currents through."},
    {PIXPACK(0x808080), "Erases walls."},
    {PIXPACK(0x808080), "Wall. Indestructible. Blocks everything."},
    {PIXPACK(0x3C3C3C), "Wall. Indestructible. Blocks particles, allows air"},
    {PIXPACK(0x575757), "Wall. Indestructible. Blocks liquids and gasses, allows solids"},
    {PIXPACK(0xFFFF22), "Conductor, allows particles, conducts electricity"},
    {PIXPACK(0x242424), "E-Hole, absorbs particles, release them when powered"},
    {PIXPACK(0xFFFFFF), "Air, creates airflow and pressure"},
    {PIXPACK(0xFFBB00), "Heats the targetted element."},
    {PIXPACK(0x00BBFF), "Cools the targetted element."},
    {PIXPACK(0x303030), "Vacuum, reduces air pressure."},
    {PIXPACK(0x579777), "Wall. Indestructible. Blocks liquids and solids, allows gasses"},
};

#define SC_WALL 0
#define SC_SPECIAL 8
#define SC_POWDERS 5
#define SC_SOLIDS 6
#define SC_ELEC 1
#define SC_EXPLOSIVE 2
#define SC_GAS 3
#define SC_LIQUID 4
#define SC_NUCLEAR 7
#define SC_TOTAL 9

static menu_section msections[] =
{
    {"\xC1", "Walls", 0},
    {"\xC2", "Electronics", 0},
    {"\xC3", "Explosives", 0},
    {"\xC5", "Gasses", 0},
    {"\xC4", "Liquids", 0},
    {"\xD0", "Powders", 0},
    {"\xD1", "Solids", 0},
    {"\xC6", "Radioactive", 0},
    {"\xCC", "Special", 0},
};

struct ui_edit
{
    int x, y, w, nx;
    char str[256],*def;
    int focus, cursor, hide;
};
typedef struct ui_edit ui_edit;

struct ui_checkbox
{
    int x, y;
    int focus, checked;
};
typedef struct ui_checkbox ui_checkbox;

extern SDLMod sdl_mod;
extern int sdl_key, sdl_wheel, sdl_caps, sdl_ascii, sdl_zoom_trig;
extern char *shift_0;
extern char *shift_1;
extern int svf_login;
extern int svf_admin;
extern int svf_mod;
extern char svf_user[64];
extern char svf_pass[64];

extern int svf_open;
extern int svf_own;
extern int svf_myvote;
extern int svf_publish;
extern char svf_id[16];
extern char svf_name[64];
extern char svf_tags[256];
extern void *svf_last;
extern int svf_lsize;

extern char *search_ids[GRID_X*GRID_Y];
extern char *search_dates[GRID_X*GRID_Y];
extern int   search_votes[GRID_X*GRID_Y];
extern int   search_publish[GRID_X*GRID_Y];
extern int	  search_scoredown[GRID_X*GRID_Y];
extern int	  search_scoreup[GRID_X*GRID_Y];
extern char *search_names[GRID_X*GRID_Y];
extern char *search_owners[GRID_X*GRID_Y];
extern void *search_thumbs[GRID_X*GRID_Y];
extern int   search_thsizes[GRID_X*GRID_Y];

extern int search_own;
extern int search_date;
extern int search_page;
extern char search_expr[256];

extern char *tag_names[TAG_MAX];
extern int tag_votes[TAG_MAX];

extern int Z_keysym;

extern int zoom_en;
extern int zoom_x, zoom_y;
extern int zoom_wx, zoom_wy;

void menu_count(void);

void get_sign_pos(int i, int *x0, int *y0, int *w, int *h);

void add_sign_ui(pixel *vid_buf, int mx, int my);

void ui_edit_draw(pixel *vid_buf, ui_edit *ed);

void ui_edit_process(int mx, int my, int mb, ui_edit *ed);

void ui_checkbox_draw(pixel *vid_buf, ui_checkbox *ed);

void ui_checkbox_process(int mx, int my, int mb, int mbq, ui_checkbox *ed);

void draw_svf_ui(pixel *vid_buf);

void error_ui(pixel *vid_buf, int err, char *txt);

void info_ui(pixel *vid_buf, char *top, char *txt);

void info_box(pixel *vid_buf, char *msg);

int confirm_ui(pixel *vid_buf, char *top, char *msg, char *btn);

void login_ui(pixel *vid_buf);

int stamp_ui(pixel *vid_buf);

void tag_list_ui(pixel *vid_buf);

int save_name_ui(pixel *vid_buf);

void menu_ui(pixel *vid_buf, int i, int *sl, int *sr);

void menu_ui_v3(pixel *vid_buf, int i, int *sl, int *sr, int b, int bq, int mx, int my);

int sdl_poll(void);

void set_cmode(int cm);

char *download_ui(pixel *vid_buf, char *uri, int *len);

int search_ui(pixel *vid_buf);

int search_results(char *str, int votes);

int execute_tagop(pixel *vid_buf, char *op, char *tag);

void execute_save(pixel *vid_buf);

void execute_delete(pixel *vid_buf, char *id);

int execute_vote(pixel *vid_buf, char *id, char *action);

#endif

