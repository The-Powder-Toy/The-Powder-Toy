#ifdef MACOSX
#include <CoreFoundation/CFString.h>
#endif
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
#include <console.h>
#include <images.h>
#if defined(WIN32) && !defined(__GNUC__)
#include <io.h>
#else
#include <dirent.h>
#endif

SDLMod sdl_mod;
int sdl_key, sdl_wheel, sdl_caps=0, sdl_ascii, sdl_zoom_trig=0;
#if (defined(LIN32) || defined(LIN64)) && defined(SDL_VIDEO_DRIVER_X11)
SDL_SysWMinfo sdl_wminfo;
Atom XA_CLIPBOARD, XA_TARGETS;
#endif

char *shift_0="`1234567890-=[]\\;',./";
char *shift_1="~!@#$%^&*()_+{}|:\"<>?";

int svf_messages = 0;
int svf_login = 0;
int svf_admin = 0;
int svf_mod = 0;
char svf_user[64] = "";
char svf_user_id[64] = "";
char svf_pass[64] = "";
char svf_session_id[64] = "";

int svf_open = 0;
int svf_own = 0;
int svf_myvote = 0;
int svf_publish = 0;
char svf_filename[255] = "";
int svf_fileopen = 0;
char svf_id[16] = "";
char svf_name[64] = "";
char svf_description[255] = "";
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
int search_fav = 0;
int search_date = 0;
int search_page = 0;
char search_expr[256] = "";

char server_motd[512] = "";

char *tag_names[TAG_MAX];
int tag_votes[TAG_MAX];

int zoom_en = 0;
int zoom_x=(XRES-ZSIZE_D)/2, zoom_y=(YRES-ZSIZE_D)/2;
int zoom_wx=0, zoom_wy=0;
unsigned char ZFACTOR = 256/ZSIZE_D;
unsigned char ZSIZE = ZSIZE_D;

int drawgrav_enable = 0;

void menu_count(void)//puts the number of elements in each section into .itemcount
{
	int i=0;
	msections[SC_LIFE].itemcount = NGOLALT;
	msections[SC_WALL].itemcount = UI_WALLCOUNT-4;
	msections[SC_SPECIAL].itemcount = 4;
	for (i=0; i<PT_NUM; i++)
	{
		msections[ptypes[i].menusection].itemcount+=ptypes[i].menu;
	}

}

void get_sign_pos(int i, int *x0, int *y0, int *w, int *h)
{
	//Changing width if sign have special content
	if (strcmp(signs[i].text, "{p}")==0)
		*w = textwidth("Pressure: -000.00");

	if (strcmp(signs[i].text, "{t}")==0)
		*w = textwidth("Temp: 0000.00");

	if (sregexp(signs[i].text, "^{c:[0-9]*|.*}$")==0)
	{
		int sldr, startm;
		char buff[256];
		memset(buff, 0, sizeof(buff));
		for (sldr=3; signs[i].text[sldr-1] != '|'; sldr++)
			startm = sldr + 1;

		sldr = startm;
		while (signs[i].text[sldr] != '}')
		{
			buff[sldr - startm] = signs[i].text[sldr];
			sldr++;
		}
		*w = textwidth(buff) + 5;
	}

	//Ususal width
	if (strcmp(signs[i].text, "{p}") && strcmp(signs[i].text, "{t}") && sregexp(signs[i].text, "^{c:[0-9]*|.*}$"))
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
	for (i=0; i<MAXSIGNS; i++)
		if (signs[i].text[0])
		{
			if (i == MSIGN)
			{
				MSIGN = -1;
				return;
			}
			get_sign_pos(i, &x, &y, &w, &h);
			if (mx>=x && mx<=x+w && my>=y && my<=y+h)
				break;
		}
	// else look for empty spot
	if (i >= MAXSIGNS)
	{
		nm = 1;
		for (i=0; i<MAXSIGNS; i++)
			if (!signs[i].text[0])
				break;
	}
	if (i >= MAXSIGNS)
		return;
	if (nm)
	{
		signs[i].x = mx;
		signs[i].y = my;
		signs[i].ju = 1;
	}

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
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
	ed.multiline = 0;
	strcpy(ed.str, signs[i].text);
	ju = signs[i].ju;

	fillrect(vid_buf, -1, -1, XRES, YRES+MENUSIZE, 0, 0, 0, 192);
	while (!sdl_poll())
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





		if (!nm)
		{
			drawtext(vid_buf, x0+138, y0+45, "\x86", 160, 48, 32, 255);
			drawtext(vid_buf, x0+138, y0+45, "\x85", 255, 255, 255, 255);
			drawtext(vid_buf, x0+152, y0+46, "Delete", 255, 255, 255, 255);
			drawrect(vid_buf, x0+134, y0+42, 50, 15, 255, 255, 255, 255);
			drawrect(vid_buf,x0+104,y0+42,26,15,255,255,255,255);
			drawtext(vid_buf, x0+110, y0+48, "Mv.", 255, 255, 255, 255);
		}

		drawtext(vid_buf, x0+5, y0+69, "OK", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+64, 192, 16, 192, 192, 192, 255);

		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		ui_edit_process(mx, my, b, &ed);

		if (b && !bq && mx>=x0+50 && mx<=x0+67 && my>=y0+42 && my<=y0+59)
			ju = 0;
		if (b && !bq && mx>=x0+68 && mx<=x0+85 && my>=y0+42 && my<=y0+59)
			ju = 1;
		if (b && !bq && mx>=x0+86 && mx<=x0+103 && my>=y0+42 && my<=y0+59)
			ju = 2;

		if (!nm && b && !bq && mx>=x0+104 && mx<=x0+130 && my>=y0+42 && my<=y0+59)
		{
			MSIGN = i;
			break;
		}
		if (b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+22 && my<y0+36)
			break;
		if (b && !bq && mx>=x0 && mx<x0+192 && my>=y0+64 && my<=y0+80)
			break;

		if (!nm && b && !bq && mx>=x0+134 && my>=y0+42 && mx<=x0+184 && my<=y0+59)
		{
			signs[i].text[0] = 0;
			return;
		}

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
		{
			if (!ed.focus)
				return;
			ed.focus = 0;
		}
	}

	strcpy(signs[i].text, ed.str);
	signs[i].ju = ju;
}
//TODO: Finish text wrapping in text edits
void ui_edit_draw(pixel *vid_buf, ui_edit *ed)
{
	int cx, i, cy;
	char echo[256], *str;

	if (ed->hide)
	{
		for (i=0; ed->str[i]; i++)
			echo[i] = 0x8D;
		echo[i] = 0;
		str = echo;
	}
	else
		str = ed->str;

	if (ed->str[0])
	{
		if (ed->multiline) {
			drawtextwrap(vid_buf, ed->x, ed->y, ed->w-14, str, 255, 255, 255, 255);
			drawtext(vid_buf, ed->x+ed->w-11, ed->y-1, "\xAA", 128, 128, 128, 255);
		} else {
			drawtext(vid_buf, ed->x, ed->y, str, 255, 255, 255, 255);
			drawtext(vid_buf, ed->x+ed->w-11, ed->y-1, "\xAA", 128, 128, 128, 255);
		}
	}
	else if (!ed->focus)
		drawtext(vid_buf, ed->x, ed->y, ed->def, 128, 128, 128, 255);
	if (ed->focus)
	{
		if (ed->multiline) {
			textnpos(str, ed->cursor, ed->w-14, &cx, &cy);
		} else {
			cx = textnwidth(str, ed->cursor);
			cy = 0;
		}

		for (i=-3; i<9; i++)
			drawpixel(vid_buf, ed->x+cx, ed->y+i+cy, 255, 255, 255, 255);
	}
}

