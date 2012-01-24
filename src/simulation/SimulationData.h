/*
 * SimulationData.h
 *
 *  Created on: Jan 24, 2012
 *      Author: Simon
 */

#define SC_WALL 0
#define SC_ELEC 1
#define SC_POWERED 2
#define SC_EXPLOSIVE 3
#define SC_GAS 4
#define SC_LIQUID 5
#define SC_POWDERS 6
#define SC_SOLIDS 7
#define SC_NUCLEAR 8
#define SC_SPECIAL 9
#define SC_LIFE 10
#define SC_TOOL 11
#define SC_CRACKER 13
#define SC_CRACKER2 14
#define SC_TOTAL 12

#define UI_WALLSTART 222
#define UI_ACTUALSTART 122
#define UI_WALLCOUNT 25

#define WL_WALLELEC	122
#define WL_EWALL	123
#define WL_DETECT	124
#define WL_STREAM	125
#define WL_SIGN	126
#define WL_FAN	127
#define WL_FANHELPER 255
#define WL_ALLOWLIQUID	128
#define WL_DESTROYALL	129
#define WL_ERASE	130
#define WL_WALL	131
#define WL_ALLOWAIR	132
#define WL_ALLOWSOLID	133
#define WL_ALLOWALLELEC	134
#define WL_EHOLE	135

#define SPC_AIR 236
#define SPC_HEAT 237
#define SPC_COOL 238
#define SPC_VACUUM 239
#define SPC_WIND 241
#define SPC_PGRV 243
#define SPC_NGRV 244
#define SPC_PROP 246

#define WL_ALLOWGAS	140
#define WL_GRAV		142
#define WL_ALLOWENERGY 145

#ifndef SIMULATIONDATA_H_
#define SIMULATIONDATA_H_

#include "Simulation.h"

/*class Simulation;
class Renderer;
struct Particle;*/

struct part_type;

struct part_transition;

struct wall_type;

struct gol_menu;

struct menu_section;

struct wall_type;

wall_type * LoadWalls(int & wallCount);

menu_section * LoadMenus(int & menuCount);

part_type * LoadElements(int & elementCount);

part_transition * LoadTransitions(int & transitionCount);

#endif /* SIMULATIONDATA_H_ */
