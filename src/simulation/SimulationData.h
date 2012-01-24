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

menu_section * LoadMenus(int & menuCount);

part_type * LoadElements(int & elementCount);

part_transition * LoadTransitions(int & transitionCount);

#endif /* SIMULATIONDATA_H_ */