void ui_edit_process(int mx, int my, int mb, ui_edit *ed)
{
	char ch, ts[2], echo[256], *str;
	int l, i;
#ifdef RAWINPUT
	char *p;
#endif

	if (mb)
	{
		if (ed->hide)
		{
			for (i=0; ed->str[i]; i++)
				echo[i] = 0x8D;
			echo[i] = 0;
			str = echo;
		}
		else
			str = ed->str;

		if (ed->multiline) {
			if (mx>=ed->x+ed->w-11 && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+11)
			{
				ed->focus = 1;
				ed->cursor = 0;
				ed->str[0] = 0;
			}
			else if (mx>=ed->x-ed->nx && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+ed->h)
			{
				ed->focus = 1;
				ed->cursor = textposxy(str, ed->w-14, mx-ed->x, my-ed->y);
			}
			else
				ed->focus = 0;
		} else {
			if (mx>=ed->x+ed->w-11 && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+11)
			{
				ed->focus = 1;
				ed->cursor = 0;
				ed->str[0] = 0;
			}
			else if (mx>=ed->x-ed->nx && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+11)
			{
				ed->focus = 1;
				ed->cursor = textwidthx(str, mx-ed->x);
			}
			else
				ed->focus = 0;
		}
	}
	if (ed->focus && sdl_key)
	{
		if (ed->hide)
		{
			for (i=0; ed->str[i]; i++)
				echo[i] = 0x8D;
			echo[i] = 0;
			str = echo;
		}
		else
			str = ed->str;

		l = strlen(ed->str);
		switch (sdl_key)
		{
		case SDLK_HOME:
			ed->cursor = 0;
			break;
		case SDLK_END:
			ed->cursor = l;
			break;
		case SDLK_LEFT:
			if (ed->cursor > 0)
				ed->cursor --;
			break;
		case SDLK_RIGHT:
			if (ed->cursor < l)
				ed->cursor ++;
			break;
		case SDLK_DELETE:
			if (sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))
				ed->str[ed->cursor] = 0;
			else if (ed->cursor < l)
				memmove(ed->str+ed->cursor, ed->str+ed->cursor+1, l-ed->cursor);
			break;
		case SDLK_BACKSPACE:
			if (sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))
			{
				if (ed->cursor > 0)
					memmove(ed->str, ed->str+ed->cursor, l-ed->cursor+1);
				ed->cursor = 0;
			}
			else if (ed->cursor > 0)
			{
				ed->cursor--;
				memmove(ed->str+ed->cursor, ed->str+ed->cursor+1, l-ed->cursor);
			}
			break;
		default:
			if(sdl_mod & (KMOD_CTRL) && sdl_key=='c')//copy
			{
				clipboard_push_text(ed->str);
				break;
			}
			else if(sdl_mod & (KMOD_CTRL) && sdl_key=='v')//paste
			{
				char *paste = clipboard_pull_text();
				int pl = strlen(paste);
				if ((textwidth(str)+textwidth(paste) > ed->w-14 && !ed->multiline) || (pl+strlen(ed->str)>255) || (float)(((textwidth(str)+textwidth(paste))/(ed->w-14)*12) > ed->h && ed->multiline))
					break;
				memmove(ed->str+ed->cursor+pl, ed->str+ed->cursor, l+pl-ed->cursor);
				memcpy(ed->str+ed->cursor,paste,pl);
				ed->cursor += pl;
				break;
			}
#ifdef RAWINPUT
			if (sdl_key>=SDLK_SPACE && sdl_key<=SDLK_z && l<255)
			{
				ch = sdl_key;
				if ((sdl_mod & (KMOD_LSHIFT|KMOD_RSHIFT|KMOD_CAPS)))
				{
					if (ch>='a' && ch<='z')
						ch &= ~0x20;
					p = strchr(shift_0, ch);
					if (p)
						ch = shift_1[p-shift_0];
				}
				ts[0]=ed->hide?0x8D:ch;
				ts[1]=0;
				if ((textwidth(str)+textwidth(ts) > ed->w-14 && !ed->multiline) || (float)(((textwidth(str)+textwidth(ts))/(ed->w-14)*12) > ed->h && ed->multiline))
					break;
				memmove(ed->str+ed->cursor+1, ed->str+ed->cursor, l+1-ed->cursor);
				ed->str[ed->cursor] = ch;
				ed->cursor++;
			}
#else
			if (sdl_ascii>=' ' && sdl_ascii<127 && l<255)
			{
				ch = sdl_ascii;
				ts[0]=ed->hide?0x8D:ch;
				ts[1]=0;
				if ((textwidth(str)+textwidth(ts) > ed->w-14 && !ed->multiline) || (float)(((textwidth(str)+textwidth(ts))/(ed->w-14)*12) > ed->h && ed->multiline))
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
	if (ed->checked)
	{
		drawtext(vid_buf, ed->x+2, ed->y+2, "\xCF", 128, 128, 128, 255);
	}
	if (ed->focus)
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

	if (mb && !mbq)
	{
		if (mx>=ed->x && mx<=ed->x+w && my>=ed->y && my<=ed->y+w)
		{
			ed->checked = (ed->checked)?0:1;
		}
	}
	else
	{
		if (mx>=ed->x && mx<=ed->x+w && my>=ed->y && my<=ed->y+w)
		{
			ed->focus = 1;
		}
		else
		{
			ed->focus = 0;
		}
	}
}

void ui_copytext_draw(pixel *vid_buf, ui_copytext *ed)
{
	int g = 180, i = 0;
	if (!ed->state) {
		if (ed->hover) {
			i = 0;
		} else {
			i = 100;
		}
		g = 255;
		drawtext(vid_buf, (ed->x+(ed->width/2))-(textwidth("Click the box to copy the text")/2), ed->y-12, "Click the box to copy the text", 255, 255, 255, 255-i);
	} else {
		i = 0;
		drawtext(vid_buf, (ed->x+(ed->width/2))-(textwidth("Copied!")/2), ed->y-12, "Copied!", 255, 255, 255, 255-i);
		g = 190;
	}

	drawrect(vid_buf, ed->x, ed->y, ed->width, ed->height, g, 255, g, 255-i);
	drawrect(vid_buf, ed->x+1, ed->y+1, ed->width-2, ed->height-2, g, 255, g, 100-i);
	drawtext(vid_buf, ed->x+6, ed->y+5, ed->text, g, 255, g, 230-i);
}

void ui_copytext_process(int mx, int my, int mb, int mbq, ui_copytext *ed)
{
	if (my>=ed->y && my<=ed->y+ed->height && mx>=ed->x && mx<=ed->x+ed->width && !ed->state) {
		if (mb && !mbq) {
			clipboard_push_text(ed->text);
			ed->state = 1;
		}
		ed->hover = 1;
	} else {
		ed->hover = 0;
	}
}

void ui_richtext_draw(pixel *vid_buf, ui_richtext *ed)
{
	ed->str[511] = 0;
	ed->printstr[511] = 0;
	drawtext(vid_buf, ed->x, ed->y, ed->printstr, 255, 255, 255, 255);
}

int markup_getregion(char *text, char *action, char *data, char *atext){
	int datamarker = 0;
	int terminator = 0;
	int minit;
	if (sregexp(text, "^{a:.*|.*}")==0)
	{
		*action = text[1];
		for (minit=3; text[minit-1] != '|'; minit++)
			datamarker = minit + 1;
		for (minit=datamarker; text[minit-1] != '}'; minit++)
			terminator = minit + 1;
		strncpy(data, text+3, datamarker-4);
		strncpy(atext, text+datamarker, terminator-datamarker-1);
		return terminator;
	}
	else
	{
		return 0;
	}	
}

void ui_richtext_settext(char *text, ui_richtext *ed)
{
	int pos = 0, action = 0, ppos = 0, ipos = 0;
	memset(ed->printstr, 0, 512);
	memset(ed->str, 0, 512);
	strcpy(ed->str, text);
	//strcpy(ed->printstr, text);
	for(action = 0; action < 6; action++){
		ed->action[action] = 0;	
		memset(ed->actiondata[action], 0, 256);
		memset(ed->actiontext[action], 0, 256);
	}
	action = 0;
	for(pos = 0; pos<512; ){
		if(!ed->str[pos])
			break;
		if(ed->str[pos] == '{'){
			int mulen = 0;
			mulen = markup_getregion(ed->str+pos, &ed->action[action], ed->actiondata[action], ed->actiontext[action]);
			if(mulen){
				ed->regionss[action] = ipos;
				ed->regionsf[action] = ipos + strlen(ed->actiontext[action]);
				//printf("%c, %s, %s [%d, %d]\n", ed->action[action], ed->actiondata[action], ed->actiontext[action], ed->regionss[action], ed->regionsf[action]);
				strcpy(ed->printstr+ppos, ed->actiontext[action]);
				ppos+=strlen(ed->actiontext[action]);
				ipos+=strlen(ed->actiontext[action]);
				pos+=mulen;
				action++;			
			} 
			else
			{
				pos++;			
			}
		} else {
			ed->printstr[ppos] = ed->str[pos];
			ppos++;
			pos++;
			ipos++;
			if(ed->str[pos] == '\b'){
				ipos-=2;			
			}
		}
	}
	ed->printstr[ppos] = 0;
	//printf("%s\n", ed->printstr);
}

void ui_richtext_process(int mx, int my, int mb, int mbq, ui_richtext *ed)
{
	int action = 0;
	int currentpos = 0;
	if(mx>ed->x && mx < ed->x+textwidth(ed->printstr) && my > ed->y && my < ed->y + 10 && mb && !mbq){
		currentpos = textwidthx(ed->printstr, mx-ed->x);
		for(action = 0; action < 6; action++){
			if(currentpos >= ed->regionss[action] && currentpos <= ed->regionsf[action])
			{	
				//Do action
				if(ed->action[action]=='a'){
					//Open link
					open_link(ed->actiondata[action]);	
				}
				break;
			}
		}
	}
}

void draw_svf_ui(pixel *vid_buf, int alternate)// all the buttons at the bottom
{
	int c;

	//the open browser button
	if(alternate)
	{
		fillrect(vid_buf, 0, YRES+(MENUSIZE-16)-1, 18, 16, 255, 255, 255, 255);
		drawtext(vid_buf, 4, YRES+(MENUSIZE-14), "\x81", 0, 0, 0, 255);
	} else {
		drawtext(vid_buf, 4, YRES+(MENUSIZE-14), "\x81", 255, 255, 255, 255);
		drawrect(vid_buf, 1, YRES+(MENUSIZE-16), 16, 14, 255, 255, 255, 255);
	}

	// the reload button
	c = (svf_open || svf_fileopen) ? 255 : 128;
	drawtext(vid_buf, 23, YRES+(MENUSIZE-14), "\x91", c, c, c, 255);
	drawrect(vid_buf, 19, YRES+(MENUSIZE-16), 16, 14, c, c, c, 255);

	// the save sim button
	if(alternate)
	{
		fillrect(vid_buf, 36, YRES+(MENUSIZE-16)-1, 152, 16, 255, 255, 255, 255);
		drawtext(vid_buf, 40, YRES+(MENUSIZE-14), "\x82", 0, 0, 0, 255);
		if(svf_fileopen)
			drawtext(vid_buf, 58, YRES+(MENUSIZE-12), svf_filename, 0, 0, 0, 255);
		else
			drawtext(vid_buf, 58, YRES+(MENUSIZE-12), "[save to disk]", 0, 0, 0, 255);
	} else {
		c = svf_login ? 255 : 128;
		drawtext(vid_buf, 40, YRES+(MENUSIZE-14), "\x82", c, c, c, 255);
		if (svf_open)
			drawtextmax(vid_buf, 58, YRES+(MENUSIZE-12), 125, svf_name, c, c, c, 255);
		else
			drawtext(vid_buf, 58, YRES+(MENUSIZE-12), "[untitled simulation]", c, c, c, 255);
		drawrect(vid_buf, 37, YRES+(MENUSIZE-16), 150, 14, c, c, c, 255);
		if (svf_open && svf_own)
			drawdots(vid_buf, 55, YRES+(MENUSIZE-15), 12, c, c, c, 255);
	}

	c = (svf_login && svf_open) ? 255 : 128;

	//the vote buttons
	drawrect(vid_buf, 189, YRES+(MENUSIZE-16), 14, 14, c, c, c, 255);
	drawrect(vid_buf, 203, YRES+(MENUSIZE-16), 14, 14, c, c, c, 255);

	if (svf_myvote==1 && (svf_login && svf_open))
	{
		fillrect(vid_buf, 189, YRES+(MENUSIZE-16), 14, 14, 0, 108, 10, 255);
	}
	else if (svf_myvote==-1 && (svf_login && svf_open))
	{
		fillrect(vid_buf, 203, YRES+(MENUSIZE-16), 14, 14, 108, 10, 0, 255);
	}
	drawtext(vid_buf, 192, YRES+(MENUSIZE-12), "\xCB", 0, 187, 18, c);
	drawtext(vid_buf, 205, YRES+(MENUSIZE-14), "\xCA", 187, 40, 0, c);

	//the tags button
	drawtext(vid_buf, 222, YRES+(MENUSIZE-15), "\x83", c, c, c, 255);
	if (svf_tags[0])
		drawtextmax(vid_buf, 240, YRES+(MENUSIZE-12), XRES+BARSIZE-405, svf_tags, c, c, c, 255);
	else
		drawtext(vid_buf, 240, YRES+(MENUSIZE-12), "[no tags set]", c, c, c, 255);

	drawrect(vid_buf, 219, YRES+(MENUSIZE-16), XRES+BARSIZE-380, 14, c, c, c, 255);

	//the clear sim button------------some of the commented values are wrong
	drawtext(vid_buf, XRES-139+BARSIZE/*371*/, YRES+(MENUSIZE-14), "\x92", 255, 255, 255, 255);
	drawrect(vid_buf, XRES-143+BARSIZE/*367*/, YRES+(MENUSIZE-16), 16, 14, 255, 255, 255, 255);

	//the login button
	drawtext(vid_buf, XRES-122+BARSIZE/*388*/, YRES+(MENUSIZE-13), "\x84", 255, 255, 255, 255);
	if (svf_login)
		drawtextmax(vid_buf, XRES-104+BARSIZE/*406*/, YRES+(MENUSIZE-12), 66, svf_user, 255, 255, 255, 255);
	else
		drawtext(vid_buf, XRES-104+BARSIZE/*406*/, YRES+(MENUSIZE-12), "[sign in]", 255, 255, 255, 255);
	drawrect(vid_buf, XRES-125+BARSIZE/*385*/, YRES+(MENUSIZE-16), 91, 14, 255, 255, 255, 255);

	//te pause button
	if (sys_pause)
	{
		fillrect(vid_buf, XRES-17+BARSIZE/*493*/, YRES+(MENUSIZE-17), 16, 16, 255, 255, 255, 255);
		drawtext(vid_buf, XRES-14+BARSIZE/*496*/, YRES+(MENUSIZE-14), "\x90", 0, 0, 0, 255);
	}
	else
	{
		drawtext(vid_buf, XRES-14+BARSIZE/*496*/, YRES+(MENUSIZE-14), "\x90", 255, 255, 255, 255);
		drawrect(vid_buf, XRES-16+BARSIZE/*494*/, YRES+(MENUSIZE-16), 14, 14, 255, 255, 255, 255);
	}

	//The simulation options button, used to be the heat sim button
	/*if (!legacy_enable)
	{
		fillrect(vid_buf, XRES-160+BARSIZE, YRES+(MENUSIZE-17), 16, 16, 255, 255, 255, 255);
		drawtext(vid_buf, XRES-154+BARSIZE, YRES+(MENUSIZE-13), "\xBE", 255, 0, 0, 255);
		drawtext(vid_buf, XRES-154+BARSIZE, YRES+(MENUSIZE-13), "\xBD", 0, 0, 0, 255);
	}
	else*/
	{
		drawtext(vid_buf, XRES-156+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xCF", 255, 255, 255, 255);
		drawrect(vid_buf, XRES-159+BARSIZE/*494*/, YRES+(MENUSIZE-16), 14, 14, 255, 255, 255, 255);
	}

	//the view mode button
	switch (cmode)
	{
	case CM_VEL:
		drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x98", 128, 160, 255, 255);
		break;
	case CM_PRESS:
		drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x99", 255, 212, 32, 255);
		break;
	case CM_PERS:
		drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x9A", 212, 212, 212, 255);
		break;
	case CM_FIRE:
		drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x9B", 255, 0, 0, 255);
		drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x9C", 255, 255, 64, 255);
		break;
	case CM_BLOB:
		drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xBF", 55, 255, 55, 255);
		break;
	case CM_HEAT:
		drawtext(vid_buf, XRES-27+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xBE", 255, 0, 0, 255);
		drawtext(vid_buf, XRES-27+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xBD", 255, 255, 255, 255);
		break;
	case CM_FANCY:
		drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xC4", 100, 150, 255, 255);
		break;
	case CM_NOTHING:
		drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x00", 100, 150, 255, 255);
		break;
	case CM_CRACK:
		drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xD4", 255, 55, 55, 255);
		drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xD5", 55, 255, 55, 255);
		break;
	case CM_GRAD:
		drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xD3", 255, 50, 255, 255);
		break;
	case CM_LIFE:
		drawtext(vid_buf, XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x00", 255, 50, 255, 255);
		break;
	}
	drawrect(vid_buf, XRES-32+BARSIZE/*478*/, YRES+(MENUSIZE-16), 14, 14, 255, 255, 255, 255);

	// special icons for admin/mods
	if (svf_admin)
	{
		drawtext(vid_buf, XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC9", 232, 127, 35, 255);
		drawtext(vid_buf, XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC7", 255, 255, 255, 255);
		drawtext(vid_buf, XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC8", 255, 255, 255, 255);
	}
	else if (svf_mod)
	{
		drawtext(vid_buf, XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC9", 35, 127, 232, 255);
		drawtext(vid_buf, XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC7", 255, 255, 255, 255);
	}//else if(amd)
	//	drawtext(vid_buf, XRES-45/*465*/, YRES+(MENUSIZE-15), "\x97", 0, 230, 153, 255); Why is this here?
}

void error_ui(pixel *vid_buf, int err, char *txt)
{
	int x0=(XRES-240)/2,y0=YRES/2,b=1,bq,mx,my,textheight;
	char *msg;

	msg = malloc(strlen(txt)+16);
	if (err)
		sprintf(msg, "%03d %s", err, txt);
	else
		sprintf(msg, "%s", txt);
	textheight = textwrapheight(msg, 240);
	y0 -= (52+textheight)/2;
	if (y0<2)
		y0 = 2;
	if (y0+50+textheight>YRES)
		textheight = YRES-50-y0;

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		clearrect(vid_buf, x0-2, y0-2, 244, 52+textheight);
		drawrect(vid_buf, x0, y0, 240, 48+textheight, 192, 192, 192, 255);
		if (err)
			drawtext(vid_buf, x0+8, y0+8, "HTTP error:", 255, 64, 32, 255);
		else
			drawtext(vid_buf, x0+8, y0+8, "Error:", 255, 64, 32, 255);
		drawtextwrap(vid_buf, x0+8, y0+26, 224, msg, 255, 255, 255, 255);
		drawtext(vid_buf, x0+5, y0+textheight+37, "Dismiss", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+textheight+32, 240, 16, 192, 192, 192, 255);
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq && mx>=x0 && mx<x0+240 && my>=y0+textheight+32 && my<=y0+textheight+48)
			break;

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	free(msg);

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
}

char *input_ui(pixel *vid_buf, char *title, char *prompt, char *text, char *shadow)
{
	int xsize = 244;
	int ysize = 90;
	int edity, editx;
	int x0=(XRES-xsize)/2,y0=(YRES-MENUSIZE-ysize)/2,b=1,bq,mx,my;
	ui_edit ed;

	edity = y0+50;
	editx = x0+12;

	ed.x = editx;
	ed.y = edity;
	ed.w = xsize - 20;
	ed.nx = 1;
	ed.def = shadow;
	ed.focus = 0;
	ed.hide = 0;
	ed.cursor = 0;
	ed.multiline = 0;
	strncpy(ed.str, text, 254);

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		clearrect(vid_buf, x0-2, y0-2, xsize+4, ysize+4);
		drawrect(vid_buf, x0, y0, xsize, ysize, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, title, 160, 160, 255, 255);
		drawtext(vid_buf, x0+8, y0+26, prompt, 255, 255, 255, 255);
		
		drawrect(vid_buf, ed.x-4, ed.y-5, ed.w+4, 16, 192, 192, 192, 255);

		ui_edit_draw(vid_buf, &ed);
		ui_edit_process(mx, my, b, &ed);

		drawtext(vid_buf, x0+5, y0+ysize-11, "OK", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+ysize-16, xsize, 16, 192, 192, 192, 255);

		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq && mx>=x0 && mx<x0+xsize && my>=y0+ysize-16 && my<=y0+ysize)
			break;

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
	return mystrdup(ed.str);
}

void info_ui(pixel *vid_buf, char *top, char *txt)
{
	int x0=(XRES-240)/2,y0=(YRES-MENUSIZE)/2,b=1,bq,mx,my;

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
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

		if (b && !bq && mx>=x0 && mx<x0+240 && my>=y0+44 && my<=y0+60)
			break;

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
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
#ifndef RENDERER
	sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
#endif
}

void copytext_ui(pixel *vid_buf, char *top, char *txt, char *copytxt)
{
	int state = 0;
	int i;
	int g = 255;
	int xsize = 244;
	int ysize = 90;
	int x0=(XRES-xsize)/2,y0=(YRES-MENUSIZE-ysize)/2,b=1,bq,mx,my;
	int buttonx = 0;
	int buttony = 0;
	int buttonwidth = 0;
	int buttonheight = 0;
	ui_copytext ed;

	buttonwidth = textwidth(copytxt)+12;
	buttonheight = 10+8;
	buttony = y0+50;
	buttonx = x0+(xsize/2)-(buttonwidth/2);

	ed.x = buttonx;
	ed.y = buttony;
	ed.width = buttonwidth;
	ed.height = buttonheight;
	ed.hover = 0;
	ed.state = 0;
	strcpy(ed.text, copytxt);

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		clearrect(vid_buf, x0-2, y0-2, xsize+4, ysize+4);
		drawrect(vid_buf, x0, y0, xsize, ysize, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, top, 160, 160, 255, 255);
		drawtext(vid_buf, x0+8, y0+26, txt, 255, 255, 255, 255);

		ui_copytext_draw(vid_buf, &ed);
		ui_copytext_process(mx, my, b, bq, &ed);

		drawtext(vid_buf, x0+5, y0+ysize-11, "OK", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+ysize-16, xsize, 16, 192, 192, 192, 255);

		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b && !bq && mx>=x0 && mx<x0+xsize && my>=y0+ysize-16 && my<=y0+ysize)
			break;

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
}

int confirm_ui(pixel *vid_buf, char *top, char *msg, char *btn)
{
	int x0=(XRES-240)/2,y0=(YRES-MENUSIZE)/2,b=1,bq,mx,my;
	int ret = 0;

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
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

		if (b && !bq && mx>=x0+160 && mx<x0+240 && my>=y0+44 && my<=y0+60)
		{
			ret = 1;
			break;
		}
		if (b && !bq && mx>=x0 && mx<x0+160 && my>=y0+44 && my<=y0+60)
			break;

		if (sdl_key==SDLK_RETURN)
		{
			ret = 1;
			break;
		}
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	return ret;
}

void login_ui(pixel *vid_buf)
{
	int x0=(XRES+BARSIZE-192)/2,y0=(YRES+MENUSIZE-80)/2,b=1,bq,mx,my,err;
	ui_edit ed1,ed2;
	char *res;

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	ed1.x = x0+25;
	ed1.y = y0+25;
	ed1.w = 158;
	ed1.nx = 1;
	ed1.def = "[user name]";
	ed1.focus = 1;
	ed1.hide = 0;
	ed1.multiline = 0;
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
	ed2.multiline = 0;
	strcpy(ed2.str, "");

	fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
	while (!sdl_poll())
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

		if (b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+22 && my<y0+36)
			break;
		if (b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+42 && my<y0+46)
			break;
		if (b && !bq && mx>=x0 && mx<x0+192 && my>=y0+64 && my<=y0+80)
			break;

		if (sdl_key==SDLK_RETURN || sdl_key==SDLK_TAB)
		{
			if (!ed1.focus)
				break;
			ed1.focus = 0;
			ed2.focus = 1;
		}
		if (sdl_key==SDLK_ESCAPE)
		{
			if (!ed1.focus && !ed2.focus)
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
	          svf_user, svf_pass, NULL,
	          &err, NULL);
	if (err != 200)
	{
		error_ui(vid_buf, err, http_ret_text(err));
		if (res)
			free(res);
		goto fail;
	}
	if (res && !strncmp(res, "OK ", 3))
	{
		char *s_id,*u_e,*nres,*u_m,*mres;
		s_id = strchr(res+3, ' ');
		if (!s_id)
			goto fail;
		*(s_id++) = 0;

		u_e = strchr(s_id, ' ');
		if (!u_e)
			goto fail;
		*(u_e++) = 0;
			
		u_m = strchr(u_e, ' ');
		if (!u_m) {
			u_m = malloc(1);
			memset(u_m, 0, 1);
		}
		else
			*(u_m++) = 0;

		strcpy(svf_user_id, res+3);
		strcpy(svf_session_id, s_id);
		mres = mystrdup(u_e);
		nres = mystrdup(u_m);

		#ifdef DEBUG
		printf("{%s} {%s} {%s} {%s}\n", svf_user_id, svf_session_id, nres, mres);
		#endif

		if (!strncmp(nres, "ADMIN", 5))
		{
			svf_admin = 1;
			svf_mod = 0;
		}
		else if (!strncmp(nres, "MOD", 3))
		{
			svf_admin = 0;
			svf_mod = 1;
		}
		else
		{
			svf_admin = 0;
			svf_mod = 0;
		}
		svf_messages = atoi(mres);
		free(res);
		svf_login = 1;
		return;
	}
	if (!res)
		res = mystrdup("Unspecified Error");
	error_ui(vid_buf, 0, res);
	free(res);

fail:
	strcpy(svf_user, "");
	strcpy(svf_pass, "");
	strcpy(svf_user_id, "");
	strcpy(svf_session_id, "");
	svf_login = 0;
	svf_own = 0;
	svf_admin = 0;
	svf_mod = 0;
	svf_messages = 0;
}

int stamp_ui(pixel *vid_buf)
{
	int b=1,bq,mx,my,d=-1,i,j,k,x,gx,gy,y,w,h,r=-1,stamp_page=0,per_page=STAMP_X*STAMP_Y,page_count;
	char page_info[64];
	page_count = ceil((float)stamp_count/(float)per_page);

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		clearrect(vid_buf, -1, -1, XRES+1, YRES+MENUSIZE+1);
		k = stamp_page*per_page;//0;
		r = -1;
		d = -1;
		for (j=0; j<GRID_Y; j++)
			for (i=0; i<GRID_X; i++)
			{
				if (stamps[k].name[0] && stamps[k].thumb)
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
					if (mx>=gx+XRES/GRID_S-4 && mx<(gx+XRES/GRID_S)+6 && my>=gy-6 && my<gy+4)
					{
						d = k;
						drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 128, 255);
						drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x86", 255, 48, 32, 255);
					}
					else
					{
						if (mx>=gx && mx<gx+(XRES/GRID_S) && my>=gy && my<gy+(YRES/GRID_S))
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

		if (stamp_page)
		{
			drawtext(vid_buf, 4, YRES+MENUSIZE-14, "\x96", 255, 255, 255, 255);
			drawrect(vid_buf, 1, YRES+MENUSIZE-18, 16, 16, 255, 255, 255, 255);
		}
		if (stamp_page<page_count-1)
		{
			drawtext(vid_buf, XRES-15, YRES+MENUSIZE-14, "\x95", 255, 255, 255, 255);
			drawrect(vid_buf, XRES-18, YRES+MENUSIZE-18, 16, 16, 255, 255, 255, 255);
		}

		if (b==1&&d!=-1)
		{
			if (confirm_ui(vid_buf, "Do you want to delete?", stamps[d].name, "Delete"))
			{
				del_stamp(d);
			}
		}

		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (b==1&&r!=-1)
			break;
		if (b==4&&r!=-1)
		{
			r = -1;
			break;
		}

		if ((b && !bq && mx>=1 && mx<=17 && my>=YRES+MENUSIZE-18 && my<YRES+MENUSIZE-2) || sdl_wheel>0)
		{
			if (stamp_page)
			{
				stamp_page --;
			}
			sdl_wheel = 0;
		}
		if ((b && !bq && mx>=XRES-18 && mx<=XRES-1 && my>=YRES+MENUSIZE-18 && my<YRES+MENUSIZE-2) || sdl_wheel<0)
		{
			if (stamp_page<page_count-1)
			{
				stamp_page ++;
			}
			sdl_wheel = 0;
		}

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
		{
			r = -1;
			break;
		}
	}

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
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
	ed.multiline = 0;
	strcpy(ed.str, "");

	fillrect(vid_buf, -1, -1, XRES, YRES+MENUSIZE, 0, 0, 0, 192);
	while (!sdl_poll())
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
		while (s)
		{
			q = strchr(p, ' ');
			if (!q)
				q = p+strlen(p);
			s = *q;
			*q = 0;
			if (svf_own || svf_admin || svf_mod)
			{
				drawtext(vid_buf, x0+20, y-1, "\x86", 160, 48, 32, 255);
				drawtext(vid_buf, x0+20, y-1, "\x85", 255, 255, 255, 255);
				d = 14;
				if (b && !bq && mx>=x0+18 && mx<x0+32 && my>=y-2 && my<y+12)
				{
					op = "delete";
					tag = mystrdup(p);
				}
			}
			else
				d = 0;
			vp = strlist_find(&vote, p);
			vn = strlist_find(&down, p);
			if ((!vp && !vn && !svf_own) || svf_admin || svf_mod)
			{
				drawtext(vid_buf, x0+d+20, y-1, "\x88", 32, 144, 32, 255);
				drawtext(vid_buf, x0+d+20, y-1, "\x87", 255, 255, 255, 255);
				if (b && !bq && mx>=x0+d+18 && mx<x0+d+32 && my>=y-2 && my<y+12)
				{
					op = "vote";
					tag = mystrdup(p);
					strlist_add(&vote, p);
				}
				drawtext(vid_buf, x0+d+34, y-1, "\x88", 144, 48, 32, 255);
				drawtext(vid_buf, x0+d+34, y-1, "\xA2", 255, 255, 255, 255);
				if (b && !bq && mx>=x0+d+32 && mx<x0+d+46 && my>=y-2 && my<y+12)
				{
					op = "down";
					tag = mystrdup(p);
					strlist_add(&down, p);
				}
			}
			if (vp)
				drawtext(vid_buf, x0+d+48+textwidth(p), y, " - voted!", 48, 192, 48, 255);
			if (vn)
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

		if (b && mx>=x0 && mx<=x0+192 && my>=y0+240 && my<y0+256)
			break;

		if (op)
		{
			d = execute_tagop(vid_buf, op, tag);
			free(tag);
			op = tag = NULL;
			if (d)
				goto finish;
		}

		if (b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+218 && my<y0+232)
		{
			d = execute_tagop(vid_buf, "add", ed.str);
			strcpy(ed.str, "");
			ed.cursor = 0;
			if (d)
				goto finish;
		}

		if (sdl_key==SDLK_RETURN)
		{
			if (!ed.focus)
				break;
			d = execute_tagop(vid_buf, "add", ed.str);
			strcpy(ed.str, "");
			ed.cursor = 0;
			if (d)
				goto finish;
		}
		if (sdl_key==SDLK_ESCAPE)
		{
			if (!ed.focus)
				break;
			strcpy(ed.str, "");
			ed.cursor = 0;
			ed.focus = 0;
		}
	}
	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
	sdl_key = 0;

finish:
	strlist_free(&vote);
}

int save_name_ui(pixel *vid_buf)
{
	int x0=(XRES-420)/2,y0=(YRES-68-YRES/4)/2,b=1,bq,mx,my,ths,idtxtwidth,nd=0;
	void *th;
	pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	ui_edit ed;
	ui_edit ed2;
	ui_checkbox cb;
	ui_copytext ctb;

	th = build_thumb(&ths, 0);

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
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
	ed.multiline = 0;
	strcpy(ed.str, svf_name);

	ed2.x = x0+13;
	ed2.y = y0+45;
	ed2.w = 166;
	ed2.h = 85;
	ed2.nx = 1;
	ed2.def = "[simulation description]";
	ed2.focus = 0;
	ed2.hide = 0;
	ed2.cursor = strlen(svf_description);
	ed2.multiline = 1;
	strcpy(ed2.str, svf_description);

	ctb.x = 0;
	ctb.y = YRES+MENUSIZE-20;
	ctb.width = textwidth(svf_id)+12;
	ctb.height = 10+7;
	ctb.hover = 0;
	ctb.state = 0;
	strcpy(ctb.text, svf_id);


	cb.x = x0+10;
	cb.y = y0+53+YRES/4;
	cb.focus = 0;
	cb.checked = svf_publish;

	fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
	draw_rgba_image(vid_buf, save_to_server_image, 0, 0, 0.7);
	
	memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		drawrect(vid_buf, x0, y0, 420, 90+YRES/4, 192, 192, 192, 255);
		clearrect(vid_buf, x0, y0, 420, 90+YRES/4);
		drawtext(vid_buf, x0+8, y0+8, "New simulation name:", 255, 255, 255, 255);
		drawtext(vid_buf, x0+10, y0+23, "\x82", 192, 192, 192, 255);
		drawrect(vid_buf, x0+8, y0+20, 176, 16, 192, 192, 192, 255);

		drawrect(vid_buf, x0+8, y0+40, 176, 95, 192, 192, 192, 255);

		ui_edit_draw(vid_buf, &ed);
		ui_edit_draw(vid_buf, &ed2);

		drawrect(vid_buf, x0+(205-XRES/3)/2-2+205, y0+30, XRES/3+3, YRES/3+3, 128, 128, 128, 255);
		render_thumb(th, ths, 0, vid_buf, x0+(205-XRES/3)/2+205, y0+32, 3);

		ui_checkbox_draw(vid_buf, &cb);
		drawtext(vid_buf, x0+34, y0+50+YRES/4, "Publish? (Do not publish others'\nworks without permission)", 192, 192, 192, 255);

		drawtext(vid_buf, x0+5, y0+79+YRES/4, "Save simulation", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+74+YRES/4, 192, 16, 192, 192, 192, 255);

		draw_line(vid_buf, x0+192, y0, x0+192, y0+90+YRES/4, 150, 150, 150, XRES+BARSIZE);

		if (svf_id[0])
		{
			//Save ID text and copybox
			idtxtwidth = textwidth("Current save ID: ");
			idtxtwidth += ctb.width;
			ctb.x = textwidth("Current save ID: ")+(XRES+BARSIZE-idtxtwidth)/2;
			drawtext(vid_buf, (XRES+BARSIZE-idtxtwidth)/2, YRES+MENUSIZE-15, "Current save ID: ", 255, 255, 255, 255);

			ui_copytext_draw(vid_buf, &ctb);
			ui_copytext_process(mx, my, b, bq, &ctb);
		}
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

		ui_edit_process(mx, my, b, &ed);
		ui_edit_process(mx, my, b, &ed2);
		ui_checkbox_process(mx, my, b, bq, &cb);

		if ((b && !bq && ((mx>=x0+9 && mx<x0+23 && my>=y0+22 && my<y0+36) ||
		                 (mx>=x0 && mx<x0+192 && my>=y0+74+YRES/4 && my<y0+90+YRES/4)))
			|| sdl_key==SDLK_RETURN)
		{
			if (th) free(th);
			if (!ed.str[0])
				return 0;
			nd = strcmp(svf_name, ed.str) || !svf_own;
			strncpy(svf_name, ed.str, 63);
			svf_name[63] = 0;
			strncpy(svf_description, ed2.str, 254);
			svf_description[254] = 0;
			if (nd)
			{
				strcpy(svf_id, "");
				strcpy(svf_tags, "");
			}
			svf_open = 1;
			svf_own = 1;
			svf_publish = cb.checked;
			svf_filename[0] = 0;
			svf_fileopen = 0;
			free(old_vid);
			return nd+1;
		}
		if (sdl_key==SDLK_ESCAPE)
		{
			if (!ed.focus)
				break;
			ed.focus = 0;
		}
	}
	if (th) free(th);
	free(old_vid);
	return 0;
}

//unused old function, with all the elements drawn at the bottom
/*
void menu_ui(pixel *vid_buf, int i, int *sl, int *sr)
{
	int b=1,bq,mx,my,h,x,y,n=0,height,width,sy,rows=0;
	pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	fillrect(vid_buf, -1, -1, XRES+1, YRES+MENUSIZE, 0, 0, 0, 192);
	memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
	while (!sdl_poll())
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
		if (i==SC_WALL)
		{
			for (n = 122; n<122+UI_WALLCOUNT; n++)
			{
				if (n!=SPC_AIR&&n!=SPC_HEAT&&n!=SPC_COOL&&n!=SPC_VACUUM)
				{
					if (x-26<=60)
					{
						x = XRES-BARSIZE-26;
						y += 19;
					}
					x -= draw_tool_xy(vid_buf, x, y, n, mwalls[n-122].colour)+5;
					if (mx>=x+32 && mx<x+58 && my>=y && my< y+15)
					{
						drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
						h = n;
					}
					else if (n==*sl)
					{
						drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
					}
					else if (n==*sr)
					{
						drawrect(vid_buf, x+30, y-1, 29, 17, 0, 0, 255, 255);
					}
				}
			}
		}
		else if (i==SC_SPECIAL)
		{
			for (n = 122; n<122+UI_WALLCOUNT; n++)
			{
				if (n==SPC_AIR||n==SPC_HEAT||n==SPC_COOL||n==SPC_VACUUM)
				{
					if (x-26<=60)
					{
						x = XRES-BARSIZE-26;
						y += 19;
					}
					x -= draw_tool_xy(vid_buf, x, y, n, mwalls[n-122].colour)+5;
					if (mx>=x+32 && mx<x+58 && my>=y && my< y+15)
					{
						drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
						h = n;
					}
					else if (n==*sl)
					{
						drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
					}
					else if (n==*sr)
					{
						drawrect(vid_buf, x+30, y-1, 29, 17, 0, 0, 255, 255);
					}
				}
			}
			for (n = 0; n<PT_NUM; n++)
			{
				if (ptypes[n].menusection==i&&ptypes[n].menu==1)
				{
					if (x-26<=60)
					{
						x = XRES-BARSIZE-26;
						y += 19;
					}
					x -= draw_tool_xy(vid_buf, x, y, n, ptypes[n].pcolors)+5;
					if (mx>=x+32 && mx<x+58 && my>=y && my< y+15)
					{
						drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
						h = n;
					}
					else if (n==*sl)
					{
						drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
					}
					else if (n==*sr)
					{
						drawrect(vid_buf, x+30, y-1, 29, 17, 0, 0, 255, 255);
					}
				}
			}
		}
		else
		{
			for (n = 0; n<PT_NUM; n++)
			{
				if (ptypes[n].menusection==i&&ptypes[n].menu==1)
				{
					if (x-26<=60)
					{
						x = XRES-BARSIZE-26;
						y += 19;
					}
					x -= draw_tool_xy(vid_buf, x, y, n, ptypes[n].pcolors)+5;
					if (mx>=x+32 && mx<x+58 && my>=y && my< y+15)
					{
						drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
						h = n;
					}
					else if (n==*sl)
					{
						drawrect(vid_buf, x+30, y-1, 29, 17, 255, 0, 0, 255);
					}
					else if (n==*sr)
					{
						drawrect(vid_buf, x+30, y-1, 29, 17, 0, 0, 255, 255);
					}
				}
			}
		}

		if (h==-1)
		{
			drawtext(vid_buf, XRES-textwidth((char *)msections[i].name)-BARSIZE, sy+height+10, (char *)msections[i].name, 255, 255, 255, 255);
		}
		else if (i==SC_WALL||(i==SC_SPECIAL&&h>=122))
		{
			drawtext(vid_buf, XRES-textwidth((char *)mwalls[h-122].descs)-BARSIZE, sy+height+10, (char *)mwalls[h-122].descs, 255, 255, 255, 255);
		}
		else
		{
			drawtext(vid_buf, XRES-textwidth((char *)ptypes[h].descs)-BARSIZE, sy+height+10, (char *)ptypes[h].descs, 255, 255, 255, 255);
		}


		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
		if (!(mx>=(XRES-BARSIZE-width)-7 && my>=sy-10 && my<sy+height+9))
		{
			break;
		}

		if (b==1&&h!=-1)
		{
			*sl = h;
			break;
		}
		if (b==4&&h!=-1)
		{
			*sr = h;
			break;
		}
		//if(b==4&&h!=-1) {
		//	h = -1;
		//	break;
		//}

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
	//drawtext(vid_buf, XRES+2, (12*i)+2, msections[i].icon, 255, 255, 255, 255);
}
*/
//current menu function
void menu_ui_v3(pixel *vid_buf, int i, int *sl, int *sr, int *dae, int b, int bq, int mx, int my)
{
	int h,x,y,n=0,height,width,sy,rows=0,xoff=0,fwidth;
	SEC = SEC2;
	mx /= sdl_scale;
	my /= sdl_scale;
	rows = ceil((float)msections[i].itemcount/16.0f);
	height = (ceil((float)msections[i].itemcount/16.0f)*18);
	width = restrict_flt(msections[i].itemcount*31, 0, 16*31);
	fwidth = msections[i].itemcount*31;
	h = -1;
	x = XRES-BARSIZE-18;
	y = YRES+1;
	sy = y;
	if (i==SC_WALL)//wall menu
	{
		for (n = UI_WALLSTART; n<UI_WALLSTART+UI_WALLCOUNT; n++)
		{
			if (n!=SPC_AIR&&n!=SPC_HEAT&&n!=SPC_COOL&&n!=SPC_VACUUM&&n!=SPC_WIND)
			{
				/*if (x-18<=2)
				{
					x = XRES-BARSIZE-18;
					y += 19;
				}*/
				x -= draw_tool_xy(vid_buf, x, y, n, wtypes[n-UI_WALLSTART].colour)+5;
				if (!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15)
				{
					drawrect(vid_buf, x+30, y-1, 29, 17, 255, 55, 55, 255);
					h = n;
				}
				if (!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15&&(sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL)))
				{
					drawrect(vid_buf, x+30, y-1, 29, 17, 0, 255, 255, 255);
					h = n;
				}
				else if (n==SLALT)
				{
					drawrect(vid_buf, x+30, y-1, 29, 17, 0, 255, 255, 255);
				}
				else if (n==*sl)
				{
					drawrect(vid_buf, x+30, y-1, 29, 17, 255, 55, 55, 255);
				}
				else if (n==*sr)
				{
					drawrect(vid_buf, x+30, y-1, 29, 17, 55, 55, 255, 255);
				}
			}
		}
	}
	else if (i==SC_SPECIAL)//special menu
	{
		for (n = UI_WALLSTART; n<UI_WALLSTART+UI_WALLCOUNT; n++)
		{
			if (n==SPC_AIR||n==SPC_HEAT||n==SPC_COOL||n==SPC_VACUUM||n==SPC_WIND)
			{
				/*if (x-18<=0)
				{
					x = XRES-BARSIZE-18;
					y += 19;
				}*/
				x -= draw_tool_xy(vid_buf, x, y, n, wtypes[n-UI_WALLSTART].colour)+5;
				if (!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15)
				{
					drawrect(vid_buf, x+30, y-1, 29, 17, 255, 55, 55, 255);
					h = n;
				}
				if (!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15&&(sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL)))
				{
					drawrect(vid_buf, x+30, y-1, 29, 17, 0, 255, 255, 255);
					h = n;
				}
				else if (n==SLALT)
				{
					drawrect(vid_buf, x+30, y-1, 29, 17, 0, 255, 255, 255);
				}
				else if (n==*sl)
				{
					drawrect(vid_buf, x+30, y-1, 29, 17, 255, 55, 55, 255);
				}
				else if (n==*sr)
				{
					drawrect(vid_buf, x+30, y-1, 29, 17, 55, 55, 255, 255);
				}
			}
		}
		for (n = 0; n<PT_NUM; n++)
		{
			if (ptypes[n].menusection==i&&ptypes[n].menu==1)
			{
				/*if (x-18<=0)
				{
					x = XRES-BARSIZE-18;
					y += 19;
				}*/
				x -= draw_tool_xy(vid_buf, x, y, n, ptypes[n].pcolors)+5;
				if (!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15)
				{
					drawrect(vid_buf, x+30, y-1, 29, 17, 255, 55, 55, 255);
					h = n;
				}
				if (!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15&&(sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL)))
				{
					drawrect(vid_buf, x+30, y-1, 29, 17, 0, 255, 255, 255);
					h = n;
				}
				else if (n==SLALT)
				{
					drawrect(vid_buf, x+30, y-1, 29, 17, 0, 255, 255, 255);
				}
				else if (n==*sl)
				{
					drawrect(vid_buf, x+30, y-1, 29, 17, 255, 55, 55, 255);
				}
				else if (n==*sr)
				{
					drawrect(vid_buf, x+30, y-1, 29, 17, 55, 55, 255, 255);
				}
			}
		}
	}
	else if(i==SC_LIFE)
	{
		int n2;
		if (fwidth > XRES-BARSIZE) { //fancy scrolling
			float overflow = fwidth-(XRES-BARSIZE), location = ((float)XRES-BARSIZE)/((float)(mx-(XRES-BARSIZE)));
			xoff = (int)(overflow / location);
		}
		for (n2 = 0; n2<NGOLALT; n2++)
		{
			n = PT_LIFE | (n2<<8);
			x -= draw_tool_xy(vid_buf, x-xoff, y, n, gmenu[n2].colour)+5;
			if (!bq && mx>=x+32-xoff && mx<x+58-xoff && my>=y && my< y+15)
			{
				drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 255, 55, 55, 255);
				h = n;
			}
			if (!bq && mx>=x+32-xoff && mx<x+58-xoff && my>=y && my< y+15&&(sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_SHIFT)))
			{
				drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 0, 255, 255, 255);
				h = n;
			}
			else if (n==SLALT)
			{
				drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 0, 255, 255, 255);
			}
			else if (n==*sl)
			{
				drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 255, 55, 55, 255);
			}
			else if (n==*sr)
			{
				drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 55, 55, 255, 255);
			}
		}
	}
	else //all other menus
	{
		if (fwidth > XRES-BARSIZE) { //fancy scrolling
			float overflow = fwidth-(XRES-BARSIZE), location = ((float)XRES-BARSIZE)/((float)(mx-(XRES-BARSIZE)));
			xoff = (int)(overflow / location);
		}
		for (n = 0; n<PT_NUM; n++)
		{
			if (ptypes[n].menusection==i&&ptypes[n].menu==1)
			{
				x -= draw_tool_xy(vid_buf, x-xoff, y, n, ptypes[n].pcolors)+5;
				if (!bq && mx>=x+32-xoff && mx<x+58-xoff && my>=y && my< y+15)
				{
					drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 255, 55, 55, 255);
					h = n;
				}
				if (!bq && mx>=x+32-xoff && mx<x+58-xoff && my>=y && my< y+15&&(sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL)))
				{
					drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 0, 255, 255, 255);
					h = n;
				}
				else if (n==SLALT)
				{
					drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 0, 255, 255, 255);
				}
				else if (n==*sl)
				{
					drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 255, 55, 55, 255);
				}
				else if (n==*sr)
				{
					drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 55, 55, 255, 255);
				}
			}
		}
	}
	if (!bq && mx>=((XRES+BARSIZE)-16) ) //highlight menu section
		if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL))
			if (i>=0&&i<SC_TOTAL)
				SEC = i;

	if (h==-1)
	{
		drawtext(vid_buf, XRES-textwidth((char *)msections[i].name)-BARSIZE, sy-10, (char *)msections[i].name, 255, 255, 255, 255);
	}
	else if (i==SC_WALL||(i==SC_SPECIAL&&h>=UI_WALLSTART))
	{
		drawtext(vid_buf, XRES-textwidth((char *)wtypes[h-UI_WALLSTART].descs)-BARSIZE, sy-10, (char *)wtypes[h-UI_WALLSTART].descs, 255, 255, 255, 255);
	}
	else if (i==SC_LIFE)
	{
		drawtext(vid_buf, XRES-textwidth((char *)gmenu[(h>>8)&0xFF].description)-BARSIZE, sy-10, (char *)gmenu[(h>>8)&0xFF].description, 255, 255, 255, 255);
	}
	else
	{
		drawtext(vid_buf, XRES-textwidth((char *)ptypes[h].descs)-BARSIZE, sy-10, (char *)ptypes[h].descs, 255, 255, 255, 255);
	}
	//these are click events, b=1 is left click, b=4 is right
	//h has the value of the element it is over, and -1 if not over an element
	if (b==1&&h==-1)
	{
		if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL) && SEC>=0)
		{
			SLALT = -1;
			SEC2 = SEC;
		}
	}
	if (b==1&&h!=-1)
	{
		if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL))
		{
			SLALT = h;
			SEC2 = -1;
		}
		else {
			*sl = h;
			*dae = 51;
		}
	}
	if (b==4&&h==-1)
	{
		if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL) && SEC>=0)
		{
			SLALT = -1;
			SEC2 = SEC;
		}
	}
	if (b==4&&h!=-1)
	{
		if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_CTRL))
		{
			SLALT = h;
			SEC2 = -1;
		}
		else {
			*sr = h;
			*dae = 51;
		}
	}
}
int color_menu_ui(pixel *vid_buf, int i, int *cr, int *cg, int *cb, int b, int bq, int mx, int my)
{
	int h,x,y,n=0,height,width,sy,rows=0,xoff=0,fwidth,a,c;
	fwidth = colorsections[i].itemcount*31;
	h = -1;
	x = XRES-BARSIZE-18;
	y = YRES+5;
	sy = y;
	if(i==1) //color menu
	{
		if (fwidth > XRES-BARSIZE) { //fancy scrolling
			float overflow = fwidth-(XRES-BARSIZE), location = ((float)XRES-BARSIZE)/((float)(mx-(XRES-BARSIZE)));
			xoff = (int)(overflow / location);
		}
		for (n = 0; n<7; n++)
		{
				for (a=1; a<15; a++)
				{
					for (c=1; c<27; c++)
					{
						vid_buf[(XRES+BARSIZE)*(y+a)+((x-xoff)+c)] = colorlist[n].colour;
					}
				}
				x -= 26+5;
				if (!bq && mx>=x+32-xoff && mx<x+58-xoff && my>=y && my< y+15)
				{
					drawrect(vid_buf, x+30-xoff, y-1, 29, 17, 255, 55, 55, 255);
					h = n;
				}
		}
	}
	if(h!=-1)
	{
		drawtext(vid_buf, XRES-textwidth((char *)colorlist[h].descs)-BARSIZE, sy-14, (char *)colorlist[h].descs, 255, 255, 255, 255);
	}
	//these are click events, b=1 is left click, b=4 is right
	//h has the value of the element it is over, and -1 if not over an element
	if (b==1 && h!=-1)
	{
		*cr = PIXR(colorlist[h].colour);
		*cg = PIXG(colorlist[h].colour);
		*cb = PIXB(colorlist[h].colour);
		return 1;
	}
	return 0;
}

