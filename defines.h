#ifndef DEFINE_H
#define DEFINE_H

#ifdef WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

#define SAVE_VERSION 41
#define MINOR_VERSION 3
#define IDENT_VERSION "S" //Change this if you're not Simon! It should be a single letter.

#define SERVER "powdertoy.co.uk"

#undef PLOSS

#define THUMB_CACHE_SIZE 256

#define IMGCONNS 3
#define TIMEOUT 100
#define HTTP_TIMEOUT 10

#define MENUSIZE 40
#define BARSIZE 14
#define XRES	612
#define YRES	384
#define NPART XRES*YRES

#define MAXSIGNS 16
#define TAG_MAX 256

#define ZSIZE_D	16
#define ZFACTOR_D	8
static unsigned char ZFACTOR = 256/ZSIZE_D;
static unsigned char ZSIZE = ZSIZE_D;

#define CELL    4
#define ISTP    (CELL/2)
#define CFDS	(4.0f/CELL)

#define TSTEPP 0.3f
#define TSTEPV 0.4f
#define VADV 0.3f
#define VLOSS 0.999f
#define PLOSS 0.9999f

#define GRID_X 5
#define GRID_Y 4
#define GRID_P 3
#define GRID_S 6
#define GRID_Z 3

#define STAMP_X 4
#define STAMP_Y 4
#define STAMP_MAX 120

#ifdef PIX16
typedef unsigned short pixel;
#else
typedef unsigned int pixel;
#endif

typedef unsigned char uint8;

extern int legacy_enable;

extern int sys_pause;
extern int framerender;

extern int mousex, mousey;
extern int death;

struct sign
{
    int x,y,ju;
    char text[256];
};
typedef struct sign sign;

struct stamp
{
    char name[11];
    pixel *thumb;
    int thumb_w, thumb_h, dodelete;
};
typedef struct stamp stamp;

extern sign signs[MAXSIGNS];
extern stamp stamps[STAMP_MAX];
extern int stamp_count;
extern int itc;
extern char itc_msg[64];

extern int do_open;
extern int sys_pause;
extern int legacy_enable; //Used to disable new features such as heat, will be set by commandline or save.
extern int death, framerender;

extern unsigned char last_major, last_minor, update_flag;

extern char http_proxy_string[256];

#endif