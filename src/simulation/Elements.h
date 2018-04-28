#ifndef ELEMENTS_H_
#define ELEMENTS_H_

//#include "Config.h"
//#include "Simulation.h"

#define R_TEMP 22
#define MAX_TEMP 9999
#define MIN_TEMP 0
#define O_MAX_TEMP 3500
#define O_MIN_TEMP -273

#define TYPE_PART			0x00001  //1 Powders
#define TYPE_LIQUID			0x00002  //2 Liquids
#define TYPE_SOLID			0x00004  //4 Solids
#define TYPE_GAS			0x00008  //8 Gases (Includes plasma)
#define TYPE_ENERGY			0x00010  //16 Energy (Thunder, Light, Neutrons etc.)
#define STATE_FLAGS			0x0001F
#define PROP_CONDUCTS		0x00020  //32 Conducts electricity
#define PROP_BLACK			0x00040  //64 Absorbs Photons (not currently implemented or used, a photwl attribute might be better)
#define PROP_NEUTPENETRATE	0x00080  //128 Penetrated by neutrons
#define PROP_NEUTABSORB		0x00100  //256 Absorbs neutrons, reflect is default
#define PROP_NEUTPASS		0x00200  //512 Neutrons pass through, such as with glass
#define PROP_DEADLY			0x00400  //1024 Is deadly for stickman
#define PROP_HOT_GLOW		0x00800  //2048 Hot Metal Glow
#define PROP_LIFE			0x01000  //4096 Is a GoL type
#define PROP_RADIOACTIVE	0x02000  //8192 Radioactive
#define PROP_LIFE_DEC		0x04000  //2^14 Life decreases by one every frame if > zero
#define PROP_LIFE_KILL		0x08000  //2^15 Kill when life value is <= zero
#define PROP_LIFE_KILL_DEC	0x10000  //2^16 Kill when life value is decremented to <= zero
#define PROP_SPARKSETTLE	0x20000  //2^17 Allow Sparks/Embers to settle
#define PROP_NOAMBHEAT		0x40000  //2^18 Don't transfer or receive heat from ambient heat.
#define PROP_DRAWONCTYPE	0x80000  //2^19 Set its ctype to another element if the element is drawn upon it (like what CLNE does)
#define PROP_NOCTYPEDRAW	0x100000 // 2^20 When this element is drawn upon with, do not set ctype (like BCLN for CLNE)

#define FLAG_STAGNANT	0x1
#define FLAG_SKIPMOVE  0x2 // skip movement for one frame, only implemented for PHOT
#define FLAG_WATEREQUAL 0x4 //if a liquid was already checked during equalization
#define FLAG_MOVABLE  0x8 // compatibility with old saves (moving SPNG), only applies to SPNG
#define FLAG_PHOTDECO  0x8 // compatibility with old saves (decorated photons), only applies to PHOT. Having the same value as FLAG_MOVABLE is fine because they apply to different elements, and this saves space for future flags,


#define UPDATE_FUNC_ARGS Simulation* sim, int i, int x, int y, int surround_space, int nt, Particle *parts, int pmap[YRES][XRES]
#define UPDATE_FUNC_SUBCALL_ARGS sim, i, x, y, surround_space, nt, parts, pmap

#define GRAPHICS_FUNC_ARGS Renderer * ren, Particle *cpart, int nx, int ny, int *pixel_mode, int* cola, int *colr, int *colg, int *colb, int *firea, int *firer, int *fireg, int *fireb
#define GRAPHICS_FUNC_SUBCALL_ARGS ren, cpart, nx, ny, pixel_mode, cola, colr, colg, colb, firea, firer, fireg, fireb

#define BOUNDS_CHECK true

#define OLD_PT_WIND 147

// Change this to change the amount of bits used to store type in pmap (and a few elements such as PIPE and CRAY)
#define PMAPBITS 9
#define PMAPMASK ((1<<PMAPBITS)-1)
#define ID(r) ((r)>>PMAPBITS)
#define TYP(r) ((r)&PMAPMASK)
#define PMAP(id, typ) ((id)<<PMAPBITS | ((typ)&PMAPMASK))
#define PMAPID(id) ((id)<<PMAPBITS)

#define PT_NUM	(1<<PMAPBITS)

#if PMAPBITS > 16
#error PMAPBITS is too large
#endif
#if ((XRES*YRES)<<PMAPBITS) > 0x100000000L
#error not enough space in pmap
#endif

struct playerst;

#include "ElementClasses.h"


#endif /* ELEMENTS_H_ */