int sdl_poll(void)
{
	SDL_Event event;
	sdl_key=sdl_wheel=sdl_ascii=0;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
			sdl_key=event.key.keysym.sym;
			sdl_ascii=event.key.keysym.unicode;
			if (event.key.keysym.sym == SDLK_CAPSLOCK)
				sdl_caps = 1;
			if (event.key.keysym.sym=='z')
			{
				sdl_zoom_trig = 1;
			}
			if ( event.key.keysym.sym == SDLK_PLUS)
			{
				sdl_wheel++;
			}
			if ( event.key.keysym.sym == SDLK_MINUS)
			{
				sdl_wheel--;
			}
			//  4
			//1 8 2
			if (event.key.keysym.sym == SDLK_RIGHT)
			{
				player[0] = (int)(player[0])|0x02;  //Go right command
			}
			if (event.key.keysym.sym == SDLK_LEFT)
			{
				player[0] = (int)(player[0])|0x01;  //Go left command
			}
			if (event.key.keysym.sym == SDLK_DOWN && ((int)(player[0])&0x08)!=0x08)
			{
				player[0] = (int)(player[0])|0x08;  //Go left command
			}
			if (event.key.keysym.sym == SDLK_UP && ((int)(player[0])&0x04)!=0x04)
			{
				player[0] = (int)(player[0])|0x04;  //Jump command
			}

			if (event.key.keysym.sym == SDLK_d)
			{
				player2[0] = (int)(player2[0])|0x02;  //Go right command
			}
			if (event.key.keysym.sym == SDLK_a)
			{
				player2[0] = (int)(player2[0])|0x01;  //Go left command
			}
			if (event.key.keysym.sym == SDLK_s && ((int)(player2[0])&0x08)!=0x08)
			{
				player2[0] = (int)(player2[0])|0x08;  //Go left command
			}
			if (event.key.keysym.sym == SDLK_w && ((int)(player2[0])&0x04)!=0x04)
			{
				player2[0] = (int)(player2[0])|0x04;  //Jump command
			}
			break;

		case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_CAPSLOCK)
				sdl_caps = 0;
			if (event.key.keysym.sym == 'z')
				sdl_zoom_trig = 0;
			if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_LEFT)
			{
				player[1] = player[0];  //Saving last movement
				player[0] = (int)(player[0])&12;  //Stop command
			}
			if (event.key.keysym.sym == SDLK_UP)
			{
				player[0] = (int)(player[0])&11;
			}
			if (event.key.keysym.sym == SDLK_DOWN)
			{
				player[0] = (int)(player[0])&7;
			}

			if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_a)
			{
				player2[1] = player2[0];  //Saving last movement
				player2[0] = (int)(player2[0])&12;  //Stop command
			}
			if (event.key.keysym.sym == SDLK_w)
			{
				player2[0] = (int)(player2[0])&11;
			}
			if (event.key.keysym.sym == SDLK_s)
			{
				player2[0] = (int)(player2[0])&7;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_WHEELUP)
				sdl_wheel++;
			if (event.button.button == SDL_BUTTON_WHEELDOWN)
				sdl_wheel--;
			break;
		case SDL_QUIT:
			return 1;
		case SDL_SYSWMEVENT:
#if (defined(LIN32) || defined(LIN64)) && defined(SDL_VIDEO_DRIVER_X11)
			if (event.syswm.msg->subsystem != SDL_SYSWM_X11)
				break;
			sdl_wminfo.info.x11.lock_func();
			XEvent xe = event.syswm.msg->event.xevent;
			if (xe.type==SelectionClear)
			{
				if (clipboard_text!=NULL) {
					free(clipboard_text);
					clipboard_text = NULL;
				}
			}
			else if (xe.type==SelectionRequest)
			{
				XEvent xr;
				xr.xselection.type = SelectionNotify;
				xr.xselection.requestor = xe.xselectionrequest.requestor;
				xr.xselection.selection = xe.xselectionrequest.selection;
				xr.xselection.target = xe.xselectionrequest.target;
				xr.xselection.property = xe.xselectionrequest.property;
				xr.xselection.time = xe.xselectionrequest.time;
				if (xe.xselectionrequest.target==XA_TARGETS)
				{
					// send list of supported formats
					Atom targets[] = {XA_TARGETS, XA_STRING};
					xr.xselection.property = xe.xselectionrequest.property;
					XChangeProperty(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, xe.xselectionrequest.property, XA_ATOM, 32, PropModeReplace, (unsigned char*)targets, (int)(sizeof(targets)/sizeof(Atom)));
				}
				// TODO: Supporting more targets would be nice
				else if (xe.xselectionrequest.target==XA_STRING && clipboard_text)
				{
					XChangeProperty(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, xe.xselectionrequest.property, xe.xselectionrequest.target, 8, PropModeReplace, clipboard_text, strlen(clipboard_text)+1);
				}
				else
				{
					// refuse clipboard request
					xr.xselection.property = None;
				}
				XSendEvent(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, 0, 0, &xr);
			}
			sdl_wminfo.info.x11.unlock_func();
#endif
			continue;
		}
	}
	sdl_mod = SDL_GetModState();
	return 0;
}

