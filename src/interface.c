#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bzlib.h>
#include <math.h>
#include <time.h>
#include <http.h>
#include <md5.h>
#include <font.h>
#include <defines.h>
#include <powder.h>
#include <interface.h>
#include <misc.h>

SDLMod sdl_mod;
int sdl_key, sdl_wheel, sdl_caps=0, sdl_ascii, sdl_zoom_trig=0;

char *shift_0="`1234567890-=[]\\;',./";
char *shift_1="~!@#$%^&*()_+{}|:\"<>?";

int svf_login = 0;
int svf_admin = 0;
int svf_mod = 0;
char svf_user[64] = "";
char svf_pass[64] = "";

int svf_open = 0;
int svf_own = 0;
int svf_myvote = 0;
int svf_publish = 0;
char svf_id[16] = "";
char svf_name[64] = "";
char svf_tags[256] = "";
void *svf_last = NULL;
int svf_lsize;

char *search_ids[GRID_X*GRID_Y];
char *search_dates[GRID_X*GRID_Y];
int   search_votes[GRID_X*GRID_Y];
int   search_publish[GRID_X*GRID_Y];
int	  search_scoredown[GRID_X*GRID_Y];
int	  search_scoreup[GRID_X*GRID_Y];
char *search_names[GRID_X*GRID_Y];
char *search_owners[GRID_X*GRID_Y];
void *search_thumbs[GRID_X*GRID_Y];
int   search_thsizes[GRID_X*GRID_Y];

int search_own = 0;
int search_date = 0;
int search_page = 0;
char search_expr[256] = "";

char *tag_names[TAG_MAX];
int tag_votes[TAG_MAX];

int Z_keysym = 'z';

int zoom_en = 0;
int zoom_x=(XRES-ZSIZE_D)/2, zoom_y=(YRES-ZSIZE_D)/2;
int zoom_wx=0, zoom_wy=0;
unsigned char ZFACTOR = 256/ZSIZE_D;
unsigned char ZSIZE = ZSIZE_D;

void menu_count(void)
{
    int i=0;
    msections[SC_WALL].itemcount = UI_WALLCOUNT-4;
    msections[SC_SPECIAL].itemcount = 4;
    for(i=0; i<PT_NUM; i++)
    {
        msections[ptypes[i].menusection].itemcount+=ptypes[i].menu;
    }

}

void get_sign_pos(int i, int *x0, int *y0, int *w, int *h)
{
    //Changing width if sign have special content
    if(strcmp(signs[i].text, "{p}")==0)
        *w = textwidth("Pressure: -000.00");

    if(strcmp(signs[i].text, "{t}")==0)
        *w = textwidth("Temp: 0000.00");

    //Ususal width
    if(strcmp(signs[i].text, "{p}") && strcmp(signs[i].text, "{t}"))
        *w = textwidth(signs[i].text) + 5;
    *h = 14;
    *x0 = (signs[i].ju == 2) ? signs[i].x - *w :
          (signs[i].ju == 1) ? signs[i].x - *w/2 : signs[i].x;
    *y0 = (signs[i].y > 18) ? signs[i].y - 18 : signs[i].y + 4;
}

void add_sign_ui(pixel *vid_buf, int mx, int my)
{
    int i, w, h, x, y, nm=0, ju;
    int x0=(XRES-192)/2,y0=(YRES-80)/2,b=1,bq;
    ui_edit ed;

    // check if it is an existing sign
    for(i=0; i<MAXSIGNS; i++)
        if(signs[i].text[0])
        {
            get_sign_pos(i, &x, &y, &w, &h);
            if(mx>=x && mx<=x+w && my>=y && my<=y+h)
                break;
        }
    // else look for empty spot
    if(i >= MAXSIGNS)
    {
        nm = 1;
        for(i=0; i<MAXSIGNS; i++)
            if(!signs[i].text[0])
                break;
    }
    if(i >= MAXSIGNS)
        return;

    if(nm)
    {
        signs[i].x = mx;
        signs[i].y = my;
        signs[i].ju = 1;
    }

    while(!sdl_poll())
    {
        b = SDL_GetMouseState(&mx, &my);
        if(!b)
            break;
    }

    ed.x = x0+25;
    ed.y = y0+25;
    ed.w = 158;
    ed.nx = 1;
    ed.def = "[message]";
    ed.focus = 1;
    ed.hide = 0;
    ed.cursor = strlen(signs[i].text);
    strcpy(ed.str, signs[i].text);
    ju = signs[i].ju;

    fillrect(vid_buf, -1, -1, XRES, YRES+MENUSIZE, 0, 0, 0, 192);
    while(!sdl_poll())
    {
        bq = b;
        b = SDL_GetMouseState(&mx, &my);
        mx /= sdl_scale;
        my /= sdl_scale;

        drawrect(vid_buf, x0, y0, 192, 80, 192, 192, 192, 255);
        clearrect(vid_buf, x0, y0, 192, 80);
        drawtext(vid_buf, x0+8, y0+8, nm ? "New sign:" : "Edit sign:", 255, 255, 255, 255);
        drawtext(vid_buf, x0+12, y0+23, "\xA1", 32, 64, 128, 255);
        drawtext(vid_buf, x0+12, y0+23, "\xA0", 255, 255, 255, 255);
        drawrect(vid_buf, x0+8, y0+20, 176, 16, 192, 192, 192, 255);
        ui_edit_draw(vid_buf, &ed);
        drawtext(vid_buf, x0+8, y0+46, "Justify:", 255, 255, 255, 255);
        draw_icon(vid_buf, x0+50, y0+42, 0x9D, ju == 0);
        draw_icon(vid_buf, x0+68, y0+42, 0x9E, ju == 1);
        draw_icon(vid_buf, x0+86, y0+42, 0x9F, ju == 2);

        if(!nm)
        {
            drawtext(vid_buf, x0+138, y0+45, "\x86", 160, 48, 32, 255);
            drawtext(vid_buf, x0+138, y0+45, "\x85", 255, 255, 255, 255);
            drawtext(vid_buf, x0+152, y0+46, "Delete", 255, 255, 255, 255);
            drawrect(vid_buf, x0+134, y0+42, 50, 15, 255, 255, 255, 255);
        }

        drawtext(vid_buf, x0+5, y0+69, "OK", 255, 255, 255, 255);
        drawrect(vid_buf, x0, y0+64, 192, 16, 192, 192, 192, 255);

        sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

        ui_edit_process(mx, my, b, &ed);

        if(b && !bq && mx>=x0+50 && mx<=x0+67 && my>=y0+42 && my<=y0+59)
            ju = 0;
        if(b && !bq && mx>=x0+68 && mx<=x0+85 && my>=y0+42 && my<=y0+59)
            ju = 1;
        if(b && !bq && mx>=x0+86 && mx<=x0+103 && my>=y0+42 && my<=y0+59)
            ju = 2;

        if(b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+22 && my<y0+36)
            break;
        if(b && !bq && mx>=x0 && mx<x0+192 && my>=y0+64 && my<=y0+80)
            break;

        if(!nm && b && !bq && mx>=x0+134 && my>=y0+42 && mx<=x0+184 && my<=y0+59)
        {
            signs[i].text[0] = 0;
            return;
        }

        if(sdl_key==SDLK_RETURN)
            break;
        if(sdl_key==SDLK_ESCAPE)
        {
            if(!ed.focus)
                return;
            ed.focus = 0;
        }
    }

    strcpy(signs[i].text, ed.str);
    signs[i].ju = ju;
}

void ui_edit_draw(pixel *vid_buf, ui_edit *ed)
{
    int cx, i;
    char echo[256], *str;

    if(ed->hide)
    {
        for(i=0; ed->str[i]; i++)
            echo[i] = 0x8D;
        echo[i] = 0;
        str = echo;
    }
    else
        str = ed->str;

    if(ed->str[0])
    {
        drawtext(vid_buf, ed->x, ed->y, str, 255, 255, 255, 255);
        drawtext(vid_buf, ed->x+ed->w-11, ed->y-1, "\xAA", 128, 128, 128, 255);
    }
    else if(!ed->focus)
        drawtext(vid_buf, ed->x, ed->y, ed->def, 128, 128, 128, 255);
    if(ed->focus)
    {
        cx = textnwidth(str, ed->cursor);
        for(i=-3; i<9; i++)
            drawpixel(vid_buf, ed->x+cx, ed->y+i, 255, 255, 255, 255);
    }
}

void ui_edit_process(int mx, int my, int mb, ui_edit *ed)
{
    char ch, ts[2], echo[256], *str;
    int l, i;
#ifdef RAWINPUT
    char *p;
#endif

    if(mb)
    {
        if(ed->hide)
        {
            for(i=0; ed->str[i]; i++)
                echo[i] = 0x8D;
            echo[i] = 0;
            str = echo;
        }
        else
            str = ed->str;

        if(mx>=ed->x+ed->w-11 && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+11)
        {
            ed->focus = 1;
            ed->cursor = 0;
            ed->str[0] = 0;
        }
        else if(mx>=ed->x-ed->nx && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+11)
        {
            ed->focus = 1;
            ed->cursor = textwidthx(str, mx-ed->x);
        }
        else
            ed->focus = 0;
    }
    if(ed->focus && sdl_key)
    {
        if(ed->hide)
        {
            for(i=0; ed->str[i]; i++)
                echo[i] = 0x8D;
            echo[i] = 0;
            str = echo;
        }
        else
            str = ed->str;

        l = strlen(ed->str);
        switch(sdl_key)
        {
        case SDLK_HOME:
            ed->cursor = 0;
            break;
        case SDLK_END:
            ed->cursor = l;
            break;
        case SDLK_LEFT:
            if(ed->cursor > 0)
                ed->cursor --;
            break;
        case SDLK_RIGHT:
            if(ed->cursor < l)
                ed->cursor ++;
            break;
        case SDLK_DELETE:
            if(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))
                ed->str[ed->cursor] = 0;
            else if(ed->cursor < l)
                memmove(ed->str+ed->cursor, ed->str+ed->cursor+1, l-ed->cursor);
            break;
        case SDLK_BACKSPACE:
            if(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))
            {
                if(ed->cursor > 0)
                    memmove(ed->str, ed->str+ed->cursor, l-ed->cursor+1);
                ed->cursor = 0;
            }
            else if(ed->cursor > 0)
            {
                ed->cursor--;
                memmove(ed->str+ed->cursor, ed->str+ed->cursor+1, l-ed->cursor);
            }
            break;
        default:
#ifdef RAWINPUT
            if(sdl_key>=SDLK_SPACE && sdl_key<=SDLK_z && l<255)
            {
                ch = sdl_key;
                if((sdl_mod & (KMOD_LSHIFT|KMOD_RSHIFT|KMOD_CAPS)))
                {
                    if(ch>='a' && ch<='z')
                        ch &= ~0x20;
                    p = strchr(shift_0, ch);
                    if(p)
                        ch = shift_1[p-shift_0];
                }
                ts[0]=ed->hide?0x8D:ch;
                ts[1]=0;
                if(textwidth(str)+textwidth(ts) > ed->w-14)
                    break;
                memmove(ed->str+ed->cursor+1, ed->str+ed->cursor, l+1-ed->cursor);
                ed->str[ed->cursor] = ch;
                ed->cursor++;
            }
#else
            if(sdl_ascii>=' ' && sdl_ascii<127)
            {
                ch = sdl_ascii;
                ts[0]=ed->hide?0x8D:ch;
                ts[1]=0;
                if(textwidth(str)+textwidth(ts) > ed->w-14)
                    break;
                memmove(ed->str+ed->cursor+1, ed->str+ed->cursor, l+1-ed->cursor);
                ed->str[ed->cursor] = ch;
                ed->cursor++;
            }
#endif
            break;
        }
    }
}

void ui_checkbox_draw(pixel *vid_buf, ui_checkbox *ed)
{
    int w = 12;
    if(ed->checked)
    {
        drawtext(vid_buf, ed->x+2, ed->y+2, "\xCF", 128, 128, 128, 255);
    }
    if(ed->focus)
    {
        drawrect(vid_buf, ed->x, ed->y, w, w, 255, 255, 255, 255);
    }
    else
    {
        drawrect(vid_buf, ed->x, ed->y, w, w, 128, 128, 128, 255);
    }
}

