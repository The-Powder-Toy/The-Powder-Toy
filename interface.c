#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "font.h"
#include "defines.h"
#include "powder.h"
#include "interface.h"

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

int Z_keysym = 'z';

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
            if( event.key.keysym.sym == SDLK_PLUS || event.key.keysym.sym == SDLK_RIGHTBRACKET)
            {
                sdl_wheel++;
            }
            if( event.key.keysym.sym == SDLK_MINUS || event.key.keysym.sym == SDLK_LEFTBRACKET)
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