void set_cmode(int cm) // sets to given view mode
{
	cmode = cm;
	itc = 51;
	if (cmode==CM_BLOB)
	{
		memset(fire_r, 0, sizeof(fire_r));
		memset(fire_g, 0, sizeof(fire_g));
		memset(fire_b, 0, sizeof(fire_b));
		strcpy(itc_msg, "Blob Display");
	}
	else if (cmode==CM_HEAT)
	{
		strcpy(itc_msg, "Heat Display");
	}
	else if (cmode==CM_FANCY)
	{
		memset(fire_r, 0, sizeof(fire_r));
		memset(fire_g, 0, sizeof(fire_g));
		memset(fire_b, 0, sizeof(fire_b));
		strcpy(itc_msg, "Fancy Display");
	}
	else if (cmode==CM_FIRE)
	{
		memset(fire_r, 0, sizeof(fire_r));
		memset(fire_g, 0, sizeof(fire_g));
		memset(fire_b, 0, sizeof(fire_b));
		strcpy(itc_msg, "Fire Display");
	}
	else if (cmode==CM_PERS)
	{
		memset(fire_bg, 0, XRES*YRES*PIXELSIZE);
		memset(pers_bg, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
		strcpy(itc_msg, "Persistent Display");
	}
	else if (cmode==CM_PRESS)
	{
		strcpy(itc_msg, "Pressure Display");
	}
	else if (cmode==CM_NOTHING)
	{
		strcpy(itc_msg, "Nothing Display");
	}
	else if (cmode==CM_CRACK)
	{
		strcpy(itc_msg, "Alternate Velocity Display");
	}
	else if (cmode==CM_GRAD)
	{
		strcpy(itc_msg, "Heat Gradient Display");
	}
	else if (cmode==CM_LIFE)
	{
		if (DEBUG_MODE) //can only get to Life view in debug mode
		{
			strcpy(itc_msg, "Life Display");
		}
		else
		{
			set_cmode(CM_CRACK);
		}
	}
	else //if no special text given, it will display this.
	{
		strcpy(itc_msg, "Velocity Display");
	}
	save_presets(0);
}

char *download_ui(pixel *vid_buf, char *uri, int *len)
{
	int dstate = 0;
	void *http = http_async_req_start(NULL, uri, NULL, 0, 0);
	int x0=(XRES-240)/2,y0=(YRES-MENUSIZE)/2;
	int done, total, i, ret, zlen, ulen;
	char str[16], *tmp, *res;

	while (!http_async_req_status(http))
	{
		sdl_poll();

		http_async_get_length(http, &total, &done);

		clearrect(vid_buf, x0-2, y0-2, 244, 64);
		drawrect(vid_buf, x0, y0, 240, 60, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, "Please wait", 255, 216, 32, 255);
		drawtext(vid_buf, x0+8, y0+26, "Downloading update...", 255, 255, 255, 255);

		if (total)
		{
			i = (236*done)/total;
			fillrect(vid_buf, x0+1, y0+45, i+1, 14, 255, 216, 32, 255);
			i = (100*done)/total;
			sprintf(str, "%d%%", i);
			if (i<50)
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
	if (ret!=200)
	{
		error_ui(vid_buf, ret, http_ret_text(ret));
		if (tmp)
			free(tmp);
		return NULL;
	}
	if (!tmp)
	{
		error_ui(vid_buf, 0, "Server did not return data");
		return NULL;
	}

	if (zlen<16)
	{
		printf("ZLen is not 16!\n");
		goto corrupt;
	}
	if (tmp[0]!=0x42 || tmp[1]!=0x75 || tmp[2]!=0x54 || tmp[3]!=0x54)
	{
		printf("Tmperr %d, %d, %d, %d\n", tmp[0], tmp[1], tmp[2], tmp[3]);
		goto corrupt;
	}

	ulen  = (unsigned char)tmp[4];
	ulen |= ((unsigned char)tmp[5])<<8;
	ulen |= ((unsigned char)tmp[6])<<16;
	ulen |= ((unsigned char)tmp[7])<<24;

	res = (char *)malloc(ulen);
	if (!res)
	{
		printf("No res!\n");
		goto corrupt;
	}
	dstate = BZ2_bzBuffToBuffDecompress((char *)res, (unsigned *)&ulen, (char *)(tmp+8), zlen-8, 0, 0);
	if (dstate)
	{
		printf("Decompression failure: %d!\n", dstate);
		free(res);
		goto corrupt;
	}

	free(tmp);
	if (len)
		*len = ulen;
	return res;

corrupt:
	error_ui(vid_buf, 0, "Downloaded update is corrupted");
	free(tmp);
	return NULL;
}

int search_ui(pixel *vid_buf)
{
	int nmp=-1,uih=0,nyu,nyd,b=1,bq,mx=0,my=0,mxq=0,myq=0,mmt=0,gi,gj,gx,gy,pos,i,mp,dp,dap,own,last_own=search_own,last_fav=search_fav,page_count=0,last_page=0,last_date=0,j,w,h,st=0,lv;
	int is_p1=0, exp_res=GRID_X*GRID_Y, tp, view_own=0;
	int thumb_drawn[GRID_X*GRID_Y];
	pixel *v_buf = (pixel *)malloc(((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);
	pixel *bthumb_rsdata = NULL;
	float ry;
	time_t http_last_use=HTTP_TIMEOUT;
	ui_edit ed;
	ui_richtext motd;


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

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
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
	ed.multiline = 0;
	strcpy(ed.str, search_expr);

	motd.x = 20;
	motd.y = 33;
	motd.str[0] = 0;

	sdl_wheel = 0;

	while (!sdl_poll())
	{
		uih = 0;
		bq = b;
		mxq = mx;
		myq = my;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		if (mx!=mxq || my!=myq || sdl_wheel || b)
			mmt = 0;
		else if (mmt<TIMEOUT)
			mmt++;

		clearrect(vid_buf, -1, -1, (XRES+BARSIZE)+1, YRES+MENUSIZE+1);

		memcpy(vid_buf, v_buf, ((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);

		drawtext(vid_buf, 11, 13, "Search:", 192, 192, 192, 255);
		if (!last || (!active && strcmp(last, ed.str)))
			drawtext(vid_buf, 51, 11, "\x8E", 192, 160, 32, 255);
		else
			drawtext(vid_buf, 51, 11, "\x8E", 32, 64, 160, 255);
		drawtext(vid_buf, 51, 11, "\x8F", 255, 255, 255, 255);
		drawrect(vid_buf, 48, 8, XRES-182, 16, 192, 192, 192, 255);

		if (!svf_login || search_fav)
		{
			search_own = 0;
			drawrect(vid_buf, XRES-64+16, 8, 56, 16, 96, 96, 96, 255);
			drawtext(vid_buf, XRES-61+16, 11, "\x94", 96, 80, 16, 255);
			drawtext(vid_buf, XRES-61+16, 11, "\x93", 128, 128, 128, 255);
			drawtext(vid_buf, XRES-46+16, 13, "My Own", 128, 128, 128, 255);
		}
		else if (search_own)
		{
			fillrect(vid_buf, XRES-65+16, 7, 58, 18, 255, 255, 255, 255);
			drawtext(vid_buf, XRES-61+16, 11, "\x94", 192, 160, 64, 255);
			drawtext(vid_buf, XRES-61+16, 11, "\x93", 32, 32, 32, 255);
			drawtext(vid_buf, XRES-46+16, 13, "My Own", 0, 0, 0, 255);
		}
		else
		{
			drawrect(vid_buf, XRES-64+16, 8, 56, 16, 192, 192, 192, 255);
			drawtext(vid_buf, XRES-61+16, 11, "\x94", 192, 160, 32, 255);
			drawtext(vid_buf, XRES-61+16, 11, "\x93", 255, 255, 255, 255);
			drawtext(vid_buf, XRES-46+16, 13, "My Own", 255, 255, 255, 255);
		}

		if(!svf_login)
		{
			search_fav = 0;
			drawrect(vid_buf, XRES-134, 8, 16, 16, 192, 192, 192, 255);
			drawtext(vid_buf, XRES-130, 11, "\xCC", 120, 120, 120, 255);
		}
		else if (search_fav)
		{
			fillrect(vid_buf, XRES-134, 7, 18, 18, 255, 255, 255, 255);
			drawtext(vid_buf, XRES-130, 11, "\xCC", 192, 160, 64, 255);
		}
		else
		{
			drawrect(vid_buf, XRES-134, 8, 16, 16, 192, 192, 192, 255);
			drawtext(vid_buf, XRES-130, 11, "\xCC", 192, 160, 32, 255);
		}

		if(search_fav)
		{
			search_date = 0;
			drawrect(vid_buf, XRES-129+16, 8, 60, 16, 96, 96, 96, 255);
			drawtext(vid_buf, XRES-126+16, 11, "\xA9", 44, 48, 32, 255);
			drawtext(vid_buf, XRES-126+16, 11, "\xA8", 32, 44, 32, 255);
			drawtext(vid_buf, XRES-126+16, 11, "\xA7", 128, 128, 128, 255);
			drawtext(vid_buf, XRES-111+16, 13, "By votes", 128, 128, 128, 255);
		}
		else if (search_date)
		{
			fillrect(vid_buf, XRES-130+16, 7, 62, 18, 255, 255, 255, 255);
			drawtext(vid_buf, XRES-126+16, 11, "\xA6", 32, 32, 32, 255);
			drawtext(vid_buf, XRES-111+16, 13, "By date", 0, 0, 0, 255);
		}
		else
		{
			drawrect(vid_buf, XRES-129+16, 8, 60, 16, 192, 192, 192, 255);
			drawtext(vid_buf, XRES-126+16, 11, "\xA9", 144, 48, 32, 255);
			drawtext(vid_buf, XRES-126+16, 11, "\xA8", 32, 144, 32, 255);
			drawtext(vid_buf, XRES-126+16, 11, "\xA7", 255, 255, 255, 255);
			drawtext(vid_buf, XRES-111+16, 13, "By votes", 255, 255, 255, 255);
		}

		if (search_page)
		{
			drawtext(vid_buf, 4, YRES+MENUSIZE-16, "\x96", 255, 255, 255, 255);
			drawrect(vid_buf, 1, YRES+MENUSIZE-20, 16, 16, 255, 255, 255, 255);
		}
		if (page_count > 9)
		{
			drawtext(vid_buf, XRES-15, YRES+MENUSIZE-16, "\x95", 255, 255, 255, 255);
			drawrect(vid_buf, XRES-18, YRES+MENUSIZE-20, 16, 16, 255, 255, 255, 255);
		}

		ui_edit_draw(vid_buf, &ed);

		if ((b && !bq && mx>=1 && mx<=17 && my>=YRES+MENUSIZE-20 && my<YRES+MENUSIZE-4) || sdl_wheel>0)
		{
			if (search_page)
			{
				search_page --;
				lasttime = TIMEOUT;
			}
			sdl_wheel = 0;
			uih = 1;
		}
		if ((b && !bq && mx>=XRES-18 && mx<=XRES-1 && my>=YRES+MENUSIZE-20 && my<YRES+MENUSIZE-4) || sdl_wheel<0)
		{
			if (page_count>exp_res)
			{
				lasttime = TIMEOUT;
				search_page ++;
				page_count = exp_res;
			}
			sdl_wheel = 0;
			uih = 1;
		}

		tp = -1;
		if (is_p1)
		{	
			//Message of the day
			ui_richtext_process(mx, my, b, bq, &motd);
			ui_richtext_draw(vid_buf, &motd);
			//Popular tags
			drawtext(vid_buf, (XRES-textwidth("Popular tags:"))/2, 49, "Popular tags:", 255, 192, 64, 255);
			for (gj=0; gj<((GRID_Y-GRID_P)*YRES)/(GRID_Y*14); gj++)
				for (gi=0; gi<(GRID_X+1); gi++)
				{
					pos = gi+(GRID_X+1)*gj;
					if (pos>TAG_MAX || !tag_names[pos])
						break;
					if (tag_votes[0])
						i = 127+(128*tag_votes[pos])/tag_votes[0];
					else
						i = 192;
					w = textwidth(tag_names[pos]);
					if (w>XRES/(GRID_X+1)-5)
						w = XRES/(GRID_X+1)-5;
					gx = (XRES/(GRID_X+1))*gi;
					gy = gj*13 + 62;
					if (mx>=gx && mx<gx+(XRES/((GRID_X+1)+1)) && my>=gy && my<gy+14)
					{
						j = (i*5)/6;
						tp = pos;
					}
					else
						j = i;
					drawtextmax(vid_buf, gx+(XRES/(GRID_X+1)-w)/2, gy, XRES/(GRID_X+1)-5, tag_names[pos], j, j, i, 255);
				}
		}

		mp = dp = -1;
		dap = -1;
		st = 0;
		for (gj=0; gj<GRID_Y; gj++)
			for (gi=0; gi<GRID_X; gi++)
			{
				if (is_p1)
				{
					pos = gi+GRID_X*(gj-GRID_Y+GRID_P);
					if (pos<0)
						break;
				}
				else
					pos = gi+GRID_X*gj;
				if (!search_ids[pos])
					break;
				gx = ((XRES/GRID_X)*gi) + (XRES/GRID_X-XRES/GRID_S)/2;
				gy = ((((YRES-(MENUSIZE-20))+15)/GRID_Y)*gj) + ((YRES-(MENUSIZE-20))/GRID_Y-(YRES-(MENUSIZE-20))/GRID_S+10)/2 + 18;
				if (textwidth(search_names[pos]) > XRES/GRID_X-10)
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
				if (mx>=gx+XRES/(GRID_S*2)-j/2 && mx<=gx+XRES/(GRID_S*2)+j/2 &&
				        my>=gy+YRES/GRID_S+18 && my<=gy+YRES/GRID_S+31)
				{
					st = 1;
					drawtext(vid_buf, gx+XRES/(GRID_S*2)-j/2, gy+YRES/GRID_S+20, search_owners[pos], 128, 128, 160, 255);
				}
				else
					drawtext(vid_buf, gx+XRES/(GRID_S*2)-j/2, gy+YRES/GRID_S+20, search_owners[pos], 128, 128, 128, 255);
				if (search_thumbs[pos]&&thumb_drawn[pos]==0)
				{
					//render_thumb(search_thumbs[pos], search_thsizes[pos], 1, v_buf, gx, gy, GRID_S);
					int finh, finw;
					pixel *thumb_rsdata = NULL;
					pixel *thumb_imgdata = ptif_unpack(search_thumbs[pos], search_thsizes[pos], &finw, &finh);
					if(thumb_imgdata!=NULL){
						thumb_rsdata = resample_img_nn(thumb_imgdata, finw, finh, XRES/GRID_S, YRES/GRID_S);
						draw_image(v_buf, thumb_rsdata, gx, gy, XRES/GRID_S, YRES/GRID_S, 255);					
						free(thumb_imgdata);
						free(thumb_rsdata);
					}
					thumb_drawn[pos] = 1;
				}
				own = svf_login && (!strcmp(svf_user, search_owners[pos]) || svf_admin || svf_mod);
				if (mx>=gx-2 && mx<=gx+XRES/GRID_S+3 && my>=gy-2 && my<=gy+YRES/GRID_S+30)
					mp = pos;
				if ((own || search_fav) && mx>=gx+XRES/GRID_S-4 && mx<=gx+XRES/GRID_S+6 && my>=gy-6 && my<=gy+4)
				{
					mp = -1;
					dp = pos;
				}
				if (own && !search_dates[pos] && mx>=gx-6 && mx<=gx+4 && my>=gy+YRES/GRID_S-4 && my<=gy+YRES/GRID_S+6)
				{
					mp = -1;
					dap = pos;
				}
				drawrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2, 6, YRES/GRID_S+3, 128, 128, 128, 255);
				fillrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2, 6, 1+(YRES/GRID_S+3)/2, 0, 107, 10, 255);
				fillrect(vid_buf, gx-2+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2), 6, 1+(YRES/GRID_S+3)/2, 107, 10, 0, 255);

				if (mp==pos && !st)
					drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 160, 160, 192, 255);
				else
					drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 128, 255);
				if (own || search_fav)
				{
					if (dp == pos)
						drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x86", 255, 48, 32, 255);
					else
						drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x86", 160, 48, 32, 255);
					drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x85", 255, 255, 255, 255);
				}
				if (!search_publish[pos])
				{
					drawtext(vid_buf, gx-6, gy-6, "\xCD", 255, 255, 255, 255);
					drawtext(vid_buf, gx-6, gy-6, "\xCE", 212, 151, 81, 255);
				}
				if (!search_dates[pos] && own)
				{
					fillrect(vid_buf, gx-5, gy+YRES/GRID_S-3, 7, 8, 255, 255, 255, 255);
					if (dap == pos) {
						drawtext(vid_buf, gx-6, gy+YRES/GRID_S-4, "\xA6", 200, 100, 80, 255);
					} else {
						drawtext(vid_buf, gx-6, gy+YRES/GRID_S-4, "\xA6", 160, 70, 50, 255);
					}
					//drawtext(vid_buf, gx-6, gy-6, "\xCE", 212, 151, 81, 255);
				}
				if (view_own || svf_admin || svf_mod)
				{
					sprintf(ts+1, "%d", search_votes[pos]);
					ts[0] = 0xBB;
					for (j=1; ts[j]; j++)
						ts[j] = 0xBC;
					ts[j-1] = 0xB9;
					ts[j] = 0xBA;
					ts[j+1] = 0;
					w = gx+XRES/GRID_S-2-textwidth(ts);
					h = gy+YRES/GRID_S-11;
					drawtext(vid_buf, w, h, ts, 16, 72, 16, 255);
					for (j=0; ts[j]; j++)
						ts[j] -= 14;
					drawtext(vid_buf, w, h, ts, 192, 192, 192, 255);
					sprintf(ts, "%d", search_votes[pos]);
					for (j=0; ts[j]; j++)
						ts[j] += 127;
					drawtext(vid_buf, w+3, h, ts, 255, 255, 255, 255);
				}
				if (search_scoreup[pos]>0||search_scoredown[pos]>0)
				{
					lv = (search_scoreup[pos]>search_scoredown[pos]?search_scoreup[pos]:search_scoredown[pos]);

					if (((YRES/GRID_S+3)/2)>lv)
					{
						ry = ((float)((YRES/GRID_S+3)/2)/(float)lv);
						if (lv<8)
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

		if (mp!=-1 && mmt>=TIMEOUT/5 && !st)
		{
			gi = mp % GRID_X;
			gj = mp / GRID_X;
			if (is_p1)
				gj += GRID_Y-GRID_P;
			gx = ((XRES/GRID_X)*gi) + (XRES/GRID_X-XRES/GRID_S)/2;
			gy = (((YRES+15)/GRID_Y)*gj) + (YRES/GRID_Y-YRES/GRID_S+10)/2 + 18;
			i = w = textwidth(search_names[mp]);
			h = YRES/GRID_Z+30;
			if (w<XRES/GRID_Z) w=XRES/GRID_Z;
			gx += XRES/(GRID_S*2)-w/2;
			gy += YRES/(GRID_S*2)-h/2;
			if (gx<2) gx=2;
			if (gx+w>=XRES-2) gx=XRES-3-w;
			if (gy<32) gy=32;
			if (gy+h>=YRES+(MENUSIZE-2)) gy=YRES+(MENUSIZE-3)-h;
			clearrect(vid_buf, gx-2, gy-3, w+4, h);
			drawrect(vid_buf, gx-2, gy-3, w+4, h, 160, 160, 192, 255);
			if (search_thumbs[mp]){
				if(mp != nmp && bthumb_rsdata){
					free(bthumb_rsdata);
					bthumb_rsdata = NULL;
				}
				if(!bthumb_rsdata){
					int finh, finw;
					pixel *thumb_imgdata = ptif_unpack(search_thumbs[mp], search_thsizes[mp], &finw, &finh);
					if(thumb_imgdata!=NULL){
						bthumb_rsdata = resample_img(thumb_imgdata, finw, finh, XRES/GRID_Z, YRES/GRID_Z);				
						free(thumb_imgdata);
					}
				}
				draw_image(vid_buf, bthumb_rsdata, gx+(w-(XRES/GRID_Z))/2, gy, XRES/GRID_Z, YRES/GRID_Z, 255);
				nmp = mp;
			}
			drawtext(vid_buf, gx+(w-i)/2, gy+YRES/GRID_Z+4, search_names[mp], 192, 192, 192, 255);
			drawtext(vid_buf, gx+(w-textwidth(search_owners[mp]))/2, gy+YRES/GRID_Z+16, search_owners[mp], 128, 128, 128, 255);
		}

		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		ui_edit_process(mx, my, b, &ed);

		if (sdl_key==SDLK_RETURN)
		{
			if (!last || (!active && (strcmp(last, ed.str) || last_own!=search_own || last_date!=search_date || last_page!=search_page)))
				lasttime = TIMEOUT;
			else if (search_ids[0] && !search_ids[1])
			{
				bq = 0;
				b = 1;
				mp = 0;
			}
		}
		if (sdl_key==SDLK_ESCAPE)
			goto finish;

		if (b && !bq && mx>=XRES-64+16 && mx<=XRES-8+16 && my>=8 && my<=24 && svf_login && !search_fav)
		{
			search_own = !search_own;
			lasttime = TIMEOUT;
		}
		if (b && !bq && mx>=XRES-129+16 && mx<=XRES-65+16 && my>=8 && my<=24 && !search_fav)
		{
			search_date = !search_date;
			lasttime = TIMEOUT;
		}
		if (b && !bq && mx>=XRES-134 && mx<=XRES-134+16 && my>=8 && my<=24 && svf_login)
		{
			search_fav = !search_fav;
			search_own = 0;
			search_date = 0;
			lasttime = TIMEOUT;
		}

		if (b && !bq && dp!=-1)
		{
			if (search_fav){
				if(confirm_ui(vid_buf, "Remove from favourites?", search_names[dp], "Remove")){
					execute_unfav(vid_buf, search_ids[dp]);
					lasttime = TIMEOUT;
					if (last)
					{
						free(last);
						last = NULL;
					}
				}
			} else {
				if (confirm_ui(vid_buf, "Do you want to delete?", search_names[dp], "Delete"))
				{
					execute_delete(vid_buf, search_ids[dp]);
					lasttime = TIMEOUT;
					if (last)
					{
						free(last);
						last = NULL;
					}
				}
			}
		}
		if (b && !bq && dap!=-1)
		{
			sprintf(ed.str, "history:%s", search_ids[dap]);
			lasttime = TIMEOUT;
		}

		if (b && !bq && tp!=-1)
		{
			strncpy(ed.str, tag_names[tp], 255);
			lasttime = TIMEOUT;
		}

		if (b && !bq && mp!=-1 && st)
		{
			sprintf(ed.str, "user:%s", search_owners[mp]);
			lasttime = TIMEOUT;
		}

		if (do_open==1)
		{
			mp = 0;
		}

		if ((b && !bq && mp!=-1 && !st && !uih) || do_open==1)
		{
			if (open_ui(vid_buf, search_ids[mp], search_dates[mp]?search_dates[mp]:NULL)==1) {
				goto finish;
			}
		}

		if (!last)
		{
			search = 1;
		}
		else if (!active && (strcmp(last, ed.str) || last_own!=search_own || last_date!=search_date || last_page!=search_page || last_fav!=search_fav))
		{
			search = 1;
			if (strcmp(last, ed.str) || last_own!=search_own || last_fav!=search_fav || last_date!=search_date)
			{
				search_page = 0;
				page_count = 0;
			}
			free(last);
			last = NULL;
		}
		else
			search = 0;

		if (search && lasttime>=TIMEOUT)
		{
			lasttime = 0;
			last = mystrdup(ed.str);
			last_own = search_own;
			last_date = search_date;
			last_page = search_page;
			last_fav = search_fav;
			active = 1;
			uri = malloc(strlen(last)*3+180+strlen(SERVER)+strlen(svf_user)+20); //Increase "padding" from 80 to 180 to fix the search memory corruption bug
			if (search_own || svf_admin || svf_mod)
				tmp = "&ShowVotes=true";
			else
				tmp = "";
			if (!search_own && !search_date && !search_fav && !*last)
			{
				if (search_page)
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
			if (search_own)
			{
				strcaturl(uri, " user:");
				strcaturl(uri, svf_user);
			}
			if (search_fav)
			{
				strcaturl(uri, " cat:favs");
			}
			if (search_date)
				strcaturl(uri, " sort:date");

			http = http_async_req_start(http, uri, NULL, 0, 1);
			if (svf_login)
			{
				//http_auth_headers(http, svf_user, svf_pass);
				http_auth_headers(http, svf_user_id, NULL, svf_session_id);
			}
			http_last_use = time(NULL);
			free(uri);
		}

		if (active && http_async_req_status(http))
		{
			http_last_use = time(NULL);
			results = http_async_req_stop(http, &status, NULL);
			view_own = last_own;
			if (status == 200)
			{
				page_count = search_results(results, last_own||svf_admin||svf_mod);
				memset(thumb_drawn, 0, sizeof(thumb_drawn));
				memset(v_buf, 0, ((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);
				nmp = -1;
			
				ui_richtext_settext(server_motd, &motd);
				motd.x = (XRES-textwidth(motd.printstr))/2;
			}
			is_p1 = (exp_res < GRID_X*GRID_Y);
			if (results)
				free(results);
			active = 0;
		}

		if (http && !active && (time(NULL)>http_last_use+HTTP_TIMEOUT))
		{
			http_async_req_close(http);
			http = NULL;
		}

		for (i=0; i<IMGCONNS; i++)
		{
			if (img_http[i] && http_async_req_status(img_http[i]))
			{
				thumb = http_async_req_stop(img_http[i], &status, &thlen);
				if (status != 200)
				{
					if (thumb)
						free(thumb);
					thumb = calloc(1,4);
					thlen = 4;
				}
				thumb_cache_add(img_id[i], thumb, thlen);
				for (pos=0; pos<GRID_X*GRID_Y; pos++) {
					if (search_dates[pos]) {
						char *id_d_temp = malloc(strlen(search_ids[pos])+strlen(search_dates[pos])+1);
						strcpy(id_d_temp, search_ids[pos]);
						strappend(id_d_temp, "_");
						strappend(id_d_temp, search_dates[pos]);
						//img_id[i] = mystrdup(id_d_temp);
						if (id_d_temp && !strcmp(id_d_temp, img_id[i])) {
							break;
						}
					} else {
						if (search_ids[pos] && !strcmp(search_ids[pos], img_id[i])) {
							break;
						}
					}
				}
				if (pos<GRID_X*GRID_Y)
				{
					search_thumbs[pos] = thumb;
					search_thsizes[pos] = thlen;
				}
				else
					free(thumb);
				free(img_id[i]);
				img_id[i] = NULL;
			}
			if (!img_id[i])
			{
				for (pos=0; pos<GRID_X*GRID_Y; pos++)
					if (search_ids[pos] && !search_thumbs[pos])
					{
						for (gi=0; gi<IMGCONNS; gi++)
							if (img_id[gi] && !strcmp(search_ids[pos], img_id[gi]))
								break;
						if (gi<IMGCONNS)
							continue;
						break;
					}
				if (pos<GRID_X*GRID_Y)
				{
					if (search_dates[pos]) {
						char *id_d_temp = malloc(strlen(search_ids[pos])+strlen(search_dates[pos])+1);
						uri = malloc(strlen(search_ids[pos])*3+strlen(search_dates[pos])*3+strlen(SERVER)+71);
						strcpy(uri, "http://" SERVER "/Get.api?Op=thumbsmall&ID=");
						strcaturl(uri, search_ids[pos]);
						strappend(uri, "&Date=");
						strcaturl(uri, search_dates[pos]);

						strcpy(id_d_temp, search_ids[pos]);
						strappend(id_d_temp, "_");
						strappend(id_d_temp, search_dates[pos]);
						img_id[i] = mystrdup(id_d_temp);
					} else {
						uri = malloc(strlen(search_ids[pos])*3+strlen(SERVER)+64);
						strcpy(uri, "http://" SERVER "/Get.api?Op=thumbsmall&ID=");
						strcaturl(uri, search_ids[pos]);
						img_id[i] = mystrdup(search_ids[pos]);
					}

					img_http[i] = http_async_req_start(img_http[i], uri, NULL, 0, 1);
					free(uri);
				}
			}
			if (!img_id[i] && img_http[i])
			{
				http_async_req_close(img_http[i]);
				img_http[i] = NULL;
			}
		}

		if (lasttime<TIMEOUT)
			lasttime++;
	}

finish:
	if (last)
		free(last);
	if (http)
		http_async_req_close(http);
	for (i=0; i<IMGCONNS; i++)
		if (img_http[i])
			http_async_req_close(img_http[i]);
			
	if(bthumb_rsdata){
		free(bthumb_rsdata);
		bthumb_rsdata = NULL;
	}

	search_results("", 0);

	strcpy(search_expr, ed.str);

	free(v_buf);
	return 0;
}

int report_ui(pixel* vid_buf, char *save_id)
{
	int b=1,bq,mx,my;
	ui_edit ed;
	ed.x = 209;
	ed.y = 159;
	ed.w = (XRES+BARSIZE-400)-18;
	ed.h = (YRES+MENUSIZE-300)-36;
	ed.nx = 1;
	ed.def = "Report details";
	ed.focus = 0;
	ed.hide = 0;
	ed.multiline = 1;
	ed.cursor = 0;
	strcpy(ed.str, "");

	fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
	while (!sdl_poll()) {
		fillrect(vid_buf, 200, 150, (XRES+BARSIZE-400), (YRES+MENUSIZE-300), 0,0,0, 255);
		drawrect(vid_buf, 200, 150, (XRES+BARSIZE-400), (YRES+MENUSIZE-300), 255, 255, 255, 255);

		drawrect(vid_buf, 205, 155, (XRES+BARSIZE-400)-10, (YRES+MENUSIZE-300)-28, 255, 255, 255, 170);

		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;


		drawrect(vid_buf, 200, (YRES+MENUSIZE-150)-18, 50, 18, 255, 255, 255, 255);
		drawtext(vid_buf, 213, (YRES+MENUSIZE-150)-13, "Cancel", 255, 255, 255, 255);

		drawrect(vid_buf, (XRES+BARSIZE-400)+150, (YRES+MENUSIZE-150)-18, 50, 18, 255, 255, 255, 255);
		drawtext(vid_buf, (XRES+BARSIZE-400)+163, (YRES+MENUSIZE-150)-13, "Report", 255, 255, 255, 255);
		if (mx>(XRES+BARSIZE-400)+150 && my>(YRES+MENUSIZE-150)-18 && mx<(XRES+BARSIZE-400)+200 && my<(YRES+MENUSIZE-150)) {
			fillrect(vid_buf, (XRES+BARSIZE-400)+150, (YRES+MENUSIZE-150)-18, 50, 18, 255, 255, 255, 40);
			if (b) {
				if (execute_report(vid_buf, save_id, ed.str)) {
					info_ui(vid_buf, "Success", "This save has been reported");
					return 1;
				} else {
					return 0;
				}
			}
		}
		if (mx>200 && my>(YRES+MENUSIZE-150)-18 && mx<250 && my<(YRES+MENUSIZE-150)) {
			fillrect(vid_buf, 200, (YRES+MENUSIZE-150)-18, 50, 18, 255, 255, 255, 40);
			if (b)
				return 0;
		}
		ui_edit_draw(vid_buf, &ed);
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		ui_edit_process(mx, my, b, &ed);
	}
	return 0;
}

int open_ui(pixel *vid_buf, char *save_id, char *save_date)
{
	int b=1,bq,mx,my,ca=0,thumb_w,thumb_h,active=0,active_2=0,active_3=0,cc=0,ccy=0,cix=0,hasdrawninfo=0,hasdrawncthumb=0,hasdrawnthumb=0,authoritah=0,myown=0,queue_open=0,data_size=0,full_thumb_data_size=0,retval=0,bc=255,openable=1;
	int nyd,nyu,ry,lv;
	float ryf;

	char *uri, *uri_2, *o_uri, *uri_3;
	void *data = NULL, *info_data, *thumb_data_full;
	save_info *info = calloc(sizeof(save_info), 1);
	void *http = NULL, *http_2 = NULL, *http_3 = NULL;
	int lasttime = TIMEOUT;
	int status, status_2, info_ready = 0, data_ready = 0, thumb_data_ready = 0;
	time_t http_last_use = HTTP_TIMEOUT,  http_last_use_2 = HTTP_TIMEOUT,  http_last_use_3 = HTTP_TIMEOUT;
	pixel *save_pic;// = malloc((XRES/2)*(YRES/2));
	pixel *save_pic_thumb = NULL;
	char *thumb_data = NULL;
	char viewcountbuffer[11];
	int thumb_data_size = 0;
	ui_edit ed;
	ui_copytext ctb;

	viewcountbuffer[0] = 0;
	pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);

	fillrect(vid_buf, 50, 50, XRES+BARSIZE-100, YRES+MENUSIZE-100, 0, 0, 0, 255);
	drawrect(vid_buf, 50, 50, XRES+BARSIZE-100, YRES+MENUSIZE-100, 255, 255, 255, 255);
	drawrect(vid_buf, 50, 50, (XRES/2)+1, (YRES/2)+1, 255, 255, 255, 155);
	drawrect(vid_buf, 50+(XRES/2)+1, 50, XRES+BARSIZE-100-((XRES/2)+1), YRES+MENUSIZE-100, 155, 155, 155, 255);
	drawtext(vid_buf, 50+(XRES/4)-textwidth("Loading...")/2, 50+(YRES/4), "Loading...", 255, 255, 255, 128);

	ed.x = 57+(XRES/2)+1;
	ed.y = YRES+MENUSIZE-118;
	ed.w = XRES+BARSIZE-114-((XRES/2)+1);
	ed.h = 48;
	ed.nx = 1;
	ed.def = "Add comment";
	ed.focus = 1;
	ed.hide = 0;
	ed.multiline = 1;
	ed.cursor = 0;
	strcpy(ed.str, "");

	ctb.x = 100;
	ctb.y = YRES+MENUSIZE-20;
	ctb.width = textwidth(save_id)+12;
	ctb.height = 10+7;
	ctb.hover = 0;
	ctb.state = 0;
	strcpy(ctb.text, save_id);

	memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	//Try to load the thumbnail from the cache
	if(!thumb_cache_find(save_id, &thumb_data, &thumb_data_size)){
		thumb_data = NULL;	
	} else {
		//We found a thumbnail in the cache, we'll draw this one while we wait for the full image to load.
		int finw, finh;
		pixel *thumb_imgdata = ptif_unpack(thumb_data, thumb_data_size, &finw, &finh);
		if(thumb_imgdata!=NULL){
			save_pic_thumb = resample_img(thumb_imgdata, finw, finh, XRES/2, YRES/2);
			//draw_image(vid_buf, save_pic_thumb, 51, 51, XRES/2, YRES/2, 255);	
		}
		free(thumb_imgdata);
		//rescale_img(full_save, imgw, imgh, &thumb_w, &thumb_h, 2);
	}

	//Begin Async loading of data
	if (save_date) {
		// We're loading an historical save
		uri = malloc(strlen(save_id)*3+strlen(save_date)*3+strlen(SERVER)+71);
		strcpy(uri, "http://" SERVER "/Get.api?Op=save&ID=");
		strcaturl(uri, save_id);
		strappend(uri, "&Date=");
		strcaturl(uri, save_date);

		uri_2 = malloc(strlen(save_id)*3+strlen(save_date)*3+strlen(SERVER)+71);
		strcpy(uri_2, "http://" SERVER "/Info.api?ID=");
		strcaturl(uri_2, save_id);
		strappend(uri_2, "&Date=");
		strcaturl(uri_2, save_date);

		uri_3 = malloc(strlen(save_id)*3+strlen(save_date)*3+strlen(SERVER)+71);
		strcpy(uri_3, "http://" SERVER "/Get.api?Op=thumblarge&ID=");
		strcaturl(uri_3, save_id);
		strappend(uri_3, "&Date=");
		strcaturl(uri_3, save_date);
	} else {
		//We're loading a normal save
		uri = malloc(strlen(save_id)*3+strlen(SERVER)+64);
		strcpy(uri, "http://" SERVER "/Get.api?Op=save&ID=");
		strcaturl(uri, save_id);

		uri_2 = malloc(strlen(save_id)*3+strlen(SERVER)+64);
		strcpy(uri_2, "http://" SERVER "/Info.api?ID=");
		strcaturl(uri_2, save_id);

		uri_3 = malloc(strlen(save_id)*3+strlen(SERVER)+64);
		strcpy(uri_3, "http://" SERVER "/Get.api?Op=thumblarge&ID=");
		strcaturl(uri_3, save_id);
	}
	http = http_async_req_start(http, uri, NULL, 0, 1);
	http_2 = http_async_req_start(http_2, uri_2, NULL, 0, 1);
	http_3 = http_async_req_start(http_3, uri_3, NULL, 0, 1);
	if (svf_login)
	{
		http_auth_headers(http, svf_user_id, NULL, svf_session_id);
		http_auth_headers(http_2, svf_user_id, NULL, svf_session_id);
	}
	http_last_use = time(NULL);
	http_last_use_2 = time(NULL);
	http_last_use_3 = time(NULL);
	free(uri);
	free(uri_2);
	free(uri_3);
	active = 1;
	active_2 = 1;
	active_3 = 1;
	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		if (active && http_async_req_status(http))
		{
			int imgh, imgw, nimgh, nimgw;
			http_last_use = time(NULL);
			data = http_async_req_stop(http, &status, &data_size);
			if (status == 200)
			{
				pixel *full_save;
				if (!data||!data_size) {
					error_ui(vid_buf, 0, "Save data is empty (may be corrupt)");
					break;
				}
				full_save = prerender_save(data, data_size, &imgw, &imgh);
				if (full_save!=NULL) {
					//save_pic = rescale_img(full_save, imgw, imgh, &thumb_w, &thumb_h, 2);
					data_ready = 1;
					free(full_save);
				} else {
					error_ui(vid_buf, 0, "Save may be from a newer version");
					break;
				}
			}
			active = 0;
			free(http);
			http = NULL;
		}
		if (active_2 && http_async_req_status(http_2))
		{
			http_last_use_2 = time(NULL);
			info_data = http_async_req_stop(http_2, &status_2, NULL);
			if (status_2 == 200 || !info_data)
			{
				info_ready = info_parse(info_data, info);
				sprintf(viewcountbuffer, "%d", info->downloadcount);
				if (info_ready<=0) {
					error_ui(vid_buf, 0, "Save info not found");
					break;
				}
			}
			if (info_data)
				free(info_data);
			active_2 = 0;
			free(http_2);
			http_2 = NULL;
		}
		if (active_3 && http_async_req_status(http_3))
		{
			int imgh, imgw, nimgh, nimgw;
			http_last_use_3 = time(NULL);
			thumb_data_full = http_async_req_stop(http_3, &status, &full_thumb_data_size);
			if (status == 200)
			{
				pixel *full_thumb;
				if (!thumb_data_full||!full_thumb_data_size) {
					//error_ui(vid_buf, 0, "Save data is empty (may be corrupt)");
					//break;
				} else {
					full_thumb = ptif_unpack(thumb_data_full, full_thumb_data_size, &imgw, &imgh);//prerender_save(data, data_size, &imgw, &imgh);
					if (full_thumb!=NULL) {
						save_pic = resample_img(full_thumb, imgw, imgh, XRES/2, YRES/2);
						thumb_data_ready = 1;
						free(full_thumb);
					}
				}
			}
			if(thumb_data_full)
				free(thumb_data_full);
			active_3 = 0;
			free(http_3);
			http_3 = NULL;
		}
		if (save_pic_thumb!=NULL && !hasdrawncthumb) {
			draw_image(vid_buf, save_pic_thumb, 51, 51, XRES/2, YRES/2, 255);
			free(save_pic_thumb);
			save_pic_thumb = NULL;		
			hasdrawncthumb = 1;
			memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
		}
		if (thumb_data_ready && !hasdrawnthumb) {
			draw_image(vid_buf, save_pic, 51, 51, XRES/2, YRES/2, 255);
			free(save_pic);
			save_pic = NULL;
			hasdrawnthumb = 1;
			memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
		}
		if (info_ready && !hasdrawninfo) {
			//Render all the save information
			cix = drawtext(vid_buf, 60, (YRES/2)+60, info->name, 255, 255, 255, 255);
			cix = drawtext(vid_buf, 60, (YRES/2)+72, "Author:", 255, 255, 255, 155);
			cix = drawtext(vid_buf, cix+4, (YRES/2)+72, info->author, 255, 255, 255, 255);
			cix = drawtext(vid_buf, cix+4, (YRES/2)+72, "Date:", 255, 255, 255, 155);
			cix = drawtext(vid_buf, cix+4, (YRES/2)+72, info->date, 255, 255, 255, 255);
			if(info->downloadcount){
				drawtext(vid_buf, 48+(XRES/2)-textwidth(viewcountbuffer)-textwidth("Views:")-4, (YRES/2)+72, "Views:", 255, 255, 255, 155);
				drawtext(vid_buf, 48+(XRES/2)-textwidth(viewcountbuffer), (YRES/2)+72, viewcountbuffer, 255, 255, 255, 255);
			}
			drawtextwrap(vid_buf, 62, (YRES/2)+86, (XRES/2)-24, info->description, 255, 255, 255, 200);

			//Draw the score bars
			if (info->voteup>0||info->votedown>0)
			{
				lv = (info->voteup>info->votedown)?info->voteup:info->votedown;
				lv = (lv>10)?lv:10;

				if (50>lv)
				{
					ryf = 50.0f/((float)lv);
					//if(lv<8)
					//{
					//	ry =  ry/(8-lv);
					//}
					nyu = info->voteup*ryf;
					nyd = info->votedown*ryf;
				}
				else
				{
					ryf = ((float)lv)/50.0f;
					nyu = info->voteup/ryf;
					nyd = info->votedown/ryf;
				}
				nyu = nyu>50?50:nyu;
				nyd = nyd>50?50:nyd;

				fillrect(vid_buf, 48+(XRES/2)-51, (YRES/2)+53, 52, 6, 0, 107, 10, 255);
				fillrect(vid_buf, 48+(XRES/2)-51, (YRES/2)+59, 52, 6, 107, 10, 0, 255);
				drawrect(vid_buf, 48+(XRES/2)-51, (YRES/2)+53, 52, 6, 128, 128, 128, 255);
				drawrect(vid_buf, 48+(XRES/2)-51, (YRES/2)+59, 52, 6, 128, 128, 128, 255);

				fillrect(vid_buf, 48+(XRES/2)-nyu, (YRES/2)+54, nyu, 4, 57, 187, 57, 255);
				fillrect(vid_buf, 48+(XRES/2)-nyd, (YRES/2)+60, nyd, 4, 187, 57, 57, 255);
			}

			ccy = 0;
			for (cc=0; cc<info->comment_count; cc++) {
				if ((ccy + 72 + ((textwidth(info->comments[cc])/(XRES+BARSIZE-100-((XRES/2)+1)-20)))*12)<(YRES+MENUSIZE-50)) {
					drawtext(vid_buf, 60+(XRES/2)+1, ccy+60, info->commentauthors[cc], 255, 255, 255, 255);
					ccy += 12;
					ccy += drawtextwrap(vid_buf, 60+(XRES/2)+1, ccy+60, XRES+BARSIZE-100-((XRES/2)+1)-20, info->comments[cc], 255, 255, 255, 185);
					ccy += 10;
					if (ccy+52<YRES+MENUSIZE-50) { //Try not to draw off the screen.
						draw_line(vid_buf, 50+(XRES/2)+2, ccy+52, XRES+BARSIZE-50, ccy+52, 100, 100, 100, XRES+BARSIZE);
					}
				}
			}
			hasdrawninfo = 1;
			myown = svf_login && !strcmp(info->author, svf_user);
			authoritah = svf_login && (!strcmp(info->author, svf_user) || svf_admin || svf_mod);
			memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
		}
		if (info_ready && svf_login) {
			//Render the comment box.
			fillrect(vid_buf, 50+(XRES/2)+1, YRES+MENUSIZE-125, XRES+BARSIZE-100-((XRES/2)+1), 75, 0, 0, 0, 255);
			drawrect(vid_buf, 50+(XRES/2)+1, YRES+MENUSIZE-125, XRES+BARSIZE-100-((XRES/2)+1), 75, 200, 200, 200, 255);

			drawrect(vid_buf, 54+(XRES/2)+1, YRES+MENUSIZE-121, XRES+BARSIZE-108-((XRES/2)+1), 48, 255, 255, 255, 200);

			ui_edit_draw(vid_buf, &ed);

			drawrect(vid_buf, XRES+BARSIZE-100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 255);
			drawtext(vid_buf, XRES+BARSIZE-90, YRES+MENUSIZE-63, "Submit", 255, 255, 255, 255);
		}

		//Save ID text and copybox
		cix = textwidth("Save ID: ");
		cix += ctb.width;
		ctb.x = textwidth("Save ID: ")+(XRES+BARSIZE-cix)/2;
		//ctb.x =
		drawtext(vid_buf, (XRES+BARSIZE-cix)/2, YRES+MENUSIZE-15, "Save ID: ", 255, 255, 255, 255);
		ui_copytext_draw(vid_buf, &ctb);
		ui_copytext_process(mx, my, b, bq, &ctb);

		//Open Button
		bc = openable?255:150;
		drawrect(vid_buf, 50, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
		drawtext(vid_buf, 73, YRES+MENUSIZE-63, "Open", 255, 255, 255, bc);
		drawtext(vid_buf, 58, YRES+MENUSIZE-64, "\x81", 255, 255, 255, bc);
		//Fav Button
		bc = svf_login?255:150;
		drawrect(vid_buf, 100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
		if(info->myfav && svf_login){
			drawtext(vid_buf, 122, YRES+MENUSIZE-63, "Unfav.", 255, 230, 230, bc);
		} else {
			drawtext(vid_buf, 122, YRES+MENUSIZE-63, "Fav.", 255, 255, 255, bc);
		}
		drawtext(vid_buf, 107, YRES+MENUSIZE-64, "\xCC", 255, 255, 255, bc);
		//Report Button
		bc = (svf_login && info_ready)?255:150;
		drawrect(vid_buf, 150, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
		drawtext(vid_buf, 168, YRES+MENUSIZE-63, "Report", 255, 255, 255, bc);
		drawtext(vid_buf, 158, YRES+MENUSIZE-63, "!", 255, 255, 255, bc);
		//Delete Button
		bc = authoritah?255:150;
		drawrect(vid_buf, 200, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
		drawtext(vid_buf, 218, YRES+MENUSIZE-63, "Delete", 255, 255, 255, bc);
		drawtext(vid_buf, 206, YRES+MENUSIZE-64, "\xAA", 255, 255, 255, bc);
		//Open in browser button
		bc = 255;
		drawrect(vid_buf, 250, YRES+MENUSIZE-68, 107, 18, 255, 255, 255, bc);
		drawtext(vid_buf, 273, YRES+MENUSIZE-63, "Open in Browser", 255, 255, 255, bc);
		drawtext(vid_buf, 258, YRES+MENUSIZE-64, "\x81", 255, 255, 255, bc);

		//Open Button
		if (sdl_key==SDLK_RETURN && openable) {
			queue_open = 1;
		}
		if (mx > 50 && mx < 50+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && openable && !queue_open) {
			fillrect(vid_buf, 50, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
			if (b && !bq) {
				//Button Clicked
				queue_open = 1;
			}
		}
		//Fav Button
		if (mx > 100 && mx < 100+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && svf_login && !queue_open) {
			fillrect(vid_buf, 100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
			if (b && !bq) {
				//Button Clicked
				if(info->myfav){
					fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
					info_box(vid_buf, "Removing from favourites...");
					execute_unfav(vid_buf, save_id);
					info->myfav = 0;
				} else {
					fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
					info_box(vid_buf, "Adding to favourites...");
					execute_fav(vid_buf, save_id);
					info->myfav = 1;
				}
			}
		}
		//Report Button
		if (mx > 150 && mx < 150+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && svf_login && info_ready && !queue_open) {
			fillrect(vid_buf, 150, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
			if (b && !bq) {
				//Button Clicked
				if (report_ui(vid_buf, save_id)) {
					retval = 0;
					break;
				}
			}
		}
		//Delete Button
		if (mx > 200 && mx < 200+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && (authoritah || myown) && !queue_open) {
			fillrect(vid_buf, 200, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
			if (b && !bq) {
				//Button Clicked
				if (myown || !info->publish) {
					if (confirm_ui(vid_buf, "Are you sure you wish to delete this?", "You will not be able recover it.", "Delete")) {
						fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
						info_box(vid_buf, "Deleting...");
						if (execute_delete(vid_buf, save_id)) {
							retval = 0;
							break;
						}
					}
				} else {
					if (confirm_ui(vid_buf, "Are you sure?", "This save will be removed from the search index.", "Remove")) {
						fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
						info_box(vid_buf, "Removing...");
						if (execute_delete(vid_buf, save_id)) {
							retval = 0;
							break;
						}
					}
				}
			}
		}
		//Open in browser button
		if (mx > 250 && mx < 250+107 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50  && !queue_open) {
			fillrect(vid_buf, 250, YRES+MENUSIZE-68, 107, 18, 255, 255, 255, 40);
			if (b && !bq) {
				//Button Clicked
				o_uri = malloc(7+strlen(SERVER)+41+strlen(save_id)*3);
				strcpy(o_uri, "http://" SERVER "/Browse/View.html?ID=");
				strcaturl(o_uri, save_id);
				open_link(o_uri);
				free(o_uri);
			}
		}
		//Submit Button
		if (mx > XRES+BARSIZE-100 && mx < XRES+BARSIZE-100+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && svf_login && info_ready && !queue_open) {
			fillrect(vid_buf, XRES+BARSIZE-100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
			if (b && !bq) {
				//Button Clicked
				fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
				info_box(vid_buf, "Submitting Comment...");
				execute_submit(vid_buf, save_id, ed.str);
				ed.str[0] = 0;
			}
		}
		//If mouse was clicked outsite of the window bounds.
		if (!(mx>50 && my>50 && mx<XRES+BARSIZE-50 && my<YRES+MENUSIZE-50) && b && !queue_open && my<YRES+MENUSIZE-21) {
			retval = 0;
			break;
		}

		//User opened the save, wait until we've got all the data first...
		if (queue_open) {
			if (info_ready && data_ready) {
				// Do Open!
				status = parse_save(data, data_size, 1, 0, 0, bmap, fvx, fvy, signs, parts, pmap);
				if (!status) {
					if(svf_last)
						free(svf_last);
					svf_last = data;
					data = NULL; //so we don't free it when returning
					svf_lsize = data_size;

					svf_open = 1;
					svf_own = svf_login && !strcmp(info->author, svf_user);
					svf_publish = info->publish && svf_login && !strcmp(info->author, svf_user);

					strcpy(svf_id, save_id);
					strcpy(svf_name, info->name);
					strcpy(svf_description, info->description);
					if (info->tags)
					{
						strncpy(svf_tags, info->tags, 255);
						svf_tags[255] = 0;
					} else {
						svf_tags[0] = 0;
					}
					svf_myvote = info->myvote;
					svf_filename[0] = 0;
					svf_fileopen = 0;
					retval = 1;
					break;
				} else {
					queue_open = 0;

					svf_open = 0;
					svf_filename[0] = 0;
					svf_fileopen = 0;
					svf_publish = 0;
					svf_own = 0;
					svf_myvote = 0;
					svf_id[0] = 0;
					svf_name[0] = 0;
					svf_description[0] = 0;
					svf_tags[0] = 0;
					if (svf_last)
						free(svf_last);
					svf_last = NULL;
					error_ui(vid_buf, 0, "An Error Occurred");
				}
			} else {
				fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 190);
				drawtext(vid_buf, 50+(XRES/4)-textwidth("Loading...")/2, 50+(YRES/4), "Loading...", 255, 255, 255, 128);
			}
		}
		if (!info_ready || !data_ready) {
			info_box(vid_buf, "Loading");
		}
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
		if (info_ready && svf_login) {
			ui_edit_process(mx, my, b, &ed);
		}

		if (sdl_key==SDLK_ESCAPE) {
			retval = 0;
			break;
		}

		if (lasttime<TIMEOUT)
			lasttime++;
	}
	//Prevent those mouse clicks being passed down.
	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
	//Close open connections
	if (http)
		http_async_req_close(http);
	if (http_2)
		http_async_req_close(http_2);
	if (http_3)
		http_async_req_close(http_3);
	info_parse("", info);
	free(info);
	free(old_vid);
	if (data) free(data);
	return retval;
}

int info_parse(char *info_data, save_info *info)
{
	int i,j;
	char *p,*q,*r,*s,*vu,*vd,*pu,*sd;

	if (info->title) free(info->title);
	if (info->name) free(info->name);
	if (info->author) free(info->author);
	if (info->date) free(info->date);
	if (info->description) free(info->description);
	if (info->tags) free(info->tags);
	for (i=0;i<6;i++)
	{
		if (info->comments[i]) free(info->comments[i]);
		if (info->commentauthors[i]) free(info->commentauthors[i]);
	}
	memset(info, 0, sizeof(save_info));

	if (!info_data || !*info_data)
		return 0;

	i = 0;
	j = 0;
	s = NULL;
	do_open = 0;
	while (1)
	{
		if (!*info_data)
			break;
		p = strchr(info_data, '\n');
		if (!p)
			p = info_data + strlen(info_data);
		else
			*(p++) = 0;

		if (!strncmp(info_data, "TITLE ", 6))
		{
			info->title = mystrdup(info_data+6);
			j++;
		}
		else if (!strncmp(info_data, "NAME ", 5))
		{
			info->name = mystrdup(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "AUTHOR ", 7))
		{
			info->author = mystrdup(info_data+7);
			j++;
		}
		else if (!strncmp(info_data, "DATE ", 5))
		{
			info->date = mystrdup(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "DESCRIPTION ", 12))
		{
			info->description = mystrdup(info_data+12);
			j++;
		}
		else if (!strncmp(info_data, "VOTEUP ", 7))
		{
			info->voteup = atoi(info_data+7);
			j++;
		}
		else if (!strncmp(info_data, "VOTEDOWN ", 9))
		{
			info->votedown = atoi(info_data+9);
			j++;
		}
		else if (!strncmp(info_data, "VOTE ", 5))
		{
			info->vote = atoi(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "MYVOTE ", 7))
		{
			info->myvote = atoi(info_data+7);
			j++;
		}
		else if (!strncmp(info_data, "DOWNLOADS ", 10))
		{
			info->downloadcount = atoi(info_data+10);
			j++;
		}
		else if (!strncmp(info_data, "MYFAV ", 6))
		{
			info->myfav = atoi(info_data+6);
			j++;
		}
		else if (!strncmp(info_data, "PUBLISH ", 8))
		{
			info->publish = atoi(info_data+8);
			j++;
		}
		else if (!strncmp(info_data, "TAGS ", 5))
		{
			info->tags = mystrdup(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "COMMENT ", 8))
		{
			if (info->comment_count>=6) {
				info_data = p;
				continue;
			} else {
				q = strchr(info_data+8, ' ');
				*(q++) = 0;
				info->commentauthors[info->comment_count] = mystrdup(info_data+8);
				info->comments[info->comment_count] = mystrdup(q);
				info->comment_count++;
			}
			j++;
		}
		info_data = p;
	}
	if (j>=8) {
		return 1;
	} else {
		return -1;
	}
}

int search_results(char *str, int votes)
{
	int i,j;
	char *p,*q,*r,*s,*vu,*vd,*pu,*sd;

	for (i=0; i<GRID_X*GRID_Y; i++)
	{
		if (search_ids[i])
		{
			free(search_ids[i]);
			search_ids[i] = NULL;
		}
		if (search_names[i])
		{
			free(search_names[i]);
			search_names[i] = NULL;
		}
		if (search_dates[i])
		{
			free(search_dates[i]);
			search_dates[i] = NULL;
		}
		if (search_owners[i])
		{
			free(search_owners[i]);
			search_owners[i] = NULL;
		}
		if (search_thumbs[i])
		{
			free(search_thumbs[i]);
			search_thumbs[i] = NULL;
			search_thsizes[i] = 0;
		}
	}
	for (j=0; j<TAG_MAX; j++)
		if (tag_names[j])
		{
			free(tag_names[j]);
			tag_names[j] = NULL;
		}
	server_motd[0] = 0;

	if (!str || !*str)
		return 0;

	i = 0;
	j = 0;
	s = NULL;
	do_open = 0;
	while (1)
	{
		if (!*str)
			break;
		p = strchr(str, '\n');
		if (!p)
			p = str + strlen(str);
		else
			*(p++) = 0;
		if (!strncmp(str, "OPEN ", 5))
		{
			do_open = 1;
			if (i>=GRID_X*GRID_Y)
				break;
			if (votes)
			{
				pu = strchr(str+5, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				s = strchr(pu, ' ');
				if (!s)
					return i;
				*(s++) = 0;
				vu = strchr(s, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			else
			{
				pu = strchr(str+5, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				vu = strchr(pu, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			if (!q)
				return i;
			*(q++) = 0;
			r = strchr(q, ' ');
			if (!r)
				return i;
			*(r++) = 0;
			search_ids[i] = mystrdup(str+5);

			search_publish[i] = atoi(pu);
			search_scoreup[i] = atoi(vu);
			search_scoredown[i] = atoi(vd);

			search_owners[i] = mystrdup(q);
			search_names[i] = mystrdup(r);

			if (s)
				search_votes[i] = atoi(s);
			thumb_cache_find(str+5, search_thumbs+i, search_thsizes+i);
			i++;
		}
		else if (!strncmp(str, "HISTORY ", 8))
		{
			if (i>=GRID_X*GRID_Y)
				break;
			if (votes)
			{
				sd = strchr(str+8, ' ');
				if (!sd)
					return i;
				*(sd++) = 0;
				pu = strchr(sd, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				s = strchr(pu, ' ');
				if (!s)
					return i;
				*(s++) = 0;
				vu = strchr(s, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			else
			{
				sd = strchr(str+8, ' ');
				if (!sd)
					return i;
				*(sd++) = 0;
				pu = strchr(sd, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				vu = strchr(pu, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			if (!q)
				return i;
			*(q++) = 0;
			r = strchr(q, ' ');
			if (!r)
				return i;
			*(r++) = 0;
			search_ids[i] = mystrdup(str+8);

			search_dates[i] = mystrdup(sd);

			search_publish[i] = atoi(pu);
			search_scoreup[i] = atoi(vu);
			search_scoredown[i] = atoi(vd);

			search_owners[i] = mystrdup(q);
			search_names[i] = mystrdup(r);

			if (s)
				search_votes[i] = atoi(s);
			thumb_cache_find(str+8, search_thumbs+i, search_thsizes+i);
			i++;
		}
		else if (!strncmp(str, "MOTD ", 5))
		{
			memcpy(server_motd, str+5, strlen(str+5));
		}
		else if (!strncmp(str, "TAG ", 4))
		{
			if (j >= TAG_MAX)
			{
				str = p;
				continue;
			}
			q = strchr(str+4, ' ');
			if (!q)
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
			if (i>=GRID_X*GRID_Y)
				break;
			if (votes)
			{
				pu = strchr(str, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				s = strchr(pu, ' ');
				if (!s)
					return i;
				*(s++) = 0;
				vu = strchr(s, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			else
			{
				pu = strchr(str, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				vu = strchr(pu, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			if (!q)
				return i;
			*(q++) = 0;
			r = strchr(q, ' ');
			if (!r)
				return i;
			*(r++) = 0;
			search_ids[i] = mystrdup(str);

			search_publish[i] = atoi(pu);
			search_scoreup[i] = atoi(vu);
			search_scoredown[i] = atoi(vd);

			search_owners[i] = mystrdup(q);
			search_names[i] = mystrdup(r);

			if (s)
				search_votes[i] = atoi(s);
			thumb_cache_find(str, search_thumbs+i, search_thsizes+i);
			i++;
		}
		str = p;
	}
	if (*str)
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
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	free(uri);

	if (status!=200)
	{
		error_ui(vid_buf, status, http_ret_text(status));
		if (result)
			free(result);
		return 1;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(vid_buf, 0, result);
		free(result);
		return 1;
	}

	if (result && result[2])
	{
		strncpy(svf_tags, result+3, 255);
		svf_id[15] = 0;
	}

	if (result)
		free(result);

	return 0;
}

void execute_save(pixel *vid_buf)
{
	int status;
	char *result;

	char *names[] = {"Name","Description", "Data:save.bin", "Thumb:thumb.bin", "Publish", "ID", NULL};
	char *uploadparts[6];
	int plens[6];

	uploadparts[0] = svf_name;
	plens[0] = strlen(svf_name);
	uploadparts[1] = svf_description;
	plens[1] = strlen(svf_description);
	uploadparts[2] = build_save(plens+2, 0, 0, XRES, YRES, bmap, fvx, fvy, signs, parts);
	uploadparts[3] = build_thumb(plens+3, 1);
	uploadparts[4] = (svf_publish==1)?"Public":"Private";
	plens[4] = strlen((svf_publish==1)?"Public":"Private");

	if (svf_id[0])
	{
		uploadparts[5] = svf_id;
		plens[5] = strlen(svf_id);
	}
	else
		names[5] = NULL;

	result = http_multipart_post(
	             "http://" SERVER "/Save.api",
	             names, uploadparts, plens,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (svf_last)
		free(svf_last);
	svf_last = uploadparts[2];
	svf_lsize = plens[2];

	free(uploadparts[3]);

	if (status!=200)
	{
		error_ui(vid_buf, status, http_ret_text(status));
		if (result)
			free(result);
		return;
	}
	if (!result || strncmp(result, "OK", 2))
	{
		if (!result)
			result = mystrdup("Could not save - no reply from server");
		error_ui(vid_buf, 0, result);
		free(result);
		return;
	}

	if (result && result[2])
	{
		strncpy(svf_id, result+3, 15);
		svf_id[15] = 0;
	}

	if (!svf_id[0])
	{
		error_ui(vid_buf, 0, "No ID supplied by server");
		free(result);
		return;
	}

	thumb_cache_inval(svf_id);

	svf_own = 1;
	if (result)
		free(result);
}

int execute_delete(pixel *vid_buf, char *id)
{
	int status;
	char *result;

	char *names[] = {"ID", NULL};
	char *parts[1];

	parts[0] = id;

	result = http_multipart_post(
	             "http://" SERVER "/Delete.api",
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(vid_buf, status, http_ret_text(status));
		if (result)
			free(result);
		return 0;
	}
	if (result && strncmp(result, "INFO: ", 6)==0)
	{
		info_ui(vid_buf, "Info", result+6);
		free(result);
		return 0;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(vid_buf, 0, result);
		free(result);
		return 0;
	}

	if (result)
		free(result);
	return 1;
}

void execute_submit(pixel *vid_buf, char *id, char *message)
{
	int status;
	char *result;

	char *names[] = {"ID", "Message", NULL};
	char *parts[2];

	parts[0] = id;
	parts[1] = message;

	result = http_multipart_post(
	             "http://" SERVER "/Comment.api",
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(vid_buf, status, http_ret_text(status));
		if (result)
			free(result);
		return;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(vid_buf, 0, result);
		free(result);
		return;
	}

	if (result)
		free(result);
}

int execute_report(pixel *vid_buf, char *id, char *reason)
{
	int status;
	char *result;

	char *names[] = {"ID", "Reason", NULL};
	char *parts[2];

	parts[0] = id;
	parts[1] = reason;

	result = http_multipart_post(
	             "http://" SERVER "/Report.api",
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(vid_buf, status, http_ret_text(status));
		if (result)
			free(result);
		return 0;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(vid_buf, 0, result);
		free(result);
		return 0;
	}

	if (result)
		free(result);
	return 1;
}

void execute_fav(pixel *vid_buf, char *id)
{
	int status;
	char *result;

	char *names[] = {"ID", NULL};
	char *parts[1];

	parts[0] = id;

	result = http_multipart_post(
	             "http://" SERVER "/Favourite.api",
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(vid_buf, status, http_ret_text(status));
		if (result)
			free(result);
		return;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(vid_buf, 0, result);
		free(result);
		return;
	}

	if (result)
		free(result);
}

void execute_unfav(pixel *vid_buf, char *id)
{
	int status;
	char *result;

	char *names[] = {"ID", NULL};
	char *parts[1];

	parts[0] = id;

	result = http_multipart_post(
	             "http://" SERVER "/Favourite.api?Action=Remove",
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(vid_buf, status, http_ret_text(status));
		if (result)
			free(result);
		return;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(vid_buf, 0, result);
		free(result);
		return;
	}

	if (result)
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
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(vid_buf, status, http_ret_text(status));
		if (result)
			free(result);
		return 0;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(vid_buf, 0, result);
		free(result);
		return 0;
	}

	if (result)
		free(result);
	return 1;
}
void open_link(char *uri) {
#ifdef WIN32
	ShellExecute(0, "OPEN", uri, NULL, NULL, 0);
#elif MACOSX
	char *cmd = malloc(7+strlen(uri));
	strcpy(cmd, "open ");
	strappend(cmd, uri);
	system(cmd);
#elif LIN32
	char *cmd = malloc(11+strlen(uri));
	strcpy(cmd, "xdg-open ");
	strappend(cmd, uri);
	system(cmd);
#elif LIN64
	char *cmd = malloc(11+strlen(uri));
	strcpy(cmd, "xdg-open ");
	strappend(cmd, uri);
	system(cmd);
#else
	printf("Cannot open browser\n");
#endif
}
struct command_history {
	void *prev_command;
	char *command;
};
typedef struct command_history command_history;
command_history *last_command = NULL;
command_history *last_command2 = NULL;
char *console_ui(pixel *vid_buf,char error[255],char console_more) {
	int mx,my,b,cc,ci = -1,i;
	pixel *old_buf=calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	command_history *currentcommand;
	command_history *currentcommand2;
	ui_edit ed;
	ed.x = 15;
	ed.y = 207;
	ed.w = XRES;
	ed.nx = 1;
	ed.def = "";
	strcpy(ed.str, "");
	ed.focus = 1;
	ed.hide = 0;
	ed.multiline = 0;
	ed.cursor = 0;
	//fillrect(vid_buf, -1, -1, XRES, 220, 0, 0, 0, 190);
	memcpy(old_buf,vid_buf,(XRES+BARSIZE)*YRES*PIXELSIZE);

	fillrect(old_buf, -1, -1, XRES, 220, 0, 0, 0, 190);

	currentcommand2 = malloc(sizeof(command_history));
	memset(currentcommand2, 0, sizeof(command_history));
	currentcommand2->prev_command = last_command2;
	currentcommand2->command = mystrdup(error);
	last_command2 = currentcommand2;

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);//enable keyrepeat for console (is disabled on console close later)
	cc = 0;
	while (cc < 80) {
		fillrect(old_buf, -1, -1+cc, XRES+BARSIZE, 2, 0, 0, 0, 160-(cc*2));
		cc++;
	}
	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;
		ed.focus = 1;

		memcpy(vid_buf,old_buf,(XRES+BARSIZE)*YRES*PIXELSIZE);
		draw_line(vid_buf, 0, 219, XRES+BARSIZE-1, 219, 228, 228, 228, XRES+BARSIZE);
#ifdef PYCONSOLE
		if (pygood)
			i=255;
		else
			i=0;
		if (pyready)
			drawtext(vid_buf, 15, 15, "Welcome to The Powder Toy console v.3 (by cracker64, python by Doxin)", 255, i, i, 255);
		else
			drawtext(vid_buf, 15, 15, "Welcome to The Powder Toy console v.3 (by cracker64, python disabled)", 255, i, i, 255);
#elif defined(LUACONSOLE)
		drawtext(vid_buf, 15, 15, "Welcome to The Powder Toy console v.4 (by cracker64, Lua enabled)", 255, 255, 255, 255);
#else
		drawtext(vid_buf, 15, 15, "Welcome to The Powder Toy console v.3 (by cracker64, python disabled)", 255, 255, 255, 255);
#endif

		cc = 0;
		currentcommand = last_command;
		while (cc < 10)
		{
			if (currentcommand==NULL)
				break;
			drawtext(vid_buf, 15, 175-(cc*12), currentcommand->command, 255, 255, 255, 255);
			if (currentcommand->prev_command!=NULL)
			{
				if (cc<9) {
					currentcommand = currentcommand->prev_command;
				} else if (currentcommand->prev_command!=NULL) {
					free(currentcommand->prev_command);
					currentcommand->prev_command = NULL;
				}
				cc++;
			}
			else
			{
				break;
			}
		}
		cc = 0;
		currentcommand2 = last_command2;
		while (cc < 10)
		{
			if (currentcommand2==NULL)
				break;
			drawtext(vid_buf, 215, 175-(cc*12), currentcommand2->command, 255, 225, 225, 255);
			if (currentcommand2->prev_command!=NULL)
			{
				if (cc<9) {
					currentcommand2 = currentcommand2->prev_command;
				} else if (currentcommand2->prev_command!=NULL) {
					free(currentcommand2->prev_command);
					currentcommand2->prev_command = NULL;
				}
				cc++;
			}
			else
			{
				break;
			}
		}

		//if(error && ed.str[0]=='\0')
		//drawtext(vid_buf, 20, 207, error, 255, 127, 127, 200);
		if (console_more==0)
			drawtext(vid_buf, 5, 207, ">", 255, 255, 255, 240);
		else
			drawtext(vid_buf, 5, 207, "...", 255, 255, 255, 240);

		ui_edit_draw(vid_buf, &ed);
		ui_edit_process(mx, my, b, &ed);
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		if (sdl_key==SDLK_RETURN)
		{
			currentcommand = malloc(sizeof(command_history));
			memset(currentcommand, 0, sizeof(command_history));
			currentcommand->prev_command = last_command;
			currentcommand->command = mystrdup(ed.str);
			last_command = currentcommand;
			free(old_buf);
			SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
			return currentcommand->command;
		}
		if (sdl_key==SDLK_ESCAPE || sdl_key==SDLK_BACKQUOTE)
		{
			console_mode = 0;
			free(old_buf);
			SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
			return NULL;
		}
		if (sdl_key==SDLK_UP || sdl_key==SDLK_DOWN)
		{
			ci += sdl_key==SDLK_UP?1:-1;
			if (ci<-1)
				ci = -1;
			if (ci==-1)
			{
				strcpy(ed.str, "");
				ed.cursor = strlen(ed.str);
			}
			else
			{
				if (last_command!=NULL) {
					currentcommand = last_command;
					for (cc = 0; cc<ci; cc++) {
						if (currentcommand->prev_command==NULL)
							ci = cc;
						else
							currentcommand = currentcommand->prev_command;
					}
					strcpy(ed.str, currentcommand->command);
					ed.cursor = strlen(ed.str);
				}
				else
				{
					ci = -1;
					strcpy(ed.str, "");
					ed.cursor = strlen(ed.str);
				}
			}
		}
	}
	console_mode = 0;
	free(old_buf);
	SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
	return NULL;
}

unsigned int decorations_ui(pixel *vid_buf,int *bsx,int *bsy, unsigned int savedColor)
{//TODO: have the text boxes be editable and update the color.
	int i,ss,hh,vv,cr=127,cg=0,cb=0,b = 0,mx,my,bq = 0,j, lb=0,lx=0,ly=0,lm=0,hidden=0;
	int window_offset_x_left = 2;
	int window_offset_x_right = XRES - 279;
	int window_offset_y = 2;
	int grid_offset_x_left = 5;
	int grid_offset_x_right = XRES - 274;
	int grid_offset_y = 5;
	int onleft_button_offset_x_left = 259;
	int onleft_button_offset_x_right = 4;
	int on_left = 1;
	int grid_offset_x;
	int window_offset_x;
	int onleft_button_offset_x;
	int h = PIXR(savedColor), s = PIXG(savedColor), v = PIXB(savedColor); 
	int th = h, ts = s, tv=v;
	pixel *old_buf=calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	ui_edit box_R;
	ui_edit box_G;
	ui_edit box_B;

	zoom_en = 0;

	box_R.x = 5;
	box_R.y = 5+255+4;
	box_R.w = 30;
	box_R.nx = 1;
	box_R.def = "";
	strcpy(box_R.str, "127");
	box_R.focus = 0;
	box_R.hide = 0;
	box_R.multiline = 0;
	box_R.cursor = 0;

	box_G.x = 40;
	box_G.y = 5+255+4;
	box_G.w = 30;
	box_G.nx = 1;
	box_G.def = "";
	strcpy(box_G.str, "");
	box_G.focus = 0;
	box_G.hide = 0;
	box_G.multiline = 0;
	box_G.cursor = 0;

	box_B.x = 75;
	box_B.y = 5+255+4;
	box_B.w = 30;
	box_B.nx = 1;
	box_B.def = "";
	strcpy(box_B.str, "");
	box_B.focus = 0;
	box_B.hide = 0;
	box_B.multiline = 0;
	box_B.cursor = 0;


	memcpy(old_buf,vid_buf,(XRES+BARSIZE)*YRES*PIXELSIZE);
	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		memcpy(vid_buf,old_buf,(XRES+BARSIZE)*(YRES+MENUSIZE)*PIXELSIZE);
		draw_parts(vid_buf);
		//ui_edit_process(mx, my, b, &box_R);
		//ui_edit_process(mx, my, b, &box_G);
		//ui_edit_process(mx, my, b, &box_B);
		//HSV_to_RGB(h,s,v,&cr,&cg,&cb);
		//if(cr != atoi(box_R.str))
			//RGB_to_HSV(atoi(box_R.str),cg,cb,&h,&s,&v);
		if(on_left==1)
		{
			grid_offset_x = grid_offset_x_left;
			window_offset_x = window_offset_x_left;
			onleft_button_offset_x = onleft_button_offset_x_left;
			box_R.x = 5;
			box_G.x = 40;
			box_B.x = 75;
		}
		else
		{
			grid_offset_x = grid_offset_x_right;
			window_offset_x = window_offset_x_right;
			onleft_button_offset_x = onleft_button_offset_x_right;
			box_R.x = XRES - 254 + 5;
			box_G.x = XRES - 254 + 40;
			box_B.x = XRES - 254 + 75;
		}
		if (zoom_en && mx>=zoom_wx && my>=zoom_wy //change mouse position while it is in a zoom window
		        && mx<(zoom_wx+ZFACTOR*ZSIZE)
		        && my<(zoom_wy+ZFACTOR*ZSIZE))
		{
			mx = (((mx-zoom_wx)/ZFACTOR)+zoom_x);
			my = (((my-zoom_wy)/ZFACTOR)+zoom_y);
		}

		drawrect(vid_buf, -1, -1, XRES+1, YRES+1, 220, 220, 220, 255);
		drawrect(vid_buf, -1, -1, XRES+2, YRES+2, 70, 70, 70, 255);
		drawtext(vid_buf, 2, 388, "Welcome to the decoration editor v.3 (by cracker64) \n\nClicking the current color on the window will move it to the other side. Right click is eraser. ", 255, 255, 255, 255);

		if(!hidden)
		{
			clearrect(vid_buf, window_offset_x, window_offset_y, 2+255+4+10+5, 2+255+20);
			drawrect(vid_buf, window_offset_x, window_offset_y, 2+255+4+10+5, 2+255+20, 255, 255, 255, 255);//window around whole thing

			drawrect(vid_buf, window_offset_x + onleft_button_offset_x +1, window_offset_y +255+6, 12, 12, 255, 255, 255, 255);
			drawrect(vid_buf, window_offset_x + 230, window_offset_y +255+6, 26, 12, 255, 255, 255, 255);
			drawtext(vid_buf, window_offset_x + 232, window_offset_y +255+9, "Clear", 255, 255, 255, 255);
			ui_edit_draw(vid_buf, &box_R);
			ui_edit_draw(vid_buf, &box_G);
			ui_edit_draw(vid_buf, &box_B);

			for(ss=0; ss<=255; ss++)
				for(hh=0;hh<=359;hh++)
				{
					cr = 0;
					cg = 0;
					cb = 0;
					HSV_to_RGB(hh,255-ss,255-ss,&cr,&cg,&cb);
					vid_buf[(ss+grid_offset_y)*(XRES+BARSIZE)+(clamp_flt(hh, 0, 359)+grid_offset_x)] = PIXRGB(cr, cg, cb);
				}
			for(vv=0; vv<=255; vv++)
				for( i=0; i<10; i++)
				{
					cr = 0;
					cg = 0;
					cb = 0;
					HSV_to_RGB(h,s,vv,&cr,&cg,&cb);
					vid_buf[(vv+grid_offset_y)*(XRES+BARSIZE)+(i+grid_offset_x+255+4)] = PIXRGB(cr, cg, cb);
				}
			addpixel(vid_buf,grid_offset_x + clamp_flt(h, 0, 359),grid_offset_y-1,255,255,255,255);
			addpixel(vid_buf,grid_offset_x -1,grid_offset_y+(255-s),255,255,255,255);

			addpixel(vid_buf,grid_offset_x + clamp_flt(th, 0, 359),grid_offset_y-1,100,100,100,255);
			addpixel(vid_buf,grid_offset_x -1,grid_offset_y+(255-ts),100,100,100,255);

			addpixel(vid_buf,grid_offset_x + 255 +3,grid_offset_y+tv,100,100,100,255);
			addpixel(vid_buf,grid_offset_x + 255 +3,grid_offset_y +v,255,255,255,255);

			HSV_to_RGB(h,s,v,&cr,&cg,&cb);
			fillrect(vid_buf, window_offset_x + onleft_button_offset_x +1, window_offset_y +255+6, 12, 12, cr, cg, cb, 255);
		}
		if( color_menu_ui(vid_buf, 1, &cr, &cg, &cb, b, bq, mx, my) )
			RGB_to_HSV(cr,cg,cb,&h,&s,&v);

		HSV_to_RGB(h,s,v,&cr,&cg,&cb);

		sprintf(box_R.str,"%d",cr);
		sprintf(box_G.str,"%d",cg);
		sprintf(box_B.str,"%d",cb);
		fillrect(vid_buf, 250, YRES+4, 40, 15, cr, cg, cb, 255);

		drawrect(vid_buf, 295, YRES+5, 25, 12, 255, 255, 255, 255);
		if(hidden)
			drawtext(vid_buf, 297, YRES+5 +3, "Show", 255, 255, 255, 255);
		else
			drawtext(vid_buf, 297, YRES+5 +3, "Hide", 255, 255, 255, 255);

		if(!lb && !hidden && mx >= window_offset_x && my >= window_offset_y && mx <= window_offset_x+255+4+10+5 && my <= window_offset_y+255+20)//in the main window
		{
			if(mx >= grid_offset_x +255+4 && my >= grid_offset_y && mx <= grid_offset_x+255+4+10 && my <= grid_offset_y+255)
			{
				tv =  my - grid_offset_y;
				if(b)
				{
					v =my - grid_offset_y;
				}
				HSV_to_RGB(h,s,tv,&cr,&cg,&cb);
				//clearrect(vid_buf, window_offset_x + onleft_button_offset_x +1, window_offset_y +255+6,12,12);
				fillrect(vid_buf, window_offset_x + onleft_button_offset_x +1, window_offset_y +255+6, 12, 12, cr, cg, cb, 255);
				sprintf(box_R.str,"%d",cr);
				sprintf(box_G.str,"%d",cg);
				sprintf(box_B.str,"%d",cb);
			}
			if(mx >= grid_offset_x && my >= grid_offset_y && mx <= grid_offset_x+255 && my <= grid_offset_y+255)
			{
				th = mx - grid_offset_x;
				th = (int)( th*359/255 );
				ts = 255 - (my - grid_offset_y);
				if(b)
				{
					h = th;
					s = ts;
				}
				HSV_to_RGB(th,ts,v,&cr,&cg,&cb);
				//clearrect(vid_buf, window_offset_x + onleft_button_offset_x +1, window_offset_y +255+6,12,12);
				fillrect(vid_buf, window_offset_x + onleft_button_offset_x +1, window_offset_y +255+6, 12, 12, cr, cg, cb, 255);
				//sprintf(box_R.def,"%d",cr);
				sprintf(box_R.str,"%d",cr);
				sprintf(box_G.str,"%d",cg);
				sprintf(box_B.str,"%d",cb);
			}
			if(b && !bq && mx >= window_offset_x + onleft_button_offset_x +1 && my >= window_offset_y +255+6 && mx <= window_offset_x + onleft_button_offset_x +13 && my <= window_offset_y +255+5 +13)
			{
				on_left = !on_left;
				lb = 3;//prevent immediate drawing after clicking
			}
			if(b && !bq && mx >= window_offset_x + 230 && my >= window_offset_y +255+6 && mx <= window_offset_x + 230 +26 && my <= window_offset_y +255+5 +13)
				if (confirm_ui(vid_buf, "Reset Decoration Layer", "Do you really want to erase everything?", "Erase") )
				{
					int i;
					for (i=0;i<NPART;i++)
						parts[i].dcolour = 0;
				}
		}
		else if (mx > XRES || my > YRES)
		{
			//click outside normal drawing area
			if (!zoom_en && b && !bq && mx >= 295 && mx <= 295+25 && my >= YRES+5 && my<= YRES+5+12)
				hidden = !hidden;
		}
		else if (sdl_zoom_trig && zoom_en<2)
		{
			mx -= ZSIZE/2;
			my -= ZSIZE/2;
			if (mx<0) mx=0;
			if (my<0) my=0;
			if (mx>XRES-ZSIZE) mx=XRES-ZSIZE;
			if (my>YRES-ZSIZE) my=YRES-ZSIZE;
			zoom_x = mx;
			zoom_y = my;
			zoom_wx = (mx<XRES/2) ? XRES-ZSIZE*ZFACTOR : 0;
			zoom_wy = 0;
			zoom_en = 1;
			hidden = 1;
			if (!b && bq)
				zoom_en = 2;
		}
		else if (b)//there is a click, outside color window
		{
			if (!(b&1))
			{
				cr = 0;
				cg = 0;
				cb = 0;
			}
			if (lb)//mouse is held down
			{
				if (lm == 1)//line tool
				{
					xor_line(lx, ly, mx, my, vid_buf);
				}
				else if (lm == 2)//box tool
				{
					xor_line(lx, ly, lx, my, vid_buf);
					xor_line(lx, my, mx, my, vid_buf);
					xor_line(mx, my, mx, ly, vid_buf);
					xor_line(mx, ly, lx, ly, vid_buf);
				}
				else if(lb!=3)//while mouse is held down, it draws lines between previous and current positions
				{
					line_decorations(lx, ly, mx, my, *bsx, *bsy, cr, cg, cb, b);
					lx = mx;
					ly = my;
				}
			}
			else //first click
			{
				if ((sdl_mod & (KMOD_SHIFT)) && !(sdl_mod & (KMOD_CTRL)))
				{
					lx = mx;
					ly = my;
					lb = b;
					lm = 1;//line
				}
				//start box tool
				else if ((sdl_mod & (KMOD_CTRL)) && !(sdl_mod & (KMOD_SHIFT|KMOD_ALT)))
				{
					lx = mx;
					ly = my;
					lb = b;
					lm = 2;//box
				}
				//sample tool
				else if (((sdl_mod & (KMOD_ALT)) && !(sdl_mod & (KMOD_SHIFT|KMOD_CTRL))) || b==SDL_BUTTON_MIDDLE)
				{
					if (my>=0 && my<YRES && mx>=0 && mx<XRES)
					{
						unsigned int tempcolor = vid_buf[(my)*(XRES+BARSIZE)+(mx)];
						cr = PIXR(tempcolor);
						cg = PIXG(tempcolor);
						cb = PIXB(tempcolor);
						if (cr || cg || cb)
							RGB_to_HSV(cr,cg,cb,&h,&s,&v);
					}
					lx = mx;
					ly = my;
					lb = 0;
					lm = 0;
				}
				else //normal click, draw deco
				{
					create_decorations(mx,my,*bsx,*bsy,cr,cg,cb,b);
					lx = mx;
					ly = my;
					lb = b;
					lm = 0;
				}
			}
		}
		else
		{
			if (!(lb&1))
			{
				cr = 0;
				cg = 0;
				cb = 0;
			}
			if (lb && lm) //lm is box/line tool
			{
				if (lm == 1)//line
					line_decorations(lx, ly, mx, my, *bsx, *bsy, cr, cg, cb, lb);
				else//box
					box_decorations(lx, ly, mx, my, cr, cg, cb, lb);
				lm = 0;
			}
			lb = 0;

		}
		if (zoom_en!=1)
			render_cursor(vid_buf, mx, my, PT_DUST, *bsx, *bsy);

		if (zoom_en)
			render_zoom(vid_buf);

		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		if (sdl_wheel)
		{
			if (sdl_zoom_trig==1)//zoom window change
			{
				ZSIZE += sdl_wheel;
				if (ZSIZE>60)
					ZSIZE = 60;
				if (ZSIZE<2)
					ZSIZE = 2;
				ZFACTOR = 256/ZSIZE;
				sdl_wheel = 0;
			}
			else //change brush size
			{
				if (!(sdl_mod & (KMOD_SHIFT|KMOD_CTRL)))
				{
					*bsx += sdl_wheel;
					*bsy += sdl_wheel;
				}
				else if (sdl_mod & (KMOD_SHIFT) && !(sdl_mod & (KMOD_CTRL)))
				{
					*bsx += sdl_wheel;
				}
				else if (sdl_mod & (KMOD_CTRL) && !(sdl_mod & (KMOD_SHIFT)))
				{
					*bsy += sdl_wheel;
				}
				if (*bsx>1180)
					*bsx = 1180;
				if (*bsx<0)
					*bsx = 0;
				if (*bsy>1180)
					*bsy = 1180;
				if (*bsy<0)
					*bsy = 0;
				sdl_wheel = 0;
				/*if(su >= PT_NUM) {
					if(sl < PT_NUM)
						su = sl;
					if(sr < PT_NUM)
						su = sr;
				}*/
			}
		}
		if (sdl_key==SDLK_LEFTBRACKET) {
			if (sdl_zoom_trig==1)
			{
				ZSIZE -= 1;
				if (ZSIZE>60)
					ZSIZE = 60;
				if (ZSIZE<2)
					ZSIZE = 2;
				ZFACTOR = 256/ZSIZE;
			}
			else
			{
				if (sdl_mod & (KMOD_LALT|KMOD_RALT) && !(sdl_mod & (KMOD_SHIFT|KMOD_CTRL)))
				{
					*bsx -= 1;
					*bsy -= 1;
				}
				else if (sdl_mod & (KMOD_SHIFT) && !(sdl_mod & (KMOD_CTRL)))
				{
					*bsx -= 1;
				}
				else if (sdl_mod & (KMOD_CTRL) && !(sdl_mod & (KMOD_SHIFT)))
				{
					*bsy -= 1;
				}
				else
				{
					*bsx -= ceil((*bsx/5)+0.5f);
					*bsy -= ceil((*bsy/5)+0.5f);
				}
				if (*bsx>1180)
					*bsx = 1180;
				if (*bsy>1180)
					*bsy = 1180;
				if (*bsx<0)
					*bsx = 0;
				if (*bsy<0)
					*bsy = 0;
			}
		}
		if (sdl_key==SDLK_RIGHTBRACKET) {
			if (sdl_zoom_trig==1)
			{
				ZSIZE += 1;
				if (ZSIZE>60)
					ZSIZE = 60;
				if (ZSIZE<2)
					ZSIZE = 2;
				ZFACTOR = 256/ZSIZE;
			}
			else
			{
				if (sdl_mod & (KMOD_LALT|KMOD_RALT) && !(sdl_mod & (KMOD_SHIFT|KMOD_CTRL)))
				{
					*bsx += 1;
					*bsy += 1;
				}
				else if (sdl_mod & (KMOD_SHIFT) && !(sdl_mod & (KMOD_CTRL)))
				{
					*bsx += 1;
				}
				else if (sdl_mod & (KMOD_CTRL) && !(sdl_mod & (KMOD_SHIFT)))
				{
					*bsy += 1;
				}
				else
				{
					*bsx += ceil((*bsx/5)+0.5f);
					*bsy += ceil((*bsy/5)+0.5f);
				}
				if (*bsx>1180)
					*bsx = 1180;
				if (*bsy>1180)
					*bsy = 1180;
				if (*bsx<0)
					*bsx = 0;
				if (*bsy<0)
					*bsy = 0;
			}
		}
		if (sdl_key==SDLK_TAB)
		{
			CURRENT_BRUSH =(CURRENT_BRUSH + 1)%BRUSH_NUM ;
		}

		if (!sdl_zoom_trig && zoom_en==1)
		{
			zoom_en = 0;
		}
		if (sdl_key=='z' && zoom_en==2)
		{
			zoom_en = 1;
			hidden = 1;
		}

		if(sdl_key=='b' || sdl_key==SDLK_ESCAPE)
		{
			free(old_buf);
			return PIXRGB(h,s,v);
		}
	}
	free(old_buf);
	return PIXRGB(h,s,v);
}
struct savelist_e {
	char *filename;
	char *name;
	pixel *image;
	void *next;
	void *prev;
};
typedef struct savelist_e savelist_e;
savelist_e *get_local_saves(char *folder, char *search, int *results_ret)
{
	int index = 0, results = 0;
	savelist_e *new_savelist = NULL;
	savelist_e *current_item = NULL, *new_item = NULL;
	char *fname;
#if defined(WIN32) && !defined(__GNUC__)
	struct _finddata_t current_file;
	intptr_t findfile_handle;
	char *filematch = malloc(strlen(folder)+4);
	sprintf(filematch, "%s%s", folder, "*.*");
	findfile_handle = _findfirst(filematch, &current_file);
	free(filematch);
	if (findfile_handle == -1L)
	{
		*results_ret = 0;
		return NULL;
	}
	do
	{
		fname = current_file.name;
#else
	struct dirent *derp;
	DIR *directory = opendir(folder);
	if(!directory)
	{
		printf("Unable to open directory\n");
		*results_ret = 0;
		return NULL;
	}
	while(derp = readdir(directory))
	{
		fname = derp->d_name;
#endif
		if(strlen(fname)>4)
		{
			char *ext = fname+(strlen(fname)-4);
			if((!strncmp(ext, ".cps", 4) || !strncmp(ext, ".stm", 4)) && (search==NULL || strstr(fname, search)))
			{
				new_item = malloc(sizeof(savelist_e));
				new_item->filename = malloc(strlen(folder)+strlen(fname)+1);
				sprintf(new_item->filename, "%s%s", folder, fname);
				new_item->name = mystrdup(fname);
				new_item->image = NULL;
				new_item->next = NULL;
				if(new_savelist==NULL){
					new_savelist = new_item;
					new_item->prev = NULL;
				} else {
					current_item->next = new_item;
					new_item->prev = current_item;
				}
				current_item = new_item;
				results++;
			}
		}
	}
#if defined(WIN32) && !defined(__GNUC__)
	while (_findnext(findfile_handle, &current_file) == 0);
	_findclose(findfile_handle);
#else
	closedir(directory);
#endif
	*results_ret = results;
	return new_savelist;
}

void free_saveslist(savelist_e *saves)
{
	if(!saves)
		return;
	if(saves->next!=NULL)
		free_saveslist(saves->next);
	if(saves->filename!=NULL)
		free(saves->filename);
	if(saves->name!=NULL)
		free(saves->name);
	if(saves->image!=NULL)
		free(saves->image);
}

int save_filename_ui(pixel *vid_buf)
{
	int xsize = 16+(XRES/3);
	int ysize = 64+(YRES/3);
	float ca = 0;
	int x0=(XRES+BARSIZE-xsize)/2,y0=(YRES+MENUSIZE-ysize)/2,b=1,bq,mx,my;
	int idtxtwidth, nd=0, imgw, imgh, save_size;
	void *save_data;
	pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	pixel *save_tmp;
	pixel *save_data_image;
	pixel *save = calloc((XRES/3)*(YRES/3), PIXELSIZE);
	ui_edit ed;

	save_data = build_save(&save_size, 0, 0, XRES, YRES, bmap, fvx, fvy, signs, parts);
	save_data_image = prerender_save(save_data, save_size, &imgw, &imgh);
	save = resample_img(save_data_image, imgw, imgh, XRES/3, YRES/3);	

	ed.x = x0+11;
	ed.y = y0+25;
	ed.w = xsize-4-16;
	ed.nx = 1;
	ed.def = "[filename]";
	ed.focus = 1;
	ed.hide = 0;
	ed.cursor = 0;
	ed.multiline = 0;
	ed.str[0] = 0;
	
	if(svf_fileopen){
		char * dotloc = NULL;
		strncpy(ed.str, svf_filename, 255);
		if(dotloc = strstr(ed.str, "."))
		{
			dotloc[0] = 0;
		}
		ed.cursor = strlen(ed.str);
	}

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
	draw_rgba_image(vid_buf, save_to_disk_image, 0, 0, 0.7);
	
	memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		clearrect(vid_buf, x0-2, y0-2, xsize+4, ysize+4);
		drawrect(vid_buf, x0, y0, xsize, ysize, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, "Filename:", 255, 255, 255, 255);
		drawrect(vid_buf, x0+8, y0+20, xsize-16, 16, 255, 255, 255, 180);
		draw_image(vid_buf, save, x0+8, y0+40, XRES/3, YRES/3, 255);
		drawrect(vid_buf, x0+8, y0+40, XRES/3, YRES/3, 192, 192, 192, 255);
		
		drawrect(vid_buf, x0, y0+ysize-16, xsize, 16, 192, 192, 192, 255);
		fillrect(vid_buf, x0, y0+ysize-16, xsize, 16, 170, 170, 192, (int)ca);
		drawtext(vid_buf, x0+8, y0+ysize-12, "Save", 255, 255, 255, 255);

		ui_edit_draw(vid_buf, &ed);
		drawtext(vid_buf, x0+12+textwidth(ed.str), y0+25, ".cps", 240, 240, 255, 180);

		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));

		memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

		ui_edit_process(mx, my, b, &ed);
		
		if(mx > x0 && mx < x0+xsize && my > y0+ysize-16 && my < y0+ysize)
		{
			if(b && !bq)
			{
				FILE *f = NULL;
				char *savefname = malloc(strlen(ed.str)+5);
				char *filename = malloc(strlen(LOCAL_SAVE_DIR)+strlen(PATH_SEP)+strlen(ed.str)+5);
				sprintf(filename, "%s%s%s.cps", LOCAL_SAVE_DIR, PATH_SEP, ed.str);
				sprintf(savefname, "%s.cps", ed.str);
			
#ifdef WIN32
				_mkdir(LOCAL_SAVE_DIR);
#else
				mkdir(LOCAL_SAVE_DIR, 0755);
#endif
				f = fopen(filename, "r");
				if(!f || confirm_ui(vid_buf, "A save with the name already exists.", filename, "Overwrite"))
				{
					if(f)
					{
						fclose(f);
						f = NULL;
					}
					f = fopen(filename, "wb");
					if (f)
					{
						fwrite(save_data, save_size, 1, f);
						fclose(f);
						if(svf_fileopen)
						{
							strncpy(svf_filename, savefname, 255);
							svf_fileopen = 1;
						}
						break;
					} else {
						error_ui(vid_buf, 0, "Unable to write to save file.");
					}
				}
				fclose(f);
			}
		}

		if (sdl_key==SDLK_ESCAPE)
		{
			if (!ed.focus)
				break;
			ed.focus = 0;
		}
	}
		
savefin:
	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
	free(save_data_image);
	free(save_data);
	free(old_vid);
	free(save);
	return 0;
}

void catalogue_ui(pixel * vid_buf)
{
	int xsize = 8+(XRES/CATALOGUE_S+8)*CATALOGUE_X;
	int ysize = 48+(YRES/CATALOGUE_S+20)*CATALOGUE_Y;
	int x0=(XRES+BARSIZE-xsize)/2,y0=(YRES+MENUSIZE-ysize)/2,b=1,bq,mx,my;
	int rescount, imageoncycle = 0, currentstart = 0, currentoffset = 0, thidden = 0, cactive = 0;
	int listy = 0, listxc;
	int listx = 0, listyc;
	pixel * vid_buf2;
	float scrollvel, offsetf = 0.0f;
	char savetext[128] = "";
	char * last = mystrdup("");
	savelist_e *saves, *cssave, *csave;
	ui_edit ed;
	
	vid_buf2 = calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	
	ed.w = xsize-16-4;
	ed.x = x0+11;
	ed.y = y0+29;
	ed.multiline = 0;
	ed.def = "[search]";
	ed.focus = 0;
	ed.hide = 0;
	ed.cursor = 0;
	ed.nx = 0;
	strcpy(ed.str, "");

	saves = get_local_saves(LOCAL_SAVE_DIR PATH_SEP, NULL, &rescount);
	cssave = csave = saves;
	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
	
	fillrect(vid_buf, -1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;
		sprintf(savetext, "Found %d save%s", rescount, rescount==1?"":"s");
		clearrect(vid_buf, x0-2, y0-2, xsize+4, ysize+4);
		clearrect(vid_buf2, x0-2, y0-2, xsize+4, ysize+4);
		drawrect(vid_buf, x0, y0, xsize, ysize, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, "Saves", 255, 216, 32, 255);
		drawtext(vid_buf, x0+xsize-8-textwidth(savetext), y0+8, savetext, 255, 216, 32, 255);
		drawrect(vid_buf, x0+8, y0+24, xsize-16, 16, 255, 255, 255, 180);
		if(strcmp(ed.str, last)){
			free(last);
			last = mystrdup(ed.str);
			currentstart = 0;
			if(saves!=NULL) free_saveslist(saves);
			saves = get_local_saves(LOCAL_SAVE_DIR PATH_SEP, last, &rescount);
			cssave = saves;
			scrollvel = 0.0f;
			offsetf = 0.0f;
			thidden = 0;
		}
		//Scrolling
		if (sdl_wheel!=0)
		{
			scrollvel -= (float)sdl_wheel;
			if(scrollvel > 5.0f) scrollvel = 5.0f;
			if(scrollvel < -5.0f) scrollvel = -5.0f;
			sdl_wheel = 0;
		}
		offsetf += scrollvel;
		scrollvel*=0.99f;
		if(offsetf >= (YRES/CATALOGUE_S+20) && rescount)
		{
			if(rescount - thidden > CATALOGUE_X*(CATALOGUE_Y+1))
			{
				int i = 0;
				for(i = 0; i < CATALOGUE_X; i++){
					if(cssave->next==NULL)
						break;
					cssave = cssave->next;
				}
				offsetf -= (YRES/CATALOGUE_S+20);
				thidden += CATALOGUE_X;
			} else {
				offsetf = (YRES/CATALOGUE_S+20);
			}
		} 
		if(offsetf > 0.0f && rescount <= CATALOGUE_X*CATALOGUE_Y && rescount)
		{
			offsetf = 0.0f;
		}
		if(offsetf < 0.0f && rescount)
		{
			if(thidden >= CATALOGUE_X)
			{
				int i = 0;
				for(i = 0; i < CATALOGUE_X; i++){
					if(cssave->prev==NULL)
						break;
					cssave = cssave->prev;
				}
				offsetf += (YRES/CATALOGUE_S+20);
				thidden -= CATALOGUE_X;
			} else {
				offsetf = 0.0f;
			}
		}
		currentoffset = (int)offsetf;
		csave = cssave;
		//Diplay
		if(csave!=NULL && rescount)
		{
			listx = 0;
			listy = 0;
			while(csave!=NULL)
			{
				listxc = x0+8+listx*(XRES/CATALOGUE_S+8);
				listyc = y0+48-currentoffset+listy*(YRES/CATALOGUE_S+20);
				if(listyc > y0+ysize) //Stop when we get to the bottom of the viewable
					break;
				cactive = 0;
				if(my > listyc && my < listyc+YRES/CATALOGUE_S+2 && mx > listxc && mx < listxc+XRES/CATALOGUE_S && my > y0+48 && my < y0+ysize)
				{
					if(b)
					{
						int status, size;
						void *data;
						data = file_load(csave->filename, &size);
						if(data){
							status = parse_save(data, size, 1, 0, 0, bmap, fvx, fvy, signs, parts, pmap);
							if(!status)
							{
								//svf_filename[0] = 0;
								strncpy(svf_filename, csave->name, 255);
								svf_fileopen = 1;
								svf_open = 0;
								svf_publish = 0;
								svf_own = 0;
								svf_myvote = 0;
								svf_id[0] = 0;
								svf_name[0] = 0;
								svf_description[0] = 0;
								svf_tags[0] = 0;
								svf_last = data;
								data = NULL;
								svf_lsize = size;
								goto openfin;
							} else {
								error_ui(vid_buf, 0, "Save data corrupt");
								free(data);
							}
						} else {
							error_ui(vid_buf, 0, "Unable to read save file");
						}
					}
					cactive = 1;
				}
				//Generate an image
				if(csave->image==NULL && !imageoncycle){ //imageoncycle: Don't read/parse more than one image per cycle, makes it more resposive for slower computers
					int imgwidth, imgheight, size;
					pixel *tmpimage = NULL;
					void *data = NULL;
					data = file_load(csave->filename, &size);
					if(data!=NULL){
						tmpimage = prerender_save(data, size, &imgwidth, &imgheight);
						if(tmpimage!=NULL)
						{
							csave->image = resample_img(tmpimage, imgwidth, imgheight, XRES/CATALOGUE_S, YRES/CATALOGUE_S);
							free(tmpimage);
						} else {
							//Blank image, this should default to something else
							csave->image = malloc((XRES/CATALOGUE_S)*(YRES/CATALOGUE_S)*PIXELSIZE);
						}
						free(data);
					} else {
						//Blank image, this should default to something else
						csave->image = malloc((XRES/CATALOGUE_S)*(YRES/CATALOGUE_S)*PIXELSIZE);
					}
					imageoncycle = 1;
				}
				if(csave->image!=NULL)
					draw_image(vid_buf2, csave->image, listxc, listyc, XRES/CATALOGUE_S, YRES/CATALOGUE_S, 255);
				drawrect(vid_buf2, listxc, listyc, XRES/CATALOGUE_S, YRES/CATALOGUE_S, 255, 255, 255, 190);
				if(cactive)
					drawtext(vid_buf2, listxc+((XRES/CATALOGUE_S)/2-textwidth(csave->name)/2), listyc+YRES/CATALOGUE_S+3, csave->name, 255, 255, 255, 255);
				else
					drawtext(vid_buf2, listxc+((XRES/CATALOGUE_S)/2-textwidth(csave->name)/2), listyc+YRES/CATALOGUE_S+3, csave->name, 240, 240, 255, 180);
				csave = csave->next;
				if(++listx==CATALOGUE_X){
					listx = 0;
					listy++;
				}
			}
			imageoncycle = 0;
		} else {
			drawtext(vid_buf2, x0+(xsize/2)-(textwidth("No saves found")/2), y0+(ysize/2)+20, "No saves found", 255, 255, 255, 180);
		}
		ui_edit_draw(vid_buf, &ed);
		ui_edit_process(mx, my, b, &ed);
		//Draw the scrollable area onto the main buffer
		{
			pixel *srctemp = vid_buf2, *desttemp = vid_buf;
			int j = 0;
			for (j = y0+48; j < y0+ysize; j++)
			{
				memcpy(desttemp+j*(XRES+BARSIZE)+x0+1, srctemp+j*(XRES+BARSIZE)+x0+1, (xsize-1)*PIXELSIZE);
				//desttemp+=(XRES+BARSIZE);//*PIXELSIZE;
				//srctemp+=(XRES+BARSIZE);//*PIXELSIZE;
			}
		}
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}
openfin:	
	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	if(saves)
		free_saveslist(saves);
	return;
}

void simulation_ui(pixel * vid_buf)
{
	int xsize = 300;
	int ysize = 164;
	int x0=(XRES-xsize)/2,y0=(YRES-MENUSIZE-ysize)/2,b=1,bq,mx,my;
	int new_scale, new_kiosk;
	ui_checkbox cb;
	ui_checkbox cb2;
	ui_checkbox cb3;
	ui_checkbox cb4;
	ui_checkbox cb5;

	cb.x = x0+xsize-16;		//Heat simulation
	cb.y = y0+23;
	cb.focus = 0;
	cb.checked = !legacy_enable;

	cb2.x = x0+xsize-16;	//Newt. Gravity
	cb2.y = y0+79;
	cb2.focus = 0;
	cb2.checked = ngrav_enable;
	
	cb3.x = x0+xsize-16;	//Large window
	cb3.y = y0+113;
	cb3.focus = 0;
	cb3.checked = (sdl_scale==2)?1:0;
	
	cb4.x = x0+xsize-16;	//Fullscreen
	cb4.y = y0+129;
	cb4.focus = 0;
	cb4.checked = (kiosk_enable==1)?1:0;
	
	cb5.x = x0+xsize-16;	//Ambient heat
	cb5.y = y0+51;
	cb5.focus = 0;
	cb5.checked = aheat_enable;

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		clearrect(vid_buf, x0-2, y0-2, xsize+4, ysize+4);
		drawrect(vid_buf, x0, y0, xsize, ysize, 192, 192, 192, 255);
		drawtext(vid_buf, x0+8, y0+8, "Simulation options", 255, 216, 32, 255);

		drawtext(vid_buf, x0+8, y0+26, "Heat simulation", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12+textwidth("Heat simulation"), y0+26, "Introduced in version 34.", 255, 255, 255, 180);
		drawtext(vid_buf, x0+12, y0+40, "Older saves may behave oddly with this enabled.", 255, 255, 255, 120);
		
		drawtext(vid_buf, x0+8, y0+54, "Ambient heat simulation", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12+textwidth("Ambient heat simulation"), y0+54, "Introduced in version 50.", 255, 255, 255, 180);
		drawtext(vid_buf, x0+12, y0+68, "Older saves may behave oddly with this enabled.", 255, 255, 255, 120);

		drawtext(vid_buf, x0+8, y0+82, "Newtonian gravity", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12+textwidth("Newtonian gravity"), y0+82, "Introduced in version 48.", 255, 255, 255, 180);
		drawtext(vid_buf, x0+12, y0+96, "May also cause slow performance on older computers", 255, 255, 255, 120);
		
		draw_line(vid_buf, x0, y0+110, x0+xsize, y0+110, 150, 150, 150, XRES+BARSIZE);
		
		drawtext(vid_buf, x0+8, y0+116, "Large window", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12+textwidth("Large window"), y0+116, "Double window size for small screens", 255, 255, 255, 180);
		
		drawtext(vid_buf, x0+8, y0+132, "Fullscreen", 255, 255, 255, 255);
		drawtext(vid_buf, x0+12+textwidth("Fullscreen"), y0+132, "Fill the entire screen", 255, 255, 255, 180);

		//TODO: Options for Air and Normal gravity
		//Maybe save/load defaults too.

		drawtext(vid_buf, x0+5, y0+ysize-11, "OK", 255, 255, 255, 255);
		drawrect(vid_buf, x0, y0+ysize-16, xsize, 16, 192, 192, 192, 255);

		ui_checkbox_draw(vid_buf, &cb);
		ui_checkbox_draw(vid_buf, &cb2);
		ui_checkbox_draw(vid_buf, &cb3);
		ui_checkbox_draw(vid_buf, &cb4);
		ui_checkbox_draw(vid_buf, &cb5);
		sdl_blit(0, 0, (XRES+BARSIZE), YRES+MENUSIZE, vid_buf, (XRES+BARSIZE));
		ui_checkbox_process(mx, my, b, bq, &cb);
		ui_checkbox_process(mx, my, b, bq, &cb2);
		ui_checkbox_process(mx, my, b, bq, &cb3);
		ui_checkbox_process(mx, my, b, bq, &cb4);
		ui_checkbox_process(mx, my, b, bq, &cb5);

		if (b && !bq && mx>=x0 && mx<x0+xsize && my>=y0+ysize-16 && my<=y0+ysize)
			break;

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	legacy_enable = !cb.checked;
	aheat_enable = cb5.checked;
	new_scale = (cb3.checked)?2:1;
	new_kiosk = (cb4.checked)?1:0;
	if(new_scale!=sdl_scale || new_kiosk!=kiosk_enable)
	{
		if (!set_scale(new_scale, new_kiosk))
			error_ui(vid_buf, 0, "Could not change display options");
	}
	if(ngrav_enable != cb2.checked)
	{
		if(cb2.checked)
			start_grav_async();
		else
			stop_grav_async();
	}

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
}