void ui_checkbox_process(int mx, int my, int mb, int mbq, ui_checkbox *ed)
{
    int w = 12;

    if(mb && !mbq)
    {
        if(mx>=ed->x && mx<=ed->x+w && my>=ed->y && my<=ed->y+w)
        {
            ed->checked = (ed->checked)?0:1;
        }
    }
    else
    {
        if(mx>=ed->x && mx<=ed->x+w && my>=ed->y && my<=ed->y+w)
        {
            ed->focus = 1;
        }
        else
        {
            ed->focus = 0;
        }
    }
}

void draw_svf_ui(pixel *vid_buf)
{
    int c;

    drawtext(vid_buf, 4, YRES+(MENUSIZE-14), "\x81", 255, 255, 255, 255);
    drawrect(vid_buf, 1, YRES+(MENUSIZE-16), 16, 14, 255, 255, 255, 255);

    c = svf_open ? 255 : 128;
    drawtext(vid_buf, 23, YRES+(MENUSIZE-14), "\x91", c, c, c, 255);
    drawrect(vid_buf, 19, YRES+(MENUSIZE-16), 16, 14, c, c, c, 255);

    c = svf_login ? 255 : 128;
    drawtext(vid_buf, 40, YRES+(MENUSIZE-14), "\x82", c, c, c, 255);
    if(svf_open)
        drawtext(vid_buf, 58, YRES+(MENUSIZE-12), svf_name, c, c, c, 255);
    else
        drawtext(vid_buf, 58, YRES+(MENUSIZE-12), "[untitled simulation]", c, c, c, 255);
    drawrect(vid_buf, 37, YRES+(MENUSIZE-16), 150, 14, c, c, c, 255);
    if(svf_open && svf_own)
        drawdots(vid_buf, 55, YRES+(MENUSIZE-15), 12, c, c, c, 255);

    c = (svf_login && svf_open) ? 255 : 128;

    drawrect(vid_buf, 189, YRES+(MENUSIZE-16), 14, 14, c, c, c, 255);
    drawrect(vid_buf, 203, YRES+(MENUSIZE-16), 14, 14, c, c, c, 255);

    if(svf_myvote==1 && (svf_login && svf_open))
    {
        fillrect(vid_buf, 189, YRES+(MENUSIZE-16), 14, 14, 0, 108, 10, 255);
    }
    else if(svf_myvote==-1 && (svf_login && svf_open))
    {
        fillrect(vid_buf, 203, YRES+(MENUSIZE-16), 14, 14, 108, 10, 0, 255);
    }

    drawtext(vid_buf, 192, YRES+(MENUSIZE-12), "\xCB", 0, 187, 18, c);
    drawtext(vid_buf, 205, YRES+(MENUSIZE-14), "\xCA", 187, 40, 0, c);

    drawtext(vid_buf, 222, YRES+(MENUSIZE-15), "\x83", c, c, c, 255);
    if(svf_tags[0])
        drawtextmax(vid_buf, 240, YRES+(MENUSIZE-12), 154, svf_tags, c, c, c, 255);
    else
        drawtext(vid_buf, 240, YRES+(MENUSIZE-12), "[no tags set]", c, c, c, 255);

    drawrect(vid_buf, 219, YRES+(MENUSIZE-16), XRES+BARSIZE-380, 14, c, c, c, 255);

    drawtext(vid_buf, XRES-139+BARSIZE/*371*/, YRES+(MENUSIZE-14), "\x92", 255, 255, 255, 255);
    drawrect(vid_buf, XRES-143+BARSIZE/*367*/, YRES+(MENUSIZE-16), 16, 14, 255, 255, 255, 255);

    drawtext(vid_buf, XRES-122+BARSIZE/*388*/, YRES+(MENUSIZE-13), "\x84", 255, 255, 255, 255);
    if(svf_login)
        drawtext(vid_buf, XRES-104+BARSIZE/*406*/, YRES+(MENUSIZE-12), svf_user, 255, 255, 255, 255);
    else
        drawtext(vid_buf, XRES-104+BARSIZE/*406*/, YRES+(MENUSIZE-12), "[sign in]", 255, 255, 255, 255);
    drawrect(vid_buf, XRES-125+BARSIZE/*385*/, YRES+(MENUSIZE-16), 91, 14, 255, 255, 255, 255);

    if(sys_pause)
    {
        fillrect(vid_buf, XRES-17+BARSIZE/*493*/, YRES+(MENUSIZE-17), 16, 16, 255, 255, 255, 255);
        drawtext(vid_buf, XRES-14+BARSIZE/*496*/, YRES+(MENUSIZE-14), "\x90", 0, 0, 0, 255);
    }
    else
    {
        drawtext(vid_buf, XRES-14+BARSIZE/*496*/, YRES+(MENUSIZE-14), "\x90", 255, 255, 255, 255);
        drawrect(vid_buf, XRES-16+BARSIZE/*494*/, YRES+(MENUSIZE-16), 14, 14, 255, 255, 255, 255);
    }

    if(!legacy_enable)
    {
        fillrect(vid_buf, XRES-160+BARSIZE/*493*/, YRES+(MENUSIZE-17), 16, 16, 255, 255, 255, 255);
        drawtext(vid_buf, XRES-154+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xBE", 255, 0, 0, 255);
        drawtext(vid_buf, XRES-154+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xBD", 0, 0, 0, 255);
    }
    else
    {
        drawtext(vid_buf, XRES-154+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xBD", 255, 255, 255, 255);
        drawrect(vid_buf, XRES-159+BARSIZE/*494*/, YRES+(MENUSIZE-16), 14, 14, 255, 255, 255, 255);
    }

    switch(cmode)
    {
    case 0:
        drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x98", 128, 160, 255, 255);
        break;
    case 1:
        drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x99", 255, 212, 32, 255);
        break;
    case 2:
        drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x9A", 212, 212, 212, 255);
        break;
    case 3:
        drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x9B", 255, 0, 0, 255);
        drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x9C", 255, 255, 64, 255);
        break;
    case 4:
        drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xBF", 55, 255, 55, 255);
        break;
    case 5:
        drawtext(vid_buf, XRES-27+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xBE", 255, 0, 0, 255);
        drawtext(vid_buf, XRES-27+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xBD", 255, 255, 255, 255);
        break;
    case 6:
        drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xC4", 100, 150, 255, 255);
        break;
    }
    drawrect(vid_buf, XRES-32+BARSIZE/*478*/, YRES+(MENUSIZE-16), 14, 14, 255, 255, 255, 255);

    if(svf_admin)
    {
        drawtext(vid_buf, XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC9", 232, 127, 35, 255);
        drawtext(vid_buf, XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC7", 255, 255, 255, 255);
        drawtext(vid_buf, XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC8", 255, 255, 255, 255);
    }
    else if(svf_mod)
    {
        drawtext(vid_buf, XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC9", 35, 127, 232, 255);
        drawtext(vid_buf, XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC7", 255, 255, 255, 255);
    }//else if(amd)
    //	drawtext(vid_buf, XRES-45/*465*/, YRES+(MENUSIZE-15), "\x97", 0, 230, 153, 255); Why is this here?
}

void error_ui(pixel *vid_buf, int err, char *txt)
{
    int x0=(XRES-240)/2,y0=(YRES-MENUSIZE)/2,b=1,bq,mx,my;
    char *msg;

    msg = malloc(strlen(txt)+16);
    if(err)
        sprintf(msg, "%03d %s", err, txt);
    else
        sprintf(msg, "%s", txt);

    while(!sdl_poll())
    {
        b = SDL_GetMouseState(&mx, &my);
        if(!b)
            break;
    }

    while(!sdl_poll())
    {
        bq = b;
        b = SDL_GetMouseState(&mx, &my);
        mx /= sdl_scale;
        my /= sdl_scale;

        clearrect(vid_buf, x0-2, y0-2, 244, 64);
        drawrect(vid_buf, x0, y0, 240, 60, 192, 192, 192, 255);
        if(err)
            drawtext(vid_buf, x0+8, y0+8, "HTTP error:", 255, 64, 32, 255);
        else
            drawtext(vid_buf, x0+8, y0+8, "Error:", 255, 64, 32, 255);
        drawtext(vid_buf, x0+8, y0+26, msg, 255, 255, 255, 255);
        drawtext(vid_buf, x0+5, y0+49, "Dismiss", 255, 255, 255, 255);
        drawrect(vid_buf, x0, y0+44, 240, 16, 192, 192, 192, 255);
        sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

        if(b && !bq && mx>=x0 && mx<x0+240 && my>=y0+44 && my<=y0+60)
            break;

        if(sdl_key==SDLK_RETURN)
            break;
        if(sdl_key==SDLK_ESCAPE)
            break;
    }

    free(msg);

    while(!sdl_poll())
    {
        b = SDL_GetMouseState(&mx, &my);
        if(!b)
            break;
    }
}

void info_ui(pixel *vid_buf, char *top, char *txt)
{
    int x0=(XRES-240)/2,y0=(YRES-MENUSIZE)/2,b=1,bq,mx,my;

    while(!sdl_poll())
    {
        b = SDL_GetMouseState(&mx, &my);
        if(!b)
            break;
    }

    while(!sdl_poll())
    {
        bq = b;
        b = SDL_GetMouseState(&mx, &my);
        mx /= sdl_scale;
        my /= sdl_scale;

        clearrect(vid_buf, x0-2, y0-2, 244, 64);
        drawrect(vid_buf, x0, y0, 240, 60, 192, 192, 192, 255);
        drawtext(vid_buf, x0+8, y0+8, top, 160, 160, 255, 255);
        drawtext(vid_buf, x0+8, y0+26, txt, 255, 255, 255, 255);
        drawtext(vid_buf, x0+5, y0+49, "OK", 255, 255, 255, 255);
        drawrect(vid_buf, x0, y0+44, 240, 16, 192, 192, 192, 255);
        sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

        if(b && !bq && mx>=x0 && mx<x0+240 && my>=y0+44 && my<=y0+60)
            break;

        if(sdl_key==SDLK_RETURN)
            break;
        if(sdl_key==SDLK_ESCAPE)
            break;
    }

    while(!sdl_poll())
    {
        b = SDL_GetMouseState(&mx, &my);
        if(!b)
            break;
    }
}

void info_box(pixel *vid_buf, char *msg)
{
    int w = textwidth(msg)+16;
    int x0=(XRES-w)/2,y0=(YRES-24)/2;

    clearrect(vid_buf, x0-2, y0-2, w+4, 28);
    drawrect(vid_buf, x0, y0, w, 24, 192, 192, 192, 255);
    drawtext(vid_buf, x0+8, y0+8, msg, 192, 192, 240, 255);
    sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
}

int confirm_ui(pixel *vid_buf, char *top, char *msg, char *btn)
{
    int x0=(XRES-240)/2,y0=(YRES-MENUSIZE)/2,b=1,bq,mx,my;
    int ret = 0;

    while(!sdl_poll())
    {
        b = SDL_GetMouseState(&mx, &my);
        if(!b)
            break;
    }

    while(!sdl_poll())
    {
        bq = b;
        b = SDL_GetMouseState(&mx, &my);
        mx /= sdl_scale;
        my /= sdl_scale;

        clearrect(vid_buf, x0-2, y0-2, 244, 64);
        drawrect(vid_buf, x0, y0, 240, 60, 192, 192, 192, 255);
        drawtext(vid_buf, x0+8, y0+8, top, 255, 216, 32, 255);
        drawtext(vid_buf, x0+8, y0+26, msg, 255, 255, 255, 255);
        drawtext(vid_buf, x0+5, y0+49, "Cancel", 255, 255, 255, 255);
        drawtext(vid_buf, x0+165, y0+49, btn, 255, 216, 32, 255);
        drawrect(vid_buf, x0, y0+44, 160, 16, 192, 192, 192, 255);
        drawrect(vid_buf, x0+160, y0+44, 80, 16, 192, 192, 192, 255);
        sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

        if(b && !bq && mx>=x0+160 && mx<x0+240 && my>=y0+44 && my<=y0+60)
        {
            ret = 1;
            break;
        }
        if(b && !bq && mx>=x0 && mx<x0+160 && my>=y0+44 && my<=y0+60)
            break;

        if(sdl_key==SDLK_RETURN)
        {
            ret = 1;
            break;
        }
        if(sdl_key==SDLK_ESCAPE)
            break;
    }

    while(!sdl_poll())
    {
        b = SDL_GetMouseState(&mx, &my);
        if(!b)
            break;
    }

    return ret;
}

void login_ui(pixel *vid_buf)
{
    int x0=(XRES-192)/2,y0=(YRES-80)/2,b=1,bq,mx,my,err;
    ui_edit ed1,ed2;
    char *res;

    while(!sdl_poll())
    {
        b = SDL_GetMouseState(&mx, &my);
        if(!b)
            break;
    }

    ed1.x = x0+25;
    ed1.y = y0+25;
    ed1.w = 158;
    ed1.nx = 1;
    ed1.def = "[user name]";
    ed1.focus = 1;
    ed1.hide = 0;
    ed1.cursor = strlen(svf_user);
    strcpy(ed1.str, svf_user);
    ed2.x = x0+25;
    ed2.y = y0+45;
    ed2.w = 158;
    ed2.nx = 1;
    ed2.def = "[password]";
    ed2.focus = 0;
    ed2.hide = 1;
    ed2.cursor = 0;
    strcpy(ed2.str, "");

    fillrect(vid_buf, -1, -1, XRES, YRES+MENUSIZE, 0, 0, 0, 192);
    while(!sdl_poll())
    {
        bq = b;
        b = SDL_GetMouseState(&mx, &my);
        mx /= sdl_scale;
        my /= sdl_scale;

        drawrect(vid_buf, x0, y0, 192, 80, 192, 192, 192, 255);
        clearrect(vid_buf, x0, y0, 192, 80);
        drawtext(vid_buf, x0+8, y0+8, "Server login:", 255, 255, 255, 255);
        drawtext(vid_buf, x0+12, y0+23, "\x8B", 32, 64, 128, 255);
        drawtext(vid_buf, x0+12, y0+23, "\x8A", 255, 255, 255, 255);
        drawrect(vid_buf, x0+8, y0+20, 176, 16, 192, 192, 192, 255);
        drawtext(vid_buf, x0+11, y0+44, "\x8C", 160, 144, 32, 255);
        drawtext(vid_buf, x0+11, y0+44, "\x84", 255, 255, 255, 255);
        drawrect(vid_buf, x0+8, y0+40, 176, 16, 192, 192, 192, 255);
        ui_edit_draw(vid_buf, &ed1);
        ui_edit_draw(vid_buf, &ed2);
        drawtext(vid_buf, x0+5, y0+69, "Sign in", 255, 255, 255, 255);
        drawrect(vid_buf, x0, y0+64, 192, 16, 192, 192, 192, 255);
        sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

        ui_edit_process(mx, my, b, &ed1);
        ui_edit_process(mx, my, b, &ed2);

        if(b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+22 && my<y0+36)
            break;
        if(b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+42 && my<y0+46)
            break;
        if(b && !bq && mx>=x0 && mx<x0+192 && my>=y0+64 && my<=y0+80)
            break;

        if(sdl_key==SDLK_RETURN || sdl_key==SDLK_TAB)
        {
            if(!ed1.focus)
                break;
            ed1.focus = 0;
            ed2.focus = 1;
        }
        if(sdl_key==SDLK_ESCAPE)
        {
            if(!ed1.focus && !ed2.focus)
                return;
            ed1.focus = 0;
            ed2.focus = 0;
        }
    }

    strcpy(svf_user, ed1.str);
    md5_ascii(svf_pass, (unsigned char *)ed2.str, 0);

    res = http_multipart_post(
              "http://" SERVER "/Login.api",
              NULL, NULL, NULL,
              svf_user, svf_pass,
              &err, NULL);
    if(err != 200)
    {
        error_ui(vid_buf, err, http_ret_text(err));
        if(res)
            free(res);
        goto fail;
    }
    if(res && !strncmp(res, "OK", 2))
    {
        if(!strcmp(res, "OK ADMIN"))
        {
            svf_admin = 1;
            svf_mod = 0;
        }
        else if(!strcmp(res, "OK MOD"))
        {
            svf_admin = 0;
            svf_mod = 1;
        }
        else
        {
            svf_admin = 0;
            svf_mod = 0;
        }
        free(res);
        svf_login = 1;
        return;
    }
    if(!res)
        res = mystrdup("Unspecified Error");
    error_ui(vid_buf, 0, res);
    free(res);

fail:
    strcpy(svf_user, "");
    strcpy(svf_pass, "");
    svf_login = 0;
    svf_own = 0;
    svf_admin = 0;
    svf_mod = 0;
}

int stamp_ui(pixel *vid_buf)
{
    int b=1,bq,mx,my,d=-1,i,j,k,x,gx,gy,y,w,h,r=-1,stamp_page=0,per_page=STAMP_X*STAMP_Y,page_count;
    char page_info[64];
    page_count = ceil((float)stamp_count/(float)per_page);

    while(!sdl_poll())
    {
        b = SDL_GetMouseState(&mx, &my);
        if(!b)
            break;
    }

    while(!sdl_poll())
    {
        bq = b;
        b = SDL_GetMouseState(&mx, &my);
        mx /= sdl_scale;
        my /= sdl_scale;

        clearrect(vid_buf, -1, -1, XRES+1, YRES+MENUSIZE+1);
        k = stamp_page*per_page;//0;
        r = -1;
        d = -1;
        for(j=0; j<GRID_Y; j++)
            for(i=0; i<GRID_X; i++)
            {
                if(stamps[k].name[0] && stamps[k].thumb)
                {
                    gx = ((XRES/GRID_X)*i) + (XRES/GRID_X-XRES/GRID_S)/2;
                    gy = ((((YRES-MENUSIZE+20)+15)/GRID_Y)*j) + ((YRES-MENUSIZE+20)/GRID_Y-(YRES-MENUSIZE+20)/GRID_S+10)/2 + 18;
                    x = (XRES*i)/GRID_X + XRES/(GRID_X*2);
                    y = (YRES*j)/GRID_Y + YRES/(GRID_Y*2);
                    gy -= 20;
                    w = stamps[k].thumb_w;
                    h = stamps[k].thumb_h;
                    x -= w/2;
                    y -= h/2;
                    draw_image(vid_buf, stamps[k].thumb, gx+(((XRES/GRID_S)/2)-(w/2)), gy+(((YRES/GRID_S)/2)-(h/2)), w, h, 255);
                    xor_rect(vid_buf, gx+(((XRES/GRID_S)/2)-(w/2)), gy+(((YRES/GRID_S)/2)-(h/2)), w, h);
                    if(mx>=gx+XRES/GRID_S-4 && mx<(gx+XRES/GRID_S)+6 && my>=gy-6 && my<gy+4)
                    {
                        d = k;
                        drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 128, 255);
                        drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x86", 255, 48, 32, 255);
                    }
                    else
                    {
                        if(mx>=gx && mx<gx+(XRES/GRID_S) && my>=gy && my<gy+(YRES/GRID_S))
                        {
                            r = k;
                            drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 210, 255);
                        }
                        else
                        {
                            drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 128, 255);
                        }
                        drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x86", 150, 48, 32, 255);
                    }
                    drawtext(vid_buf, gx+XRES/(GRID_S*2)-textwidth(stamps[k].name)/2, gy+YRES/GRID_S+7, stamps[k].name, 192, 192, 192, 255);
                    drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x85", 255, 255, 255, 255);
                }
                k++;
            }

        sprintf(page_info, "Page %d of %d", stamp_page+1, page_count);

        drawtext(vid_buf, (XRES/2)-(textwidth(page_info)/2), YRES+MENUSIZE-14, page_info, 255, 255, 255, 255);

        if(stamp_page)
        {
            drawtext(vid_buf, 4, YRES+MENUSIZE-14, "\x96", 255, 255, 255, 255);
            drawrect(vid_buf, 1, YRES+MENUSIZE-18, 16, 16, 255, 255, 255, 255);
        }
        if(stamp_page<page_count-1)
        {
            drawtext(vid_buf, XRES-15, YRES+MENUSIZE-14, "\x95", 255, 255, 255, 255);
            drawrect(vid_buf, XRES-18, YRES+MENUSIZE-18, 16, 16, 255, 255, 255, 255);
        }

        if(b==1&&d!=-1)
        {
            if(confirm_ui(vid_buf, "Do you want to delete?", stamps[d].name, "Delete"))
            {
                del_stamp(d);
            }
        }

        sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

        if(b==1&&r!=-1)
            break;
        if(b==4&&r!=-1)
        {
            r = -1;
            break;
        }

        if((b && !bq && mx>=1 && mx<=17 && my>=YRES+MENUSIZE-18 && my<YRES+MENUSIZE-2) || sdl_wheel>0)
        {
            if(stamp_page)
            {
                stamp_page --;
            }
            sdl_wheel = 0;
        }
        if((b && !bq && mx>=XRES-18 && mx<=XRES-1 && my>=YRES+MENUSIZE-18 && my<YRES+MENUSIZE-2) || sdl_wheel<0)
        {
            if(stamp_page<page_count-1)
            {
                stamp_page ++;
            }
            sdl_wheel = 0;
        }

        if(sdl_key==SDLK_RETURN)
            break;
        if(sdl_key==SDLK_ESCAPE)
        {
            r = -1;
            break;
        }
    }

    while(!sdl_poll())
    {
        b = SDL_GetMouseState(&mx, &my);
        if(!b)
            break;
    }

    return r;
}

void tag_list_ui(pixel *vid_buf)
{
    int y,d,x0=(XRES-192)/2,y0=(YRES-256)/2,b=1,bq,mx,my,vp,vn;
    char *p,*q,s;
    char *tag=NULL, *op=NULL;
    ui_edit ed;
    struct strlist *vote=NULL,*down=NULL;

    ed.x = x0+25;
    ed.y = y0+221;
    ed.w = 158;
    ed.nx = 1;
    ed.def = "[new tag]";
    ed.focus = 0;
    ed.hide = 0;
    ed.cursor = 0;
    strcpy(ed.str, "");

    fillrect(vid_buf, -1, -1, XRES, YRES+MENUSIZE, 0, 0, 0, 192);
    while(!sdl_poll())
    {
        bq = b;
        b = SDL_GetMouseState(&mx, &my);
        mx /= sdl_scale;
        my /= sdl_scale;

        op = tag = NULL;

        drawrect(vid_buf, x0, y0, 192, 256, 192, 192, 192, 255);
        clearrect(vid_buf, x0, y0, 192, 256);
        drawtext(vid_buf, x0+8, y0+8, "Current tags:", 255, 255, 255, 255);
        p = svf_tags;
        s = svf_tags[0] ? ' ' : 0;
        y = 36 + y0;
        while(s)
        {
            q = strchr(p, ' ');
            if(!q)
                q = p+strlen(p);
            s = *q;
            *q = 0;
            if(svf_own || svf_admin || svf_mod)
            {
                drawtext(vid_buf, x0+20, y-1, "\x86", 160, 48, 32, 255);
                drawtext(vid_buf, x0+20, y-1, "\x85", 255, 255, 255, 255);
                d = 14;
                if(b && !bq && mx>=x0+18 && mx<x0+32 && my>=y-2 && my<y+12)
                {
                    op = "delete";
                    tag = mystrdup(p);
                }
            }
            else
                d = 0;
            vp = strlist_find(&vote, p);
            vn = strlist_find(&down, p);
            if((!vp && !vn && !svf_own) || svf_admin || svf_mod)
            {
                drawtext(vid_buf, x0+d+20, y-1, "\x88", 32, 144, 32, 255);
                drawtext(vid_buf, x0+d+20, y-1, "\x87", 255, 255, 255, 255);
                if(b && !bq && mx>=x0+d+18 && mx<x0+d+32 && my>=y-2 && my<y+12)
                {
                    op = "vote";
                    tag = mystrdup(p);
                    strlist_add(&vote, p);
                }
                drawtext(vid_buf, x0+d+34, y-1, "\x88", 144, 48, 32, 255);
                drawtext(vid_buf, x0+d+34, y-1, "\xA2", 255, 255, 255, 255);
                if(b && !bq && mx>=x0+d+32 && mx<x0+d+46 && my>=y-2 && my<y+12)
                {
                    op = "down";
                    tag = mystrdup(p);
                    strlist_add(&down, p);
                }
            }
            if(vp)
                drawtext(vid_buf, x0+d+48+textwidth(p), y, " - voted!", 48, 192, 48, 255);
            if(vn)
                drawtext(vid_buf, x0+d+48+textwidth(p), y, " - voted.", 192, 64, 32, 255);
            drawtext(vid_buf, x0+d+48, y, p, 192, 192, 192, 255);
            *q = s;
            p = q+1;
            y += 16;
        }
        drawtext(vid_buf, x0+11, y0+219, "\x86", 32, 144, 32, 255);
        drawtext(vid_buf, x0+11, y0+219, "\x89", 255, 255, 255, 255);
        drawrect(vid_buf, x0+8, y0+216, 176, 16, 192, 192, 192, 255);
        ui_edit_draw(vid_buf, &ed);
        drawtext(vid_buf, x0+5, y0+245, "Close", 255, 255, 255, 255);
        drawrect(vid_buf, x0, y0+240, 192, 16, 192, 192, 192, 255);
        sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

        ui_edit_process(mx, my, b, &ed);

        if(b && mx>=x0 && mx<=x0+192 && my>=y0+240 && my<y0+256)
            break;

        if(op)
        {
            d = execute_tagop(vid_buf, op, tag);
            free(tag);
            op = tag = NULL;
            if(d)
                goto finish;
        }

        if(b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+218 && my<y0+232)
        {
            d = execute_tagop(vid_buf, "add", ed.str);
            strcpy(ed.str, "");
            ed.cursor = 0;
            if(d)
                goto finish;
        }

        if(sdl_key==SDLK_RETURN)
        {
            if(!ed.focus)
                break;
            d = execute_tagop(vid_buf, "add", ed.str);
            strcpy(ed.str, "");
            ed.cursor = 0;
            if(d)
                goto finish;
        }
        if(sdl_key==SDLK_ESCAPE)
        {
            if(!ed.focus)
                break;
            strcpy(ed.str, "");
            ed.cursor = 0;
            ed.focus = 0;
        }
    }
    while(!sdl_poll())
    {
        b = SDL_GetMouseState(&mx, &my);
        if(!b)
            break;
    }
    sdl_key = 0;

finish:
    strlist_free(&vote);
}

int save_name_ui(pixel *vid_buf)
{
    int x0=(XRES-192)/2,y0=(YRES-68-YRES/4)/2,b=1,bq,mx,my,ths,nd=0;
    void *th;
    ui_edit ed;
    ui_checkbox cb;

    th = build_thumb(&ths, 0);

    while(!sdl_poll())
    {
        b = SDL_GetMouseState(&mx, &my);
        if(!b)
            break;
    }

    ed.x = x0+25;
    ed.y = y0+25;
    ed.w = 158;
    ed.nx = 1;
    ed.def = "[simulation name]";
    ed.focus = 1;
    ed.hide = 0;
    ed.cursor = strlen(svf_name);
    strcpy(ed.str, svf_name);

    cb.x = x0+10;
    cb.y = y0+53+YRES/4;
    cb.focus = 0;
    cb.checked = svf_publish;

    fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
    while(!sdl_poll())
    {
        bq = b;
        b = SDL_GetMouseState(&mx, &my);
        mx /= sdl_scale;
        my /= sdl_scale;

        drawrect(vid_buf, x0, y0, 192, 90+YRES/4, 192, 192, 192, 255);
        clearrect(vid_buf, x0, y0, 192, 90+YRES/4);
        drawtext(vid_buf, x0+8, y0+8, "New simulation name:", 255, 255, 255, 255);
        drawtext(vid_buf, x0+10, y0+23, "\x82", 192, 192, 192, 255);
        drawrect(vid_buf, x0+8, y0+20, 176, 16, 192, 192, 192, 255);

        ui_edit_draw(vid_buf, &ed);

        drawrect(vid_buf, x0+(192-XRES/4)/2-2, y0+42, XRES/4+3, YRES/4+3, 128, 128, 128, 255);
        render_thumb(th, ths, 0, vid_buf, x0+(192-XRES/4)/2, y0+44, 4);

        ui_checkbox_draw(vid_buf, &cb);
        drawtext(vid_buf, x0+34, y0+50+YRES/4, "Publish? (Do not publish others'\nworks without permission)", 192, 192, 192, 255);

        drawtext(vid_buf, x0+5, y0+79+YRES/4, "Save simulation", 255, 255, 255, 255);
        drawrect(vid_buf, x0, y0+74+YRES/4, 192, 16, 192, 192, 192, 255);

        sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

        ui_edit_process(mx, my, b, &ed);
        ui_checkbox_process(mx, my, b, bq, &cb);

        if(b && !bq && ((mx>=x0+9 && mx<x0+23 && my>=y0+22 && my<y0+36) ||
                        (mx>=x0 && mx<x0+192 && my>=y0+74+YRES/4 && my<y0+90+YRES/4)))
        {
            free(th);
            if(!ed.str[0])
                return 0;
            nd = strcmp(svf_name, ed.str) || !svf_own;
            strncpy(svf_name, ed.str, 63);
            svf_name[63] = 0;
            if(nd)
            {
                strcpy(svf_id, "");
                strcpy(svf_tags, "");
            }
            svf_open = 1;
            svf_own = 1;
            svf_publish = cb.checked;
            return nd+1;
        }

        if(sdl_key==SDLK_RETURN)
        {
            free(th);
            if(!ed.str[0])
                return 0;
            nd = strcmp(svf_name, ed.str) || !svf_own;
            strncpy(svf_name, ed.str, 63);
            svf_name[63] = 0;
            if(nd)
            {
                strcpy(svf_id, "");
                strcpy(svf_tags, "");
            }
            svf_open = 1;
            svf_own = 1;
            svf_publish = cb.checked;
            return nd+1;
        }
        if(sdl_key==SDLK_ESCAPE)
        {
            if(!ed.focus)
                break;
            ed.focus = 0;
        }
    }
    free(th);
    return 0;
}

void menu_ui(pixel *vid_buf, int i, int *sl, int *sr)
{
    int b=1,bq,mx,my,h,x,y,n=0,height,width,sy,rows=0;
    pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
    fillrect(vid_buf, -1, -1, XRES+1, YRES+MENUSIZE, 0, 0, 0, 192);
    memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

    while(!sdl_poll())
    {
        b = SDL_GetMouseState(&mx, &my);
        if(!b)
            break;
    }
    while(!sdl_poll())
    {
        bq = b;
        b = SDL_GetMouseState(&mx, &my);
        mx /= sdl_scale;
        my /= sdl_scale;
        rows = ceil((float)msections[i].itemcount/16.0f);
        height = (ceil((float)msections[i].itemcount/16.0f)*18);
        width = restrict_flt(msections[i].itemcount*31, 0, 16*31);
        //clearrect(vid_buf, -1, -1, XRES+1, YRES+MENUSIZE+1);
        h = -1;
        x = XRES-BARSIZE-26;
        y = (((YRES/SC_TOTAL)*i)+((YRES/SC_TOTAL)/2))-(height/2)+(FONT_H/2)+1;
        sy = y;
        //clearrect(vid_buf, (XRES-BARSIZE-width)+1, y-4, width+4, height+4+rows);
        fillrect(vid_buf, (XRES-BARSIZE-width)-7, y-10, width+16, height+16+rows, 0, 0, 0, 100);
        drawrect(vid_buf, (XRES-BARSIZE-width)-7, y-10, width+16, height+16+rows, 255, 255, 255, 255);
        fillrect(vid_buf, (XRES-BARSIZE)+11, (((YRES/SC_TOTAL)*i)+((YRES/SC_TOTAL)/2))-2, 15, FONT_H+3, 0, 0, 0, 100);
        drawrect(vid_buf, (XRES-BARSIZE)+10, (((YRES/SC_TOTAL)*i)+((YRES/SC_TOTAL)/2))-2, 16, FONT_H+3, 255, 255, 255, 255);
        drawrect(vid_buf, (XRES-BARSIZE)+9, (((YRES/SC_TOTAL)*i)+((YRES/SC_TOTAL)/2))-1, 1, FONT_H+1, 0, 0, 0, 255);
        if(i==SC_WALL)
        {
            for(n = 122; n<122+UI_WALLCOUNT; n++)
            {
                if(n!=SPC_AIR&&n!=SPC_HEAT&&n!=SPC_COOL&&n!=SPC_VACUUM)
                {
                    if(x-26<=60)
                    {
                        x = XRES-BARSIZE-26;
                        y += 19;
                    }
                    x -= draw_tool_xy(vid_buf, x, y, n, mwalls[n-122].colour)+5;
                    if(mx>=x+32 && mx<x+58 && my>=y && my< y+15)
                    {
                        drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                        h = n;
                    }
                    else if(n==*sl)
                    {
                        drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                    }
                    else if(n==*sr)
                    {
                        drawrect(vid_buf, x+30, y-1, 29, 17, 0, 0, 255, 255);
                    }
                }
            }
        }
        else if(i==SC_SPECIAL)
        {
            for(n = 122; n<122+UI_WALLCOUNT; n++)
            {
                if(n==SPC_AIR||n==SPC_HEAT||n==SPC_COOL||n==SPC_VACUUM)
                {
                    if(x-26<=60)
                    {
                        x = XRES-BARSIZE-26;
                        y += 19;
                    }
                    x -= draw_tool_xy(vid_buf, x, y, n, mwalls[n-122].colour)+5;
                    if(mx>=x+32 && mx<x+58 && my>=y && my< y+15)
                    {
                        drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                        h = n;
                    }
                    else if(n==*sl)
                    {
                        drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                    }
                    else if(n==*sr)
                    {
                        drawrect(vid_buf, x+30, y-1, 29, 17, 0, 0, 255, 255);
                    }
                }
            }
            for(n = 0; n<PT_NUM; n++)
            {
                if(ptypes[n].menusection==i&&ptypes[n].menu==1)
                {
                    if(x-26<=60)
                    {
                        x = XRES-BARSIZE-26;
                        y += 19;
                    }
                    x -= draw_tool_xy(vid_buf, x, y, n, ptypes[n].pcolors)+5;
                    if(mx>=x+32 && mx<x+58 && my>=y && my< y+15)
                    {
                        drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                        h = n;
                    }
                    else if(n==*sl)
                    {
                        drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                    }
                    else if(n==*sr)
                    {
                        drawrect(vid_buf, x+30, y-1, 29, 17, 0, 0, 255, 255);
                    }
                }
            }
        }
        else
        {
            for(n = 0; n<PT_NUM; n++)
            {
                if(ptypes[n].menusection==i&&ptypes[n].menu==1)
                {
                    if(x-26<=60)
                    {
                        x = XRES-BARSIZE-26;
                        y += 19;
                    }
                    x -= draw_tool_xy(vid_buf, x, y, n, ptypes[n].pcolors)+5;
                    if(mx>=x+32 && mx<x+58 && my>=y && my< y+15)
                    {
                        drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                        h = n;
                    }
                    else if(n==*sl)
                    {
                        drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                    }
                    else if(n==*sr)
                    {
                        drawrect(vid_buf, x+30, y-1, 29, 17, 0, 0, 255, 255);
                    }
                }
            }
        }

        if(h==-1)
        {
            drawtext(vid_buf, XRES-textwidth((char *)msections[i].name)-BARSIZE, sy+height+10, (char *)msections[i].name, 255, 255, 255, 255);
        }
        else if(i==SC_WALL||(i==SC_SPECIAL&&h>=122))
        {
            drawtext(vid_buf, XRES-textwidth((char *)mwalls[h-122].descs)-BARSIZE, sy+height+10, (char *)mwalls[h-122].descs, 255, 255, 255, 255);
        }
        else
        {
            drawtext(vid_buf, XRES-textwidth((char *)ptypes[h].descs)-BARSIZE, sy+height+10, (char *)ptypes[h].descs, 255, 255, 255, 255);
        }


        sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
        memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
        if(!(mx>=(XRES-BARSIZE-width)-7 && my>=sy-10 && my<sy+height+9))
        {
            break;
        }

        if(b==1&&h!=-1)
        {
            *sl = h;
            break;
        }
        if(b==4&&h!=-1)
        {
            *sr = h;
            break;
        }
        //if(b==4&&h!=-1) {
        //	h = -1;
        //	break;
        //}

        if(sdl_key==SDLK_RETURN)
            break;
        if(sdl_key==SDLK_ESCAPE)
            break;
    }

    while(!sdl_poll())
    {
        b = SDL_GetMouseState(&mx, &my);
        if(!b)
            break;
    }
    //drawtext(vid_buf, XRES+2, (12*i)+2, msections[i].icon, 255, 255, 255, 255);
}

void menu_ui_v3(pixel *vid_buf, int i, int *sl, int *sr, int b, int bq, int mx, int my)
{
    int h,x,y,n=0,height,width,sy,rows=0;
    mx /= sdl_scale;
    my /= sdl_scale;
    rows = ceil((float)msections[i].itemcount/16.0f);
    height = (ceil((float)msections[i].itemcount/16.0f)*18);
    width = restrict_flt(msections[i].itemcount*31, 0, 16*31);
    h = -1;
    x = XRES-BARSIZE-26;
    y = YRES+1;
    sy = y;
    if(i==SC_WALL)
    {
        for(n = 122; n<122+UI_WALLCOUNT; n++)
        {
            if(n!=SPC_AIR&&n!=SPC_HEAT&&n!=SPC_COOL&&n!=SPC_VACUUM)
            {
                if(x-26<=20)
                {
                    x = XRES-BARSIZE-26;
                    y += 19;
                }
                x -= draw_tool_xy(vid_buf, x, y, n, mwalls[n-122].colour)+5;
                if(!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15)
                {
                    drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                    h = n;
                }
                else if(n==*sl)
                {
                    drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                }
                else if(n==*sr)
                {
                    drawrect(vid_buf, x+30, y-1, 29, 17, 0, 0, 255, 255);
                }
            }
        }
    }
    else if(i==SC_SPECIAL)
    {
        for(n = 122; n<122+UI_WALLCOUNT; n++)
        {
            if(n==SPC_AIR||n==SPC_HEAT||n==SPC_COOL||n==SPC_VACUUM)
            {
                if(x-26<=20)
                {
                    x = XRES-BARSIZE-26;
                    y += 19;
                }
                x -= draw_tool_xy(vid_buf, x, y, n, mwalls[n-122].colour)+5;
                if(!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15)
                {
                    drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                    h = n;
                }
                else if(n==*sl)
                {
                    drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                }
                else if(n==*sr)
                {
                    drawrect(vid_buf, x+30, y-1, 29, 17, 0, 0, 255, 255);
                }
            }
        }
        for(n = 0; n<PT_NUM; n++)
        {
            if(ptypes[n].menusection==i&&ptypes[n].menu==1)
            {
                if(x-26<=20)
                {
                    x = XRES-BARSIZE-26;
                    y += 19;
                }
                x -= draw_tool_xy(vid_buf, x, y, n, ptypes[n].pcolors)+5;
                if(!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15)
                {
                    drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                    h = n;
                }
                else if(n==*sl)
                {
                    drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                }
                else if(n==*sr)
                {
                    drawrect(vid_buf, x+30, y-1, 29, 17, 0, 0, 255, 255);
                }
            }
        }
    }
    else
    {
        for(n = 0; n<PT_NUM; n++)
        {
            if(ptypes[n].menusection==i&&ptypes[n].menu==1)
            {
                if(x-26<=20)
                {
                    x = XRES-BARSIZE-26;
                    y += 19;
                }
                x -= draw_tool_xy(vid_buf, x, y, n, ptypes[n].pcolors)+5;
                if(!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15)
                {
                    drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                    h = n;
                }
                else if(n==*sl)
                {
                    drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
                }
                else if(n==*sr)
                {
                    drawrect(vid_buf, x+30, y-1, 29, 17, 0, 0, 255, 255);
                }
            }
        }
    }

    if(h==-1)
    {
        drawtext(vid_buf, XRES-textwidth((char *)msections[i].name)-BARSIZE, sy-10, (char *)msections[i].name, 255, 255, 255, 255);
    }
    else if(i==SC_WALL||(i==SC_SPECIAL&&h>=122))
    {
        drawtext(vid_buf, XRES-textwidth((char *)mwalls[h-122].descs)-BARSIZE, sy-10, (char *)mwalls[h-122].descs, 255, 255, 255, 255);
    }
    else
    {
        drawtext(vid_buf, XRES-textwidth((char *)ptypes[h].descs)-BARSIZE, sy-10, (char *)ptypes[h].descs, 255, 255, 255, 255);
    }

    if(b==1&&h!=-1)
    {
        *sl = h;
    }
    if(b==4&&h!=-1)
    {
        *sr = h;
    }
}

int sdl_poll(void)
{
    SDL_Event event;
    sdl_key=sdl_wheel=sdl_ascii=0;
    while(SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_KEYDOWN:
            sdl_key=event.key.keysym.sym;
            sdl_ascii=event.key.keysym.unicode;
            if(event.key.keysym.sym == SDLK_CAPSLOCK)
                sdl_caps = 1;
            if(event.key.keysym.unicode=='z' || event.key.keysym.unicode=='Z')
            {
                sdl_zoom_trig = 1;
                Z_keysym = event.key.keysym.sym;
            }
            if( event.key.keysym.sym == SDLK_PLUS)
            {
                sdl_wheel++;
            }
            if( event.key.keysym.sym == SDLK_MINUS)
            {
                sdl_wheel--;
            }
            //  4
            //1 8 2
            if(event.key.keysym.sym == SDLK_RIGHT)
            {
                player[0] = (int)(player[0])|0x02;  //Go right command
            }
            if(event.key.keysym.sym == SDLK_LEFT)
            {
                player[0] = (int)(player[0])|0x01;  //Go left command
            }
            if(event.key.keysym.sym == SDLK_DOWN && ((int)(player[0])&0x08)!=0x08)
            {
                player[0] = (int)(player[0])|0x08;  //Go left command
            }
            if(event.key.keysym.sym == SDLK_UP && ((int)(player[0])&0x04)!=0x04)
            {
                player[0] = (int)(player[0])|0x04;  //Jump command
            }
            break;

        case SDL_KEYUP:
            if(event.key.keysym.sym == SDLK_CAPSLOCK)
                sdl_caps = 0;
            if(event.key.keysym.sym == Z_keysym)
                sdl_zoom_trig = 0;
            if(event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_LEFT)
            {
                player[1] = player[0];  //Saving last movement
                player[0] = (int)(player[0])&12;  //Stop command
            }
            if(event.key.keysym.sym == SDLK_UP)
            {
                player[0] = (int)(player[0])&11;
            }
            if(event.key.keysym.sym == SDLK_DOWN)
            {
                player[0] = (int)(player[0])&7;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if(event.button.button == SDL_BUTTON_WHEELUP)
                sdl_wheel++;
            if(event.button.button == SDL_BUTTON_WHEELDOWN)
                sdl_wheel--;
            break;
        case SDL_QUIT:
            return 1;
        }
    }
    sdl_mod = SDL_GetModState();
    return 0;
}

void set_cmode(int cm)
{
    cmode = cm;
    itc = 51;
    if(cmode==4)
    {
        memset(fire_r, 0, sizeof(fire_r));
        memset(fire_g, 0, sizeof(fire_g));
        memset(fire_b, 0, sizeof(fire_b));
        strcpy(itc_msg, "Blob Display");
    }
    else if(cmode==5)
    {
        strcpy(itc_msg, "Heat Display");
    }
    else if(cmode==6)
    {
        memset(fire_r, 0, sizeof(fire_r));
        memset(fire_g, 0, sizeof(fire_g));
        memset(fire_b, 0, sizeof(fire_b));
        strcpy(itc_msg, "Fancy Display");
    }
    else if(cmode==3)
    {
        memset(fire_r, 0, sizeof(fire_r));
        memset(fire_g, 0, sizeof(fire_g));
        memset(fire_b, 0, sizeof(fire_b));
        strcpy(itc_msg, "Fire Display");
    }
    else if(cmode==2)
    {
        memset(fire_bg, 0, XRES*YRES*PIXELSIZE);
        strcpy(itc_msg, "Persistent Display");
    }
    else if(cmode==1)
        strcpy(itc_msg, "Pressure Display");
    else
        strcpy(itc_msg, "Velocity Display");
}

char *download_ui(pixel *vid_buf, char *uri, int *len)
{
    int dstate = 0;
    void *http = http_async_req_start(NULL, uri, NULL, 0, 0);
    int x0=(XRES-240)/2,y0=(YRES-MENUSIZE)/2;
    int done, total, i, ret, zlen, ulen;
    char str[16], *tmp, *res;

    while(!http_async_req_status(http))
    {
        sdl_poll();

        http_async_get_length(http, &total, &done);

        clearrect(vid_buf, x0-2, y0-2, 244, 64);
        drawrect(vid_buf, x0, y0, 240, 60, 192, 192, 192, 255);
        drawtext(vid_buf, x0+8, y0+8, "Please wait", 255, 216, 32, 255);
        drawtext(vid_buf, x0+8, y0+26, "Downloading update...", 255, 255, 255, 255);

        if(total)
        {
            i = (236*done)/total;
            fillrect(vid_buf, x0+1, y0+45, i+1, 14, 255, 216, 32, 255);
            i = (100*done)/total;
            sprintf(str, "%d%%", i);
            if(i<50)
                drawtext(vid_buf, x0+120-textwidth(str)/2, y0+48, str, 192, 192, 192, 255);
            else
                drawtext(vid_buf, x0+120-textwidth(str)/2, y0+48, str, 0, 0, 0, 255);
        }
        else
            drawtext(vid_buf, x0+120-textwidth("Waiting...")/2, y0+48, "Waiting...", 255, 216, 32, 255);

        drawrect(vid_buf, x0, y0+44, 240, 16, 192, 192, 192, 255);
        sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
    }

    tmp = http_async_req_stop(http, &ret, &zlen);
    if(ret!=200)
    {
        error_ui(vid_buf, ret, http_ret_text(ret));
        if(tmp)
            free(tmp);
        return NULL;
    }
    if(!tmp)
    {
        error_ui(vid_buf, 0, "Server did not return data");
        return NULL;
    }

    if(zlen<16)
    {
        printf("ZLen is not 16!\n");
        goto corrupt;
    }
    if(tmp[0]!=0x42 || tmp[1]!=0x75 || tmp[2]!=0x54 || tmp[3]!=0x54)
    {
        printf("Tmperr %d, %d, %d, %d\n", tmp[0], tmp[1], tmp[2], tmp[3]);
        goto corrupt;
    }

    ulen  = (unsigned char)tmp[4];
    ulen |= ((unsigned char)tmp[5])<<8;
    ulen |= ((unsigned char)tmp[6])<<16;
    ulen |= ((unsigned char)tmp[7])<<24;

    res = (char *)malloc(ulen);
    if(!res)
    {
        printf("No res!\n");
        goto corrupt;
    }
    dstate = BZ2_bzBuffToBuffDecompress((char *)res, (unsigned *)&ulen, (char *)(tmp+8), zlen-8, 0, 0);
    if(dstate)
    {
        printf("Decompression failure: %d!\n", dstate);
        free(res);
        goto corrupt;
    }

    free(tmp);
    if(len)
        *len = ulen;
    return res;

corrupt:
    error_ui(vid_buf, 0, "Downloaded update is corrupted");
    free(tmp);
    return NULL;
}

int search_ui(pixel *vid_buf)
{
    int uih=0,nyu,nyd,b=1,bq,mx=0,my=0,mxq=0,myq=0,mmt=0,gi,gj,gx,gy,pos,i,mp,dp,dap,own,last_own=search_own,page_count=0,last_page=0,last_date=0,j,w,h,st=0,lv;
    int is_p1=0, exp_res=GRID_X*GRID_Y, tp, view_own=0;
    int thumb_drawn[GRID_X*GRID_Y];
    pixel *v_buf = (pixel *)malloc(((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);
    float ry;
    time_t http_last_use=HTTP_TIMEOUT;
    ui_edit ed;


    void *http = NULL;
    int active = 0;
    char *last = NULL;
    int search = 0;
    int lasttime = TIMEOUT;
    char *uri;
    int status;
    char *results;
    char *tmp, ts[64];

    void *img_http[IMGCONNS];
    char *img_id[IMGCONNS];
    void *thumb, *data;
    int thlen, dlen;

    memset(v_buf, 0, ((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);

    memset(img_http, 0, sizeof(img_http));
    memset(img_id, 0, sizeof(img_id));

    memset(search_ids, 0, sizeof(search_ids));
    memset(search_dates, 0, sizeof(search_dates));
    memset(search_names, 0, sizeof(search_names));
    memset(search_scoreup, 0, sizeof(search_scoreup));
    memset(search_scoredown, 0, sizeof(search_scoredown));
    memset(search_publish, 0, sizeof(search_publish));
    memset(search_owners, 0, sizeof(search_owners));
    memset(search_thumbs, 0, sizeof(search_thumbs));
    memset(search_thsizes, 0, sizeof(search_thsizes));

    memset(thumb_drawn, 0, sizeof(thumb_drawn));

    do_open = 0;

    while(!sdl_poll())
    {
        b = SDL_GetMouseState(&mx, &my);
        if(!b)
            break;
    }

    ed.x = 65;
    ed.y = 13;
    ed.w = XRES-200;
    ed.nx = 1;
    ed.def = "[search terms]";
    ed.focus = 1;
    ed.hide = 0;
    ed.cursor = strlen(search_expr);
    strcpy(ed.str, search_expr);

    sdl_wheel = 0;

    while(!sdl_poll())
    {
        uih = 0;
        bq = b;
        mxq = mx;
        myq = my;
        b = SDL_GetMouseState(&mx, &my);
        mx /= sdl_scale;
        my /= sdl_scale;

        if(mx!=mxq || my!=myq || sdl_wheel || b)
            mmt = 0;
        else if(mmt<TIMEOUT)
            mmt++;

        clearrect(vid_buf, -1, -1, (XRES+BARSIZE)+1, YRES+MENUSIZE+1);

        memcpy(vid_buf, v_buf, ((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);

        drawtext(vid_buf, 11, 13, "Search:", 192, 192, 192, 255);
        if(!last || (!active && strcmp(last, ed.str)))
            drawtext(vid_buf, 51, 11, "\x8E", 192, 160, 32, 255);
        else
            drawtext(vid_buf, 51, 11, "\x8E", 32, 64, 160, 255);
        drawtext(vid_buf, 51, 11, "\x8F", 255, 255, 255, 255);
        drawrect(vid_buf, 48, 8, XRES-182, 16, 192, 192, 192, 255);

        if(!svf_login)
        {
            search_own = 0;
            drawrect(vid_buf, XRES-64, 8, 56, 16, 96, 96, 96, 255);
            drawtext(vid_buf, XRES-61, 11, "\x94", 96, 80, 16, 255);
            drawtext(vid_buf, XRES-61, 11, "\x93", 128, 128, 128, 255);
            drawtext(vid_buf, XRES-46, 13, "My Own", 128, 128, 128, 255);
        }
        else if(search_own)
        {
            fillrect(vid_buf, XRES-65, 7, 58, 18, 255, 255, 255, 255);
            drawtext(vid_buf, XRES-61, 11, "\x94", 192, 160, 64, 255);
            drawtext(vid_buf, XRES-61, 11, "\x93", 32, 32, 32, 255);
            drawtext(vid_buf, XRES-46, 13, "My Own", 0, 0, 0, 255);
        }
        else
        {
            drawrect(vid_buf, XRES-64, 8, 56, 16, 192, 192, 192, 255);
            drawtext(vid_buf, XRES-61, 11, "\x94", 192, 160, 32, 255);
            drawtext(vid_buf, XRES-61, 11, "\x93", 255, 255, 255, 255);
            drawtext(vid_buf, XRES-46, 13, "My Own", 255, 255, 255, 255);
        }

        if(search_date)
        {
            fillrect(vid_buf, XRES-130, 7, 62, 18, 255, 255, 255, 255);
            drawtext(vid_buf, XRES-126, 11, "\xA6", 32, 32, 32, 255);
            drawtext(vid_buf, XRES-111, 13, "By date", 0, 0, 0, 255);
        }
        else
        {
            drawrect(vid_buf, XRES-129, 8, 60, 16, 192, 192, 192, 255);
            drawtext(vid_buf, XRES-126, 11, "\xA9", 144, 48, 32, 255);
            drawtext(vid_buf, XRES-126, 11, "\xA8", 32, 144, 32, 255);
            drawtext(vid_buf, XRES-126, 11, "\xA7", 255, 255, 255, 255);
            drawtext(vid_buf, XRES-111, 13, "By votes", 255, 255, 255, 255);
        }

        if(search_page)
        {
            drawtext(vid_buf, 4, YRES+MENUSIZE-16, "\x96", 255, 255, 255, 255);
            drawrect(vid_buf, 1, YRES+MENUSIZE-20, 16, 16, 255, 255, 255, 255);
        }
        if(page_count > 9)
        {
            drawtext(vid_buf, XRES-15, YRES+MENUSIZE-16, "\x95", 255, 255, 255, 255);
            drawrect(vid_buf, XRES-18, YRES+MENUSIZE-20, 16, 16, 255, 255, 255, 255);
        }

        ui_edit_draw(vid_buf, &ed);

        if((b && !bq && mx>=1 && mx<=17 && my>=YRES+MENUSIZE-20 && my<YRES+MENUSIZE-4) || sdl_wheel>0)
        {
            if(search_page)
            {
                search_page --;
                lasttime = TIMEOUT;
            }
            sdl_wheel = 0;
            uih = 1;
        }
        if((b && !bq && mx>=XRES-18 && mx<=XRES-1 && my>=YRES+MENUSIZE-20 && my<YRES+MENUSIZE-4) || sdl_wheel<0)
        {
            if(page_count>exp_res)
            {
                lasttime = TIMEOUT;
                search_page ++;
                page_count = exp_res;
            }
            sdl_wheel = 0;
            uih = 1;
        }

        tp = -1;
        if(is_p1)
        {
            drawtext(vid_buf, (XRES-textwidth("Popular tags:"))/2, 31, "Popular tags:", 255, 192, 64, 255);
            for(gj=0; gj<((GRID_Y-GRID_P)*YRES)/(GRID_Y*14); gj++)
                for(gi=0; gi<GRID_X; gi++)
                {
                    pos = gi+GRID_X*gj;
                    if(pos>TAG_MAX || !tag_names[pos])
                        break;
                    if(tag_votes[0])
                        i = 127+(128*tag_votes[pos])/tag_votes[0];
                    else
                        i = 192;
                    w = textwidth(tag_names[pos]);
                    if(w>XRES/GRID_X-5)
                        w = XRES/GRID_X-5;
                    gx = (XRES/GRID_X)*gi;
                    gy = gj*14 + 46;
                    if(mx>=gx && mx<gx+(XRES/GRID_X) && my>=gy && my<gy+14)
                    {
                        j = (i*5)/6;
                        tp = pos;
                    }
                    else
                        j = i;
                    drawtextmax(vid_buf, gx+(XRES/GRID_X-w)/2, gy, XRES/GRID_X-5, tag_names[pos], j, j, i, 255);
                }
        }

        mp = dp = -1;
        dap = -1;
        st = 0;
        for(gj=0; gj<GRID_Y; gj++)
            for(gi=0; gi<GRID_X; gi++)
            {
                if(is_p1)
                {
                    pos = gi+GRID_X*(gj-GRID_Y+GRID_P);
                    if(pos<0)
                        break;
                }
                else
                    pos = gi+GRID_X*gj;
                if(!search_ids[pos])
                    break;
                gx = ((XRES/GRID_X)*gi) + (XRES/GRID_X-XRES/GRID_S)/2;
                gy = ((((YRES-(MENUSIZE-20))+15)/GRID_Y)*gj) + ((YRES-(MENUSIZE-20))/GRID_Y-(YRES-(MENUSIZE-20))/GRID_S+10)/2 + 18;
                if(textwidth(search_names[pos]) > XRES/GRID_X-10)
                {
                    tmp = malloc(strlen(search_names[pos])+4);
                    strcpy(tmp, search_names[pos]);
                    j = textwidthx(tmp, XRES/GRID_X-15);
                    strcpy(tmp+j, "...");
                    drawtext(vid_buf, gx+XRES/(GRID_S*2)-textwidth(tmp)/2, gy+YRES/GRID_S+7, tmp, 192, 192, 192, 255);
                    free(tmp);
                }
                else
                    drawtext(vid_buf, gx+XRES/(GRID_S*2)-textwidth(search_names[pos])/2, gy+YRES/GRID_S+7, search_names[pos], 192, 192, 192, 255);
                j = textwidth(search_owners[pos]);
                if(mx>=gx+XRES/(GRID_S*2)-j/2 && mx<=gx+XRES/(GRID_S*2)+j/2 &&
                        my>=gy+YRES/GRID_S+18 && my<=gy+YRES/GRID_S+31)
                {
                    st = 1;
                    drawtext(vid_buf, gx+XRES/(GRID_S*2)-j/2, gy+YRES/GRID_S+20, search_owners[pos], 128, 128, 160, 255);
                }
                else
                    drawtext(vid_buf, gx+XRES/(GRID_S*2)-j/2, gy+YRES/GRID_S+20, search_owners[pos], 128, 128, 128, 255);
                if(search_thumbs[pos]&&thumb_drawn[pos]==0)
                {
                    render_thumb(search_thumbs[pos], search_thsizes[pos], 1, v_buf, gx, gy, GRID_S);
                    thumb_drawn[pos] = 1;
                }
                own = svf_login && (!strcmp(svf_user, search_owners[pos]) || svf_admin || svf_mod);
                if(mx>=gx-2 && mx<=gx+XRES/GRID_S+3 && my>=gy-2 && my<=gy+YRES/GRID_S+30)
                    mp = pos;
                if(own)
                {
                    if(mx>=gx+XRES/GRID_S-4 && mx<=gx+XRES/GRID_S+6 && my>=gy-6 && my<=gy+4)
                    {
                        mp = -1;
                        dp = pos;
                    }
                    if(!search_dates[pos] && mx>=gx-6 && mx<=gx+4 && my>=gy+YRES/GRID_S-4 && my<=gy+YRES/GRID_S+6)
                    {
                        mp = -1;
                        dap = pos;
                    }
                }
                drawrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2, 6, YRES/GRID_S+3, 128, 128, 128, 255);
                fillrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2, 6, 1+(YRES/GRID_S+3)/2, 0, 107, 10, 255);
                fillrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2), 6, 1+(YRES/GRID_S+3)/2, 107, 10, 0, 255);

                if(mp==pos && !st)
                    drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 160, 160, 192, 255);
                else
                    drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 128, 255);
                if(own)
                {
                    if(dp == pos)
                        drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x86", 255, 48, 32, 255);
                    else
                        drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x86", 160, 48, 32, 255);
                    drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x85", 255, 255, 255, 255);
                }
                if(!search_publish[pos])
                {
                    drawtext(vid_buf, gx-6, gy-6, "\xCD", 255, 255, 255, 255);
                    drawtext(vid_buf, gx-6, gy-6, "\xCE", 212, 151, 81, 255);
                }
                if(!search_dates[pos] && own)
                {
                    fillrect(vid_buf, gx-5, gy+YRES/GRID_S-3, 7, 8, 255, 255, 255, 255);
                    if(dap == pos) {
                        drawtext(vid_buf, gx-6, gy+YRES/GRID_S-4, "\xA6", 200, 100, 80, 255);
                    } else {
                        drawtext(vid_buf, gx-6, gy+YRES/GRID_S-4, "\xA6", 160, 70, 50, 255);
                    }
                    //drawtext(vid_buf, gx-6, gy-6, "\xCE", 212, 151, 81, 255);
                }
                if(view_own || svf_admin || svf_mod)
                {
                    sprintf(ts+1, "%d", search_votes[pos]);
                    ts[0] = 0xBB;
                    for(j=1; ts[j]; j++)
                        ts[j] = 0xBC;
                    ts[j-1] = 0xB9;
                    ts[j] = 0xBA;
                    ts[j+1] = 0;
                    w = gx+XRES/GRID_S-2-textwidth(ts);
                    h = gy+YRES/GRID_S-11;
                    drawtext(vid_buf, w, h, ts, 16, 72, 16, 255);
                    for(j=0; ts[j]; j++)
                        ts[j] -= 14;
                    drawtext(vid_buf, w, h, ts, 192, 192, 192, 255);
                    sprintf(ts, "%d", search_votes[pos]);
                    for(j=0; ts[j]; j++)
                        ts[j] += 127;
                    drawtext(vid_buf, w+3, h, ts, 255, 255, 255, 255);
                }
                if(search_scoreup[pos]>0||search_scoredown[pos]>0)
                {
                    lv = (search_scoreup[pos]>search_scoredown[pos]?search_scoreup[pos]:search_scoredown[pos]);

                    if(((YRES/GRID_S+3)/2)>lv)
                    {
                        ry = ((float)((YRES/GRID_S+3)/2)/(float)lv);
                        if(lv<8)
                        {
                            ry =  ry/(8-lv);
                        }
                        nyu = search_scoreup[pos]*ry;
                        nyd = search_scoredown[pos]*ry;
                    }
                    else
                    {
                        ry = ((float)lv/(float)((YRES/GRID_S+3)/2));
                        nyu = search_scoreup[pos]/ry;
                        nyd = search_scoredown[pos]/ry;
                    }


                    fillrect(vid_buf, gx-1+(XRES/GRID_S)+5, gy-1+((YRES/GRID_S+3)/2)-nyu, 4, nyu, 57, 187, 57, 255);
                    fillrect(vid_buf, gx-1+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2), 4, nyd, 187, 57, 57, 255);
                    //drawrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2)-nyu, 4, nyu, 0, 107, 10, 255);
                    //drawrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2)+1, 4, nyd, 107, 10, 0, 255);
                }
            }

        if(mp!=-1 && mmt>=TIMEOUT/5 && !st)
        {
            gi = mp % GRID_X;
            gj = mp / GRID_X;
            if(is_p1)
                gj += GRID_Y-GRID_P;
            gx = ((XRES/GRID_X)*gi) + (XRES/GRID_X-XRES/GRID_S)/2;
            gy = (((YRES+15)/GRID_Y)*gj) + (YRES/GRID_Y-YRES/GRID_S+10)/2 + 18;
            i = w = textwidth(search_names[mp]);
            h = YRES/GRID_Z+30;
            if(w<XRES/GRID_Z) w=XRES/GRID_Z;
            gx += XRES/(GRID_S*2)-w/2;
            gy += YRES/(GRID_S*2)-h/2;
            if(gx<2) gx=2;
            if(gx+w>=XRES-2) gx=XRES-3-w;
            if(gy<32) gy=32;
            if(gy+h>=YRES+(MENUSIZE-2)) gy=YRES+(MENUSIZE-3)-h;
            clearrect(vid_buf, gx-2, gy-3, w+4, h);
            drawrect(vid_buf, gx-2, gy-3, w+4, h, 160, 160, 192, 255);
            if(search_thumbs[mp])
                render_thumb(search_thumbs[mp], search_thsizes[mp], 1, vid_buf, gx+(w-(XRES/GRID_Z))/2, gy, GRID_Z);
            drawtext(vid_buf, gx+(w-i)/2, gy+YRES/GRID_Z+4, search_names[mp], 192, 192, 192, 255);
            drawtext(vid_buf, gx+(w-textwidth(search_owners[mp]))/2, gy+YRES/GRID_Z+16, search_owners[mp], 128, 128, 128, 255);
        }

        sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

        ui_edit_process(mx, my, b, &ed);

        if(sdl_key==SDLK_RETURN)
        {
            if(!last || (!active && (strcmp(last, ed.str) || last_own!=search_own || last_date!=search_date || last_page!=search_page)))
                lasttime = TIMEOUT;
            else if(search_ids[0] && !search_ids[1])
            {
                bq = 0;
                b = 1;
                mp = 0;
            }
        }
        if(sdl_key==SDLK_ESCAPE)
            goto finish;

        if(b && !bq && mx>=XRES-64 && mx<=XRES-8 && my>=8 && my<=24 && svf_login)
        {
            search_own = !search_own;
            lasttime = TIMEOUT;
        }
        if(b && !bq && mx>=XRES-129 && mx<=XRES-65 && my>=8 && my<=24)
        {
            search_date = !search_date;
            lasttime = TIMEOUT;
        }

        if(b && !bq && dp!=-1)
            if(confirm_ui(vid_buf, "Do you want to delete?", search_names[dp], "Delete"))
            {
                execute_delete(vid_buf, search_ids[dp]);
                lasttime = TIMEOUT;
                if(last)
                {
                    free(last);
                    last = NULL;
                }
            }
        if(b && !bq && dap!=-1)
        {
            sprintf(ed.str, "history:%s", search_ids[dap]);
            lasttime = TIMEOUT;
        }

        if(b && !bq && tp!=-1)
        {
            strncpy(ed.str, tag_names[tp], 255);
            lasttime = TIMEOUT;
        }

        if(b && !bq && mp!=-1 && st)
        {
            sprintf(ed.str, "user:%s", search_owners[mp]);
            lasttime = TIMEOUT;
        }

        if(do_open==1)
        {
            mp = 0;
        }

        if((b && !bq && mp!=-1 && !st && !uih) || do_open==1)
        {
            fillrect(vid_buf, 0, 0, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 255);
            info_box(vid_buf, "Loading...");

            if(search_dates[mp]) {
                uri = malloc(strlen(search_ids[mp])*3+strlen(search_dates[mp])*3+strlen(SERVER)+71);
                strcpy(uri, "http://" SERVER "/Get.api?Op=save&ID=");
                strcaturl(uri, search_ids[mp]);
                strappend(uri, "&Date=");
                strcaturl(uri, search_dates[mp]);
            } else {
                uri = malloc(strlen(search_ids[mp])*3+strlen(SERVER)+64);
                strcpy(uri, "http://" SERVER "/Get.api?Op=save&ID=");
                strcaturl(uri, search_ids[mp]);
            }
            data = http_simple_get(uri, &status, &dlen);
            free(uri);

            if(status == 200)
            {
                status = parse_save(data, dlen, 1, 0, 0);
                switch(status)
                {
                case 1:
                    error_ui(vid_buf, 0, "Simulation corrupted");
                    break;
                case 2:
                    error_ui(vid_buf, 0, "Simulation from a newer version");
                    break;
                case 3:
                    error_ui(vid_buf, 0, "Simulation on a too large grid");
                    break;
                }
                if(!status)
                {
                    char *tnames[] = {"ID", NULL};
                    char *tparts[1];
                    int tplens[1];
                    if(svf_last)
                        free(svf_last);
                    svf_last = data;
                    svf_lsize = dlen;

                    tparts[0] = search_ids[mp];
                    tplens[0] = strlen(search_ids[mp]);
                    data = http_multipart_post("http://" SERVER "/Tags.api", tnames, tparts, tplens, svf_user, svf_pass, &status, NULL);

                    svf_open = 1;
                    svf_own = svf_login && !strcmp(search_owners[mp], svf_user);
                    svf_publish = search_publish[mp] && svf_login && !strcmp(search_owners[mp], svf_user);

                    strcpy(svf_id, search_ids[mp]);
                    strcpy(svf_name, search_names[mp]);
                    if(status == 200)
                    {
                        if(data)
                        {
                            strncpy(svf_tags, data, 255);
                            svf_tags[255] = 0;
                        }
                        else
                            svf_tags[0] = 0;
                    }
                    else
                    {
                        svf_tags[0] = 0;
                    }

                    if(svf_login)
                    {
                        char *names[] = {"ID", NULL};
                        char *parts[1];
                        parts[0] = search_ids[mp];
                        data = http_multipart_post("http://" SERVER "/Vote.api", names, parts, NULL, svf_user, svf_pass, &status, NULL);
                        if(status == 200)
                        {
                            if(data)
                            {
                                if(!strcmp(data, "Up"))
                                {
                                    svf_myvote = 1;
                                }
                                else if(!strcmp(data, "Down"))
                                {
                                    svf_myvote = -1;
                                }
                                else
                                {
                                    svf_myvote = 0;
                                }
                            }
                            else
                            {
                                svf_myvote = 0;
                            }
                        }
                        else
                        {
                            svf_myvote = 0;
                        }
                    }
                }
                else
                {
                    svf_open = 0;
                    svf_publish = 0;
                    svf_own = 0;
                    svf_myvote = 0;
                    svf_id[0] = 0;
                    svf_name[0] = 0;
                    svf_tags[0] = 0;
                    if(svf_last)
                        free(svf_last);
                    svf_last = NULL;
                }
            }
            else
                error_ui(vid_buf, status, http_ret_text(status));

            if(data)
                free(data);
            goto finish;
        }

        if(!last)
        {
            search = 1;
        }
        else if(!active && (strcmp(last, ed.str) || last_own!=search_own || last_date!=search_date || last_page!=search_page))
        {
            search = 1;
            if(strcmp(last, ed.str) || last_own!=search_own || last_date!=search_date)
            {
                search_page = 0;
                page_count = 0;
            }
            free(last);
            last = NULL;
        }
        else
            search = 0;

        if(search && lasttime>=TIMEOUT)
        {
            lasttime = 0;
            last = mystrdup(ed.str);
            last_own = search_own;
            last_date = search_date;
            last_page = search_page;
            active = 1;
            uri = malloc(strlen(last)*3+80+strlen(SERVER)+strlen(svf_user));
            if(search_own || svf_admin || svf_mod)
                tmp = "&ShowVotes=true";
            else
                tmp = "";
            if(!search_own && !search_date && !*last)
            {
                if(search_page)
                {
                    exp_res = GRID_X*GRID_Y;
                    sprintf(uri, "http://" SERVER "/Search.api?Start=%d&Count=%d%s&Query=", (search_page-1)*GRID_X*GRID_Y+GRID_X*GRID_P, exp_res+1, tmp);
                }
                else
                {
                    exp_res = GRID_X*GRID_P;
                    sprintf(uri, "http://" SERVER "/Search.api?Start=%d&Count=%d&t=%d%s&Query=", 0, exp_res+1, ((GRID_Y-GRID_P)*YRES)/(GRID_Y*14)*GRID_X, tmp);
                }
            }
            else
            {
                exp_res = GRID_X*GRID_Y;
                sprintf(uri, "http://" SERVER "/Search.api?Start=%d&Count=%d%s&Query=", search_page*GRID_X*GRID_Y, exp_res+1, tmp);
            }
            strcaturl(uri, last);
            if(search_own)
            {
                strcaturl(uri, " user:");
                strcaturl(uri, svf_user);
            }
            if(search_date)
                strcaturl(uri, " sort:date");

            http = http_async_req_start(http, uri, NULL, 0, 1);
            if(svf_login)
            {
                http_auth_headers(http, svf_user, svf_pass);
            }
            http_last_use = time(NULL);
            free(uri);
        }

        if(active && http_async_req_status(http))
        {
            http_last_use = time(NULL);
            results = http_async_req_stop(http, &status, NULL);
            view_own = last_own;
            if(status == 200)
            {
                page_count = search_results(results, last_own||svf_admin||svf_mod);
                memset(thumb_drawn, 0, sizeof(thumb_drawn));
                memset(v_buf, 0, ((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);
            }
            is_p1 = (exp_res < GRID_X*GRID_Y);
            free(results);
            active = 0;
        }

        if(http && !active && (time(NULL)>http_last_use+HTTP_TIMEOUT))
        {
            http_async_req_close(http);
            http = NULL;
        }

        for(i=0; i<IMGCONNS; i++)
        {
            if(img_http[i] && http_async_req_status(img_http[i]))
            {
                thumb = http_async_req_stop(img_http[i], &status, &thlen);
                if(status != 200)
                {
                    if(thumb)
                        free(thumb);
                    thumb = calloc(1,4);
                    thlen = 4;
                }
                thumb_cache_add(img_id[i], thumb, thlen);
                for(pos=0; pos<GRID_X*GRID_Y; pos++) {
                    if(search_dates[pos]) {
                        char *id_d_temp = malloc(strlen(search_ids[pos])+strlen(search_dates[pos])+1);
                        strcpy(id_d_temp, search_ids[pos]);
                        strappend(id_d_temp, "_");
                        strappend(id_d_temp, search_dates[pos]);
                        //img_id[i] = mystrdup(id_d_temp);
                        if(id_d_temp && !strcmp(id_d_temp, img_id[i])) {
                            break;
                        }
                    } else {
                        if(search_ids[pos] && !strcmp(search_ids[pos], img_id[i])) {
                            break;
                        }
                    }
                }
                if(pos<GRID_X*GRID_Y)
                {
                    search_thumbs[pos] = thumb;
                    search_thsizes[pos] = thlen;
                }
                else
                    free(thumb);
                free(img_id[i]);
                img_id[i] = NULL;
            }
            if(!img_id[i])
            {
                for(pos=0; pos<GRID_X*GRID_Y; pos++)
                    if(search_ids[pos] && !search_thumbs[pos])
                    {
                        for(gi=0; gi<IMGCONNS; gi++)
                            if(img_id[gi] && !strcmp(search_ids[pos], img_id[gi]))
                                break;
                        if(gi<IMGCONNS)
                            continue;
                        break;
                    }
                if(pos<GRID_X*GRID_Y)
                {
                    if(search_dates[pos]) {
                        char *id_d_temp = malloc(strlen(search_ids[pos])+strlen(search_dates[pos])+1);
                        uri = malloc(strlen(search_ids[pos])*3+strlen(search_dates[pos])*3+strlen(SERVER)+71);
                        strcpy(uri, "http://" SERVER "/Get.api?Op=thumb&ID=");
                        strcaturl(uri, search_ids[pos]);
                        strappend(uri, "&Date=");
                        strcaturl(uri, search_dates[pos]);

                        strcpy(id_d_temp, search_ids[pos]);
                        strappend(id_d_temp, "_");
                        strappend(id_d_temp, search_dates[pos]);
                        img_id[i] = mystrdup(id_d_temp);
                    } else {
                        uri = malloc(strlen(search_ids[pos])*3+strlen(SERVER)+64);
                        strcpy(uri, "http://" SERVER "/Get.api?Op=thumb&ID=");
                        strcaturl(uri, search_ids[pos]);
                        img_id[i] = mystrdup(search_ids[pos]);
                    }

                    img_http[i] = http_async_req_start(img_http[i], uri, NULL, 0, 1);
                    free(uri);
                }
            }
            if(!img_id[i] && img_http[i])
            {
                http_async_req_close(img_http[i]);
                img_http[i] = NULL;
            }
        }

        if(lasttime<TIMEOUT)
            lasttime++;
    }

finish:
    if(last)
        free(last);
    if(http)
        http_async_req_close(http);
    for(i=0; i<IMGCONNS; i++)
        if(img_http[i])
            http_async_req_close(img_http[i]);

    search_results("", 0);

    strcpy(search_expr, ed.str);

    return 0;
}

int search_results(char *str, int votes)
{
    int i,j;
    char *p,*q,*r,*s,*vu,*vd,*pu,*sd;

    for(i=0; i<GRID_X*GRID_Y; i++)
    {
        if(search_ids[i])
        {
            free(search_ids[i]);
            search_ids[i] = NULL;
        }
        if(search_names[i])
        {
            free(search_names[i]);
            search_names[i] = NULL;
        }
        if(search_dates[i])
        {
            free(search_dates[i]);
            search_dates[i] = NULL;
        }
        if(search_owners[i])
        {
            free(search_owners[i]);
            search_owners[i] = NULL;
        }
        if(search_thumbs[i])
        {
            free(search_thumbs[i]);
            search_thumbs[i] = NULL;
            search_thsizes[i] = 0;
        }
    }
    for(j=0; j<TAG_MAX; j++)
        if(tag_names[j])
        {
            free(tag_names[j]);
            tag_names[j] = NULL;
        }

    if(!str || !*str)
        return 0;

    i = 0;
    j = 0;
    s = NULL;
    do_open = 0;
    while(1)
    {
        if(!*str)
            break;
        p = strchr(str, '\n');
        if(!p)
            p = str + strlen(str);
        else
            *(p++) = 0;
        if(!strncmp(str, "OPEN ", 5))
        {
            do_open = 1;
            if(i>=GRID_X*GRID_Y)
                break;
            if(votes)
            {
                pu = strchr(str+5, ' ');
                if(!pu)
                    return i;
                *(pu++) = 0;
                s = strchr(pu, ' ');
                if(!s)
                    return i;
                *(s++) = 0;
                vu = strchr(s, ' ');
                if(!vu)
                    return i;
                *(vu++) = 0;
                vd = strchr(vu, ' ');
                if(!vd)
                    return i;
                *(vd++) = 0;
                q = strchr(vd, ' ');
            }
            else
            {
                pu = strchr(str+5, ' ');
                if(!pu)
                    return i;
                *(pu++) = 0;
                vu = strchr(pu, ' ');
                if(!vu)
                    return i;
                *(vu++) = 0;
                vd = strchr(vu, ' ');
                if(!vd)
                    return i;
                *(vd++) = 0;
                q = strchr(vd, ' ');
            }
            if(!q)
                return i;
            *(q++) = 0;
            r = strchr(q, ' ');
            if(!r)
                return i;
            *(r++) = 0;
            search_ids[i] = mystrdup(str+5);

            search_publish[i] = atoi(pu);
            search_scoreup[i] = atoi(vu);
            search_scoredown[i] = atoi(vd);

            search_owners[i] = mystrdup(q);
            search_names[i] = mystrdup(r);

            if(s)
                search_votes[i] = atoi(s);
            thumb_cache_find(str+5, search_thumbs+i, search_thsizes+i);
            i++;
        }
        else if(!strncmp(str, "HISTORY ", 8))
        {
            if(i>=GRID_X*GRID_Y)
                break;
            if(votes)
            {
                sd = strchr(str+8, ' ');
                if(!sd)
                    return i;
                *(sd++) = 0;
                pu = strchr(sd, ' ');
                if(!pu)
                    return i;
                *(pu++) = 0;
                s = strchr(pu, ' ');
                if(!s)
                    return i;
                *(s++) = 0;
                vu = strchr(s, ' ');
                if(!vu)
                    return i;
                *(vu++) = 0;
                vd = strchr(vu, ' ');
                if(!vd)
                    return i;
                *(vd++) = 0;
                q = strchr(vd, ' ');
            }
            else
            {
                sd = strchr(str+8, ' ');
                if(!sd)
                    return i;
                *(sd++) = 0;
                pu = strchr(sd, ' ');
                if(!pu)
                    return i;
                *(pu++) = 0;
                vu = strchr(pu, ' ');
                if(!vu)
                    return i;
                *(vu++) = 0;
                vd = strchr(vu, ' ');
                if(!vd)
                    return i;
                *(vd++) = 0;
                q = strchr(vd, ' ');
            }
            if(!q)
                return i;
            *(q++) = 0;
            r = strchr(q, ' ');
            if(!r)
                return i;
            *(r++) = 0;
            search_ids[i] = mystrdup(str+8);

            search_dates[i] = mystrdup(sd);

            search_publish[i] = atoi(pu);
            search_scoreup[i] = atoi(vu);
            search_scoredown[i] = atoi(vd);

            search_owners[i] = mystrdup(q);
            search_names[i] = mystrdup(r);

            if(s)
                search_votes[i] = atoi(s);
            thumb_cache_find(str+8, search_thumbs+i, search_thsizes+i);
            i++;
        }
        else if(!strncmp(str, "TAG ", 4))
        {
            if(j >= TAG_MAX)
            {
                str = p;
                continue;
            }
            q = strchr(str+4, ' ');
            if(!q)
            {
                str = p;
                continue;
            }
            *(q++) = 0;
            tag_names[j] = mystrdup(str+4);
            tag_votes[j] = atoi(q);
            j++;
        }
        else
        {
            if(i>=GRID_X*GRID_Y)
                break;
            if(votes)
            {
                pu = strchr(str, ' ');
                if(!pu)
                    return i;
                *(pu++) = 0;
                s = strchr(pu, ' ');
                if(!s)
                    return i;
                *(s++) = 0;
                vu = strchr(s, ' ');
                if(!vu)
                    return i;
                *(vu++) = 0;
                vd = strchr(vu, ' ');
                if(!vd)
                    return i;
                *(vd++) = 0;
                q = strchr(vd, ' ');
            }
            else
            {
                pu = strchr(str, ' ');
                if(!pu)
                    return i;
                *(pu++) = 0;
                vu = strchr(pu, ' ');
                if(!vu)
                    return i;
                *(vu++) = 0;
                vd = strchr(vu, ' ');
                if(!vd)
                    return i;
                *(vd++) = 0;
                q = strchr(vd, ' ');
            }
            if(!q)
                return i;
            *(q++) = 0;
            r = strchr(q, ' ');
            if(!r)
                return i;
            *(r++) = 0;
            search_ids[i] = mystrdup(str);

            search_publish[i] = atoi(pu);
            search_scoreup[i] = atoi(vu);
            search_scoredown[i] = atoi(vd);

            search_owners[i] = mystrdup(q);
            search_names[i] = mystrdup(r);

            if(s)
                search_votes[i] = atoi(s);
            thumb_cache_find(str, search_thumbs+i, search_thsizes+i);
            i++;
        }
        str = p;
    }
    if(*str)
        i++;
    return i;
}

int execute_tagop(pixel *vid_buf, char *op, char *tag)
{
    int status;
    char *result;

    char *names[] = {"ID", "Tag", NULL};
    char *parts[2];

    char *uri = malloc(strlen(SERVER)+strlen(op)+36);
    sprintf(uri, "http://" SERVER "/Tag.api?Op=%s", op);

    parts[0] = svf_id;
    parts[1] = tag;

    result = http_multipart_post(
                 uri,
                 names, parts, NULL,
                 svf_user, svf_pass,
                 &status, NULL);

    free(uri);

    if(status!=200)
    {
        error_ui(vid_buf, status, http_ret_text(status));
        if(result)
            free(result);
        return 1;
    }
    if(result && strncmp(result, "OK", 2))
    {
        error_ui(vid_buf, 0, result);
        free(result);
        return 1;
    }

    if(result[2])
    {
        strncpy(svf_tags, result+3, 255);
        svf_id[15] = 0;
    }

    if(result)
        free(result);

    return 0;
}

void execute_save(pixel *vid_buf)
{
    int status;
    char *result;

    char *names[] = {"Name", "Data:save.bin", "Thumb:thumb.bin", "Publish", "ID", NULL};
    char *parts[5];
    int plens[5];

    parts[0] = svf_name;
    plens[0] = strlen(svf_name);
    parts[1] = build_save(plens+1, 0, 0, XRES, YRES);
    parts[2] = build_thumb(plens+2, 1);
    parts[3] = (svf_publish==1)?"Public":"Private";
    plens[3] = strlen((svf_publish==1)?"Public":"Private");

    if(svf_id[0])
    {
        parts[4] = svf_id;
        plens[4] = strlen(svf_id);
    }
    else
        names[4] = NULL;

    result = http_multipart_post(
                 "http://" SERVER "/Save.api",
                 names, parts, plens,
                 svf_user, svf_pass,
                 &status, NULL);

    if(svf_last)
        free(svf_last);
    svf_last = parts[1];
    svf_lsize = plens[1];

    free(parts[2]);

    if(status!=200)
    {
        error_ui(vid_buf, status, http_ret_text(status));
        if(result)
            free(result);
        return;
    }
    if(result && strncmp(result, "OK", 2))
    {
        error_ui(vid_buf, 0, result);
        free(result);
        return;
    }

    if(result[2])
    {
        strncpy(svf_id, result+3, 15);
        svf_id[15] = 0;
    }

    if(!svf_id[0])
    {
        error_ui(vid_buf, 0, "No ID supplied by server");
        free(result);
        return;
    }

    thumb_cache_inval(svf_id);

    svf_own = 1;
    if(result)
        free(result);
}

void execute_delete(pixel *vid_buf, char *id)
{
    int status;
    char *result;

    char *names[] = {"ID", NULL};
    char *parts[1];

    parts[0] = id;

    result = http_multipart_post(
                 "http://" SERVER "/Delete.api",
                 names, parts, NULL,
                 svf_user, svf_pass,
                 &status, NULL);

    if(status!=200)
    {
        error_ui(vid_buf, status, http_ret_text(status));
        if(result)
            free(result);
        return;
    }
    if(result && strncmp(result, "OK", 2))
    {
        error_ui(vid_buf, 0, result);
        free(result);
        return;
    }

    if(result)
        free(result);
}

int execute_vote(pixel *vid_buf, char *id, char *action)
{
    int status;
    char *result;

    char *names[] = {"ID", "Action", NULL};
    char *parts[2];

    parts[0] = id;
    parts[1] = action;

    result = http_multipart_post(
                 "http://" SERVER "/Vote.api",
                 names, parts, NULL,
                 svf_user, svf_pass,
                 &status, NULL);

    if(status!=200)
    {
        error_ui(vid_buf, status, http_ret_text(status));
        if(result)
            free(result);
        return 0;
    }
    if(result && strncmp(result, "OK", 2))
    {
        error_ui(vid_buf, 0, result);
        free(result);
        return 0;
    }

    if(result)
        free(result);
    return 1;
}
