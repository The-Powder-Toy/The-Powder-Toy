/*
 * Elements.h
 *
 *  Created on: Jan 5, 2012
 *      Author: Simon
 */

#ifndef ELEMENTS_H_
#define ELEMENTS_H_

//#include "Config.h"
//#include "Simulation.h"

#define R_TEMP 22
#define MAX_TEMP 9999
#define MIN_TEMP 0
#define O_MAX_TEMP 3500
#define O_MIN_TEMP -273

#define TYPE_PART			0x00001 //1 Powders
#define TYPE_LIQUID			0x00002 //2 Liquids
#define TYPE_SOLID			0x00004 //4 Solids
#define TYPE_GAS			0x00008 //8 Gasses (Includes plasma)
#define TYPE_ENERGY			0x00010 //16 Energy (Thunder, Light, Neutrons etc.)
#define PROP_CONDUCTS		0x00020 //32 Conducts electricity
#define PROP_BLACK			0x00040 //64 Absorbs Photons (not currently implemented or used, a photwl attribute might be better)
#define PROP_NEUTPENETRATE	0x00080 //128 Penetrated by neutrons
#define PROP_NEUTABSORB		0x00100 //256 Absorbs neutrons, reflect is default
#define PROP_NEUTPASS		0x00200 //512 Neutrons pass through, such as with glass
#define PROP_DEADLY			0x00400 //1024 Is deadly for stickman
#define PROP_HOT_GLOW		0x00800 //2048 Hot Metal Glow
#define PROP_LIFE			0x01000 //4096 Is a GoL type
#define PROP_RADIOACTIVE	0x02000 //8192 Radioactive
#define PROP_LIFE_DEC		0x04000 //2^14 Life decreases by one every frame if > zero
#define PROP_LIFE_KILL		0x08000 //2^15 Kill when life value is <= zero
#define PROP_LIFE_KILL_DEC	0x10000 //2^16 Kill when life value is decremented to <= zero
#define PROP_SPARKSETTLE	0x20000	//2^17 Allow Sparks/Embers to settle
#define PROP_NOAMBHEAT      0x40000 //2^18 Don't transfer or receive heat from ambient heat.

#define FLAG_STAGNANT	0x1
#define FLAG_SKIPMOVE  0x2 // skip movement for one frame, only implemented for PHOT
#define FLAG_WATEREQUAL 0x4 //if a liquid was already checked during equalization
#define FLAG_MOVABLE  0x8 // if can move

#define ST_NONE 0
#define ST_SOLID 1
#define ST_LIQUID 2
#define ST_GAS 3

#define UPDATE_FUNC_ARGS Simulation* sim, int i, int x, int y, int surround_space, int nt, Particle *parts, int pmap[YRES][XRES]
#define UPDATE_FUNC_SUBCALL_ARGS sim, i, x, y, surround_space, nt, parts, pmap

#define GRAPHICS_FUNC_ARGS Renderer * ren, Particle *cpart, int nx, int ny, int *pixel_mode, int* cola, int *colr, int *colg, int *colb, int *firea, int *firer, int *fireg, int *fireb
#define GRAPHICS_FUNC_SUBCALL_ARGS ren, cpart, nx, ny, pixel_mode, cola, colr, colg, colb, firea, firer, fireg, fireb

#define SPC_AIR 236
#define SPC_HEAT 237
#define SPC_COOL 238
#define SPC_VACUUM 239
#define SPC_WIND 241
#define SPC_PGRV 243
#define SPC_NGRV 244
#define SPC_PROP 246


#define NGT_GOL 0
#define NGT_HLIF 1
#define NGT_ASIM 2
#define NGT_2x2 3
#define NGT_DANI 4
#define NGT_AMOE 5
#define NGT_MOVE 6
#define NGT_PGOL 7
#define NGT_DMOE 8
#define NGT_34 9
#define NGT_LLIF 10
#define NGT_STAN 11
#define NGT_SEED 12
#define NGT_MAZE 13
#define NGT_COAG 14
#define NGT_WALL 15
#define NGT_GNAR 16
#define NGT_REPL 17
#define NGT_MYST 18
#define NGT_LOTE 19
#define NGT_FRG2 20
#define NGT_STAR 21
#define NGT_FROG 22
#define NGT_BRAN 23

#define OLD_PT_WIND 147

//#define PT_NUM  161
#define PT_NUM	256

struct playerst;

#include "ElementClasses.h"


#endif /* ELEMENTS_H_ */